#pragma once
#include <bits/stdc++.h>
#include "BOBHash32.h"
#include "LRU.h"
#include "LFU.h"
#include "tower.h"

#include "mutex_pool.h"
#include "main.h"


using namespace std;


class TransitionSketchMultiThread : public TransitionSketch
{
public:
    single_layer_mutex_pool lrulfu_pool;
    multi_layer_mutex_pool tower_pool;

    TransitionSketchMultiThread(double mem, int _bucket_num, int _cols, int _counter_len,
                  int rand_seed, double per, mutex_pool_config_t mutex_config=default_mutex_pool_config) : lrulfu_pool(mutex_config["LRULFU"][0]), tower_pool(mutex_config["Tower"]), TransitionSketch(mem, _bucket_num, _cols, _counter_len, rand_seed, per)
    {}

    vector<int> search_idx_in_sketch(uint32_t id){
        return tower->search_idx(id);
    }

    vector<int> build(uint32_t * items, int n) override {
        vector<int> final_result(3);
        std::vector<std::vector<int>> thread_results(test_thread_number, std::vector<int>(3, 0));
        thread_results.reserve(test_thread_number);
        // distribute the items to different threads
        const int num_threads = test_thread_number;

        std::vector<std::thread> threads;
        size_t chunk_size = n / num_threads;
        
        for (int i = 0; i < num_threads; ++i) {
            size_t start_idx = i * chunk_size;
            size_t end_idx = (i == num_threads - 1) ? n : (i + 1) * chunk_size;
            threads.emplace_back([&items, start_idx, end_idx, i, &thread_results, this]() {
                thread_results[i] = build(items, start_idx, end_idx);
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        threads.clear();

        for(const auto& result : thread_results) {
            for(int i = 0; i < 3; i++) {
                final_result[i] += result[i];
            }
        }
        
        return final_result;
    }
    
    vector<int> build(uint32_t * items, int l, int r){
        vector<int> result(3);
        for (int i = l; i < r; ++i){
            auto item = items[i];
            int idx = lHash->run((const char *)&item, 4) % bucket_num;
#if _LOCK_STAGE_LEVEL_ >= 1
            std::unique_lock<std::shared_mutex> lrulfu_lock(lrulfu_pool[idx]);
#endif
            int res = lru->insert(item, 1, idx);
            if(res == -1){
                res = lfu->find(item, 1, idx);
                if(res == -1){
                    auto p = lfu->insert(item, 1, 0, idx);
                    if(p.first == -1){
                        // lock tower pool (semi-atomic, layer-wise atomic is sufficient to avoid dead lock)
                        // maybe full atomic is faster, use full atomic
#if _LOCK_STAGE_LEVEL_ >= 2
                        auto tmp = lfu->find_min(idx);
                        auto item2_id = tmp.first;
                        auto idx_list1 = search_idx_in_sketch(item);
                        auto idx_list2 = search_idx_in_sketch(item2_id);
                        std::unique_lock<std::shared_mutex> pool_lock(tower_pool.pool_lock);
                        std::vector<std::unique_lock<std::shared_mutex>> locks;
                        for(int i = 0; i < idx_list1.size(); i++) {
                            if (idx_list1[i] < idx_list2[i]) {
                                locks.emplace_back(tower_pool[i][idx_list1[i]]);
                                if (tower_pool.judge_if_not_equal(i, idx_list1[i], idx_list2[i])) {
                                    locks.emplace_back(tower_pool[i][idx_list2[i]]);
                                }
                            } else {
                                locks.emplace_back(tower_pool[i][idx_list2[i]]);
                                if (tower_pool.judge_if_not_equal(i, idx_list1[i], idx_list2[i])) {
                                    locks.emplace_back(tower_pool[i][idx_list1[i]]);
                                }
                            }
                        }
                        pool_lock.unlock();
#endif
                        // lock success
                        res = query_by_sketch(item);
                        result[0] ++;
                        if(res == 0){
                            apply_by_sketch(item, 0, 1);
                        }
                        else{
                            auto tmp = lfu->find_min(idx);
                            if(res >= tmp.second){
                                result[1] ++;
                                lfu->pop(idx);
                                lfu->insert(item, res + 1, 1, idx);
                                int cnt = query_by_sketch(tmp.first);
                                if(tmp.second > cnt){
                                    result[2] += tmp.second - cnt;
                                    apply_by_sketch(tmp.first, cnt, tmp.second);
                                }
                            }
                            else{
                                apply_by_sketch(item, res, res + 1);
                            }
                        }
                    }
                }
                else{
                    auto element = lru->pop(item, idx);
                    lru->insert(item, res, idx);
                    lfu->insert(element.first, element.second, 1, idx);
                }
            }
        }
        return result;
    }

};