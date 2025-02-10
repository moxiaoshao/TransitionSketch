#include <bits/stdc++.h>
#include "BOBHash32.h"
#include "utils.h"
using namespace std;

class Matrix
{
public:
    uint32_t counter_len, counter_per_int;
    uint32_t w, h, mask;
    uint32_t **counters;
    BOBHash32 *hash;

    uint32_t counter_max;

    Matrix() {}
    Matrix(uint32_t in_w, uint32_t in_h, uint32_t in_counter_len, uint32_t random_seed)
    {
        init(in_w, in_h, in_counter_len, random_seed);
    }
    ~Matrix() { clear(); }

    void init(uint32_t in_w, uint32_t in_h, uint32_t in_counter_len, uint32_t random_seed)
    {
        w = in_w, h = in_h, counter_len = in_counter_len;
        counter_per_int = 8 / counter_len;
        if (counter_len == 32)
            mask = counter_max = 0xffffffff;
        else
            mask = counter_max = (1u << counter_len) - 1;
        uint32_t actual_w = (w + counter_per_int - 1) / counter_per_int;
        counters = new uint32_t *[h];
        *counters = new uint32_t[h * actual_w];

        for (uint32_t i = 1; i < h; ++i)
            counters[i] = *counters + i * actual_w;
        memset(*counters, 0, sizeof(uint32_t) * h * actual_w);
        hash = new BOBHash32(random_seed);
    }

    void clear()
    {
        if (*counters)
            delete[] * counters;
        if (counters)
            delete[] counters;
        if (hash)
            delete hash;
    }

    inline void insert(uint32_t key, uint16_t key_len, uint32_t row_id, uint32_t f = 1)
    {
        uint32_t index = hash->run((const char *)&key, key_len) % w;
        uint32_t buf = counters[row_id][index / counter_per_int];
        uint32_t shift = index % counter_per_int * counter_len;
        uint32_t val = (buf >> shift) & mask;
        if (f > counter_max - val)
            val = counter_max;
        else
            val += f;
        counters[row_id][index / counter_per_int] = buf & ~(mask << shift) | (val << shift);
    }

    inline void insert_with_hash_value(uint32_t hash_value, uint32_t row_id, uint32_t f = 1)
    {
        uint32_t index = hash_value % w;
        uint32_t buf = counters[row_id][index / counter_per_int];
        uint32_t shift = index % counter_per_int * counter_len;
        uint32_t val = (buf >> shift) & mask;
        if (f > counter_max - val)
            val = counter_max;
        else
            val += f;
        counters[row_id][index / counter_per_int] = buf & ~(mask << shift) | (val << shift);
    }

    inline void apply_with_hash_value(uint32_t hash_value, uint32_t row_id, uint32_t f = 1)
    {
        uint32_t index = hash_value % w;
        uint32_t buf = counters[row_id][index / counter_per_int];
        uint32_t shift = index % counter_per_int * counter_len;
        uint32_t val = (buf >> shift) & mask;
        if (f > counter_max)
            val = counter_max;
        else
            val = f;
        counters[row_id][index / counter_per_int] = buf & ~(mask << shift) | (val << shift);
    }

    inline uint32_t query(uint32_t key, uint32_t row_id)
    {
        uint32_t index = hash->run((const char *)&key, 4) % w;
        uint32_t buf = counters[row_id][index / counter_per_int];
        uint32_t shift = index % counter_per_int * counter_len;
        uint32_t val = (buf >> shift) & mask;
        return val;
    }

    inline uint32_t query_with_hash_value(uint32_t hash_value, uint32_t row_id)
    {
        uint32_t index = hash_value % w;
        uint32_t buf = counters[row_id][index / counter_per_int];
        uint32_t shift = index % counter_per_int * counter_len;
        uint32_t val = (buf >> shift) & mask;
        assert(val < (1 << 8));
        return val;
    }

    uint32_t query_pos(int index, int row_id)
    {
        uint32_t buf = counters[row_id][index / counter_per_int];
        int shift = index % counter_per_int * counter_len;
        uint32_t val = (buf >> shift) & mask;
        return val;
    }

    double overflowRate()
    {
        uint32_t overflow_cnt = 0, used_cnt = 0;
        for (uint32_t row_id = 0; row_id < h; ++row_id)
            for (uint32_t index = 0; index < w; ++index)
            {
                uint32_t buf = counters[row_id][index / counter_per_int];
                uint32_t shift = index % counter_per_int * counter_len;
                uint32_t val = (buf >> shift) & mask;
                overflow_cnt += (val == counter_max);
                used_cnt += (val > 0);
            }
        return (double)overflow_cnt / used_cnt;
    }

    void used_count(int &used, int &total)
    {
        total = w * h;
        used = 0;
        for (uint32_t row_id = 0; row_id < h; ++row_id)
            for (uint32_t index = 0; index < w; ++index)
            {
                uint32_t buf = counters[row_id][index / counter_per_int];
                uint32_t shift = index % counter_per_int * counter_len;
                uint32_t val = (buf >> shift) & mask;
                used += (val > 0);
            }
    }
};

class TowerSketch
{
protected:
    uint32_t level;
    Matrix *mat;
    int threshold;




public:
    TowerSketch() {}
    TowerSketch(vector<uint32_t> &w, uint32_t h, vector<uint32_t> &counter_len, int _threshold = 0)
    {
        init(w, h, counter_len, _threshold);
    }
    ~TowerSketch() { clear(); }

    void init(vector<uint32_t> &w, uint32_t h, vector<uint32_t> &counter_len, int _threshold)
    {
        level = w.size();
        mat = new Matrix[level];
        for (uint32_t i = 0; i < level; ++i){
            mat[i].init(w[i], h, counter_len[i], 750 + i);
        }
        threshold = _threshold;

    }

    void clear()
    {
        if (mat)
            delete[] mat;
    }

    virtual void insert(uint32_t key, uint32_t row_id, uint32_t f = 1)
    {
        for (uint32_t i_level = 0; i_level < level; ++i_level)
            mat[i_level].insert_with_hash_value(key, row_id, f);
    }


    virtual void apply_all(uint32_t key, uint32_t row_id, uint32_t f = 1)
    {
        for (uint32_t i_level = 0; i_level < level; ++i_level){
            int cnt = mat[i_level].query_with_hash_value(key, row_id);
            if(cnt < f){
                mat[i_level].apply_with_hash_value(key, row_id, f);
            }
        }
    }

    uint32_t query(uint32_t key, uint32_t row_id)
    {
        uint32_t ret = UINT8_MAX;
        for (uint32_t i_level = 0; i_level < level; ++i_level)
        {
            uint32_t tmp = mat[i_level].query_with_hash_value(key, row_id);
            assert(tmp <= mat[i_level].counter_max);
            assert(mat[i_level].counter_max < (1 << 8));
            if (tmp < mat[i_level].counter_max)
                ret = min(ret, tmp);
        }
        return ret;
    }




};





