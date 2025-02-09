#ifndef _MULTI_LAYER_MUTEX_POOL_
#define _MULTI_LAYER_MUTEX_POOL_

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <random>
#include <thread>
#include <cassert>
#include <map>

#define ASSERT_MOD_POWER_OF_TWO

class single_layer_mutex_pool {
public:
    std::vector<std::shared_mutex> locks;
    // single_layer_mutex_pool(int n) {
    //     std::vector<std::shared_mutex> list(n);
    //     locks.swap(list);
    // }
#ifdef ASSERT_MOD_POWER_OF_TWO
    size_t size_minus_one;
    single_layer_mutex_pool(int n) : locks(n), size_minus_one(n - 1) {
        assert((n & (n - 1)) == 0);
    }
#else
    size_t size;
    single_layer_mutex_pool(int n) : locks(n), size(n) {}
#endif
    // shared_mutex usage:
    // std::unique_lock<std::shared_mutex> lock(locks[i]);
    // std::shared_lock<std::shared_mutex> lock(locks[i]);
    std::shared_mutex &operator[](size_t i) {
#ifdef ASSERT_MOD_POWER_OF_TWO
        return locks[i & size_minus_one];
#else
        return locks[i % size];
#endif
    }
};

class multi_layer_mutex_pool {
public:
    std::vector<single_layer_mutex_pool> locks;
    std::shared_mutex pool_lock;
    std::vector<int> config;

    // multi_layer_mutex_pool(int n, int m) : locks(n, single_layer_mutex_pool(m)) {}

    multi_layer_mutex_pool(std::vector<int> &n) {
        for (int i = 0; i < n.size(); i++) {
            locks.push_back(single_layer_mutex_pool(n[i]));
        }
#ifdef ASSERT_MOD_POWER_OF_TWO
        config.resize(n.size());
        for (int i = 0; i < n.size(); i++) {
            config[i] = n[i] - 1;
            assert((n[i] & (n[i] - 1)) == 0);
        }
#else
        config = n;
#endif
    }

    single_layer_mutex_pool &operator[](size_t i) {
        return locks[i];
    }

    bool judge_if_not_equal(int layer, int idx1, int idx2) {
#ifdef ASSERT_MOD_POWER_OF_TWO
        return ((idx1 - idx2) & config[layer]) != 0;
#else
        return ((idx1 - idx2) % config[layer]) != 0;
#endif
    }
};

typedef std::map<std::string, std::vector<int>> mutex_pool_config_t;
mutex_pool_config_t default_mutex_pool_config = {
    {"LRULFU", {1024}},
    {"Tower", {64, 64, 64}}
};

int test_thread_number = 1;
double test_throughput_result = 0;

// control the level of lock action, 0 for no lock, 1 for lock in LRULFU, 2 for lock in Tower
#define _LOCK_STAGE_LEVEL_ 1

#endif