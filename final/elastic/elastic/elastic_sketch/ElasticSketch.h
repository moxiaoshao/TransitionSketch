#ifndef _ELASTIC_SKETCH_H_
#define _ELASTIC_SKETCH_H_

// #include "HeavyPart->h"
#include "HeavyPart-noSIMD.h"
#include "LightPart.h"

// template <int bucket_num, int tot_memory_in_bytes>
class ElasticSketch
{   
    int bucket_num, tot_memory_in_bytes;
    int heavy_mem;
    int light_mem;

    HeavyPart *heavy_part;
    LightPart *light_part;

public:
    ElasticSketch(int _bucket_num, int _tot_memory_in_bytes) : bucket_num(_bucket_num), tot_memory_in_bytes(_tot_memory_in_bytes) {
        heavy_mem = bucket_num * COUNTER_PER_BUCKET * 8;
        light_mem = tot_memory_in_bytes - heavy_mem;
        heavy_part = new HeavyPart(bucket_num);
        light_part = new LightPart(light_mem) ;
    }
    ~ElasticSketch() {}
    void clear()
    {
        heavy_part->clear();
        light_part->clear();
    }

    void insert(uint32_t key, int f = 1)
    {
        uint32_t swap_key;
        uint32_t swap_val = 0;
        int result = heavy_part->insert(key, swap_key, swap_val, f);
        switch (result)
        {
        case 0:
            return;
        case 1:
        {
            if (HIGHEST_BIT_IS_1(swap_val))
                light_part->insert(swap_key, GetCounterVal(swap_val));
            else
                light_part->swap_insert(swap_key, swap_val);
            return;
        }
        case 2:
            light_part->insert(key, 1);
            return;
        default:
            printf("error return value !\n");
            exit(1);
        }
    }



    int query(uint32_t key)
    {
        uint32_t heavy_result = heavy_part->query(key);
        if (heavy_result == 0 || HIGHEST_BIT_IS_1(heavy_result))
        {
            int light_result = light_part->query(key);
            return (int)GetCounterVal(heavy_result) + light_result;
        }
        return heavy_result;
    }

    int query_compressed_part(uint32_t key, uint32_t *compress_part, int compress_counter_num)
    {
        uint32_t heavy_result = heavy_part->query(key);
        if (heavy_result == 0 || HIGHEST_BIT_IS_1(heavy_result))
        {
            int light_result = light_part->query_compressed_part(key, compress_part, compress_counter_num);
            return (int)GetCounterVal(heavy_result) + light_result;
        }
        return heavy_result;
    }

    void get_heavy_hitters(int threshold, vector<pair<string, int>> &results)
    {
        for (int i = 0; i < bucket_num; ++i)
            for (int j = 0; j < MAX_VALID_COUNTER; ++j)
            {
                uint32_t key = heavy_part->buckets[i].key[j];
                int val = query((uint32_t)key);
                if (val >= threshold)
                {
                    results.push_back(make_pair(string((const char *)&key, 4), val));
                }
            }
    }

    /* interface */
    int get_compress_width(int ratio) { return light_part->get_compress_width(ratio); }
    void compress(int ratio, uint32_t *dst) { light_part->compress(ratio, dst); }
    int get_bucket_num() { return heavy_part->get_bucket_num(); }
    double get_bandwidth(int compress_ratio)
    {
        int result = heavy_part->get_memory_usage();
        result += get_compress_width(compress_ratio) * sizeof(uint32_t);
        return result * 1.0 / 1024 / 1024;
    }

    int get_cardinality()
    {
        int card = light_part->get_cardinality();
        for (int i = 0; i < bucket_num; ++i)
            for (int j = 0; j < MAX_VALID_COUNTER; ++j)
            {
                // uint8_t key[KEY_LENGTH_4];
                uint32_t key = heavy_part->buckets[i].key[j];
                int val = heavy_part->buckets[i].val[j];
                int ex_val = light_part->query(key);

                if (HIGHEST_BIT_IS_1(val) && ex_val)
                {
                    val += ex_val;
                    card--;
                }
                if (GetCounterVal(val))
                    card++;
            }
        return card;
    }

    double get_entropy()
    {
        int tot = 0;
        double entr = 0;

        light_part->get_entropy(tot, entr);

        for (int i = 0; i < bucket_num; ++i)
            for (int j = 0; j < MAX_VALID_COUNTER; ++j)
            {
                // uint8_t key[KEY_LENGTH_4];
                uint32_t key = heavy_part->buckets[i].key[j];
                int val = heavy_part->buckets[i].val[j];

                int ex_val = light_part->query(key);

                if (HIGHEST_BIT_IS_1(val) && ex_val)
                {
                    val += ex_val;

                    tot -= ex_val;

                    entr -= ex_val * log2(ex_val);
                }
                val = GetCounterVal(val);
                if (val)
                {
                    tot += val;
                    entr += val * log2(val);
                }
            }
        return -entr / tot + log2(tot);
    }

    void get_distribution(vector<double> &dist)
    {
        light_part->get_distribution(dist);

        for (int i = 0; i < bucket_num; ++i)
            for (int j = 0; j < MAX_VALID_COUNTER; ++j)
            {
                // uint8_t key[KEY_LENGTH_4];
                uint32_t key = heavy_part->buckets[i].key[j];
                int val = heavy_part->buckets[i].val[j];

                int ex_val = light_part->query(key);

                if (HIGHEST_BIT_IS_1(val) && ex_val != 0)
                {
                    val += ex_val;
                    dist[ex_val]--;
                }
                val = GetCounterVal(val);
                if (val)
                {
                    if (val + 1 > dist.size())
                        dist.resize(val + 1);
                    dist[val]++;
                }
            }
    }

    void *operator new(size_t sz)
    {
        constexpr uint32_t alignment = 64;
        size_t alloc_size = (2 * alignment + sz) / alignment * alignment;
        void *ptr = ::operator new(alloc_size);
        void *old_ptr = ptr;
        void *new_ptr = ((char *)std::align(alignment, sz, ptr, alloc_size) + alignment);
        ((void **)new_ptr)[-1] = old_ptr;

        return new_ptr;
    }
    void operator delete(void *p)
    {
        ::operator delete(((void **)p)[-1]);
    }
};

#endif