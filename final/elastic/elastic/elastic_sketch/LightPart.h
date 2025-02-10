#ifndef _LIGHT_PART_H_
#define _LIGHT_PART_H_

#include "../../common/EMFSD.h"
#include "param.h"

// template <int init_mem_in_bytes>
class LightPart
{
    uint32_t *counters;
    int counter_num, init_mem_in_bytes;
    BOBHash32 *bobhash = NULL;

public:
    
    int mice_dist[256];
    EMFSD *em_fsd_algo = NULL;

    LightPart(int _init_mem_in_bytes) : init_mem_in_bytes(_init_mem_in_bytes)
    {   
        counter_num = init_mem_in_bytes;
        cout << "counter num = " << counter_num << endl;
        counters = new uint32_t[counter_num];
        clear();
        // for(int i = 0; i < counter_num; i ++) assert(counters[i] == 0);
        std::random_device rd;
        bobhash = new BOBHash32(rd() % MAX_PRIME32);
    }
    ~LightPart()
    {
        delete bobhash;
    }

    void clear()
    {
        memset(counters, 0, sizeof(uint32_t) * counter_num);
        memset(mice_dist, 0, sizeof(int) * 256);
    }

    /* insertion */
    void insert(uint32_t key, int f = 1)
    {
        uint32_t hash_val = (uint32_t)bobhash->run((const char *)&key, KEY_LENGTH_4);
        uint32_t pos = hash_val % (uint32_t)counter_num;
        int old_val = (int)counters[pos];
        int new_val = (int)counters[pos] + f;

        new_val = new_val < 255 ? new_val : 255;
        
        counters[pos] = (uint32_t)new_val;
        if(new_val > 255) cout << "new val : " << new_val <<endl;
        assert(counters[pos] <= 255);
        if(old_val > 255) cout << old_val << endl;
        mice_dist[old_val]--;
        mice_dist[new_val]++;
    }

    void swap_insert(uint32_t key, int f)
    {
        uint32_t hash_val = (uint32_t)bobhash->run((const char *)&key, KEY_LENGTH_4);
        uint32_t pos = hash_val % (uint32_t)counter_num;

        f = f < 255 ? f : 255;
        if (counters[pos] < f)
        {
            int old_val = (int)counters[pos];
            counters[pos] = (uint32_t)f;
            int new_val = (int)counters[pos];

            mice_dist[old_val]--;
            mice_dist[new_val]++;
        }
    }

    /* query */
    int query(uint32_t key)
    {
        uint32_t hash_val = (uint32_t)bobhash->run((const char *)&key, KEY_LENGTH_4);
        uint32_t pos = hash_val % (uint32_t)counter_num;

        return (int)counters[pos];
    }

    /* compress */
    void compress(int ratio, uint32_t *dst)
    {
        int width = get_compress_width(ratio);

        for (int i = 0; i < width && i < counter_num; ++i)
        {
            uint32_t max_val = 0;
            for (int j = i; j < counter_num; j += width)
                max_val = counters[j] > max_val ? counters[j] : max_val;
            dst[i] = max_val;
        }
    }

    int query_compressed_part(uint32_t key, uint32_t *compress_part, int compress_counter_num)
    {
        uint32_t hash_val = (uint32_t)bobhash->run((const char *)&key, KEY_LENGTH_4);
        uint32_t pos = (hash_val % (uint32_t)counter_num) % compress_counter_num;

        return (int)compress_part[pos];
    }

    /* other measurement task */
    int get_compress_width(int ratio) { return (counter_num / ratio); }
    int get_compress_memory(int ratio) { return (uint32_t)(counter_num / ratio); }
    int get_memory_usage() { return counter_num; }

    int get_cardinality()
    {
        int mice_card = 0;
        for (int i = 1; i < 256; i++)
            mice_card += mice_dist[i];

        double rate = (counter_num - mice_card) / (double)counter_num;
        return counter_num * log(1 / rate);
    }

    void get_entropy(int &tot, double &entr)
    {
        for (int i = 1; i < 256; i++)
        {
            tot += mice_dist[i] * i;
            entr += mice_dist[i] * i * log2(i);
        }
    }

    void get_distribution(vector<double> &dist)
    {
        uint32_t tmp_counters[counter_num];
        for (int i = 0; i < counter_num; i++)
            tmp_counters[i] = counters[i];

        em_fsd_algo = new EMFSD();
        em_fsd_algo->set_counters(counter_num, tmp_counters, UINT32_MAX);

        for (int i = 0; i < 20; ++i)
            em_fsd_algo->next_epoch();

        dist = em_fsd_algo->ns;
    }
};

#endif