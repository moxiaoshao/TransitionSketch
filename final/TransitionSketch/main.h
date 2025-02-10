#include <bits/stdc++.h>
#include "BOBHash32.h"
#include "LRU.h"
#include "LFU.h"
#include "tower.h"


using namespace std;


class TransitionSketch
{
public:
    LRU *lru;
    LFU *lfu;
    TowerSketch *tower;
    int bucket_num, op;
    BOBHash32 *lHash;
    int cnt1, cnt2;
    ~TransitionSketch() { clear(); }

    void clear(){
        if (lru)
            delete lru;
        if (lfu)
            delete lfu;
        if (tower)
            delete tower;
    }
    TransitionSketch(double mem, int _bucket_num, int _cols, int _counter_len,
                  int rand_seed, double per)
    {   
        cnt1 = 0, cnt2 = 0;
        lru = new LRU(_bucket_num, _cols, _counter_len, rand_seed);
        lfu = new LFU(_bucket_num, _cols, _counter_len, rand_seed);
        bucket_num = _bucket_num;
        lHash = new BOBHash32(rand_seed);
        vector<uint32_t> width;
        for (int i = 0; i < width_mul_tower.size(); i++)
        {
            width.push_back((int)(mem * per * 1024 * width_mul_tower[i]));
        }
        tower = new TowerSketch(width, 1, cs_tower, 0);
    }

    int query_by_sketch(uint32_t id){
        return tower->query(id, 0);
    }

    void apply_by_sketch(uint32_t id, int old_v, int new_v){
        tower->apply_all(id, 0, new_v);
    }

    vector<int> build(uint32_t * items, int n){
        vector<int> result(3);
        for (int i = 0; i < n; ++i){
            auto item = items[i];
            int idx = lHash->run((const char *)&item, 4) % bucket_num;
            int res = lru->insert(item, 1, idx);
            if(res == -1){
                res = lfu->find(item, 1, idx);
                if(res == -1){
                    auto p = lfu->insert(item, 1, 0, idx);
                    if(p.first == -1){
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

    inline int query(uint32_t id)
    {
        int idx = lHash->run((const char *)&id, 4) % bucket_num;
        int res = lru->find_id(id, idx);
        if(res == -1){
            res = lfu->find_id(id, idx);
            if(res == -1){
                cnt2 ++;
                return tower->query(id, 0);
            }
            else{
                cnt1 ++;
                return res;
            }
        }   
        else{
            cnt1 ++;
            return res;
        }
    }

    unordered_map<int, int> query_topk(int k){
        vector<pair<int, int>> alls;
        unordered_map<int, int> res;
        auto res1 =  lru->extract();
        auto res2 = lfu->extract();
        for(auto i : res1){
            alls.push_back(i);
        }
        for(auto i : res2){
            alls.push_back(i);
        }
        sort(alls.begin(), alls.end(), [&](pair<int, int> a, pair<int, int> b){
            if(a.second != b.second)
                return a.second > b.second;
            return a.first > b.first;
        });
        for(int i = 0; i < k; i ++) res[alls[i].first] = alls[i].second;
        return res;
    }

};