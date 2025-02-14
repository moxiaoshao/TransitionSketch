#ifndef _CMSKETCH_H
#define _CMSKETCH_H

#include "BOBHash32.h"
#include "SPA.h"
#include <cstring>
#include <algorithm>

using namespace std;

// template <int memory_in_bytes, int d>
class CMSketch: public SPA
{
private:
    int memory_in_bytes;
    int d;
    int w;
    int counter_max = (1 << 8) - 1;
	int *counters;
    BOBHash32 **hash;
    //int counters[w];
	// BOBHash32 * hash[d];

public:
    CMSketch(int _memory_int_bytes, int _d) : memory_in_bytes(_memory_int_bytes), d(_d)
    {
        w = memory_in_bytes * 8 / 8;
        counters = new int[w];
        hash = new BOBHash32 *[d];
        memset(counters, 0, sizeof(int) * w);
        for (int i = 0; i < d; i++)
            hash[i] = new BOBHash32(i + 750);
    }

    void print_basic_info()
    {
        printf("CM sketch\n");
        printf("\tCounters: %d\n", w);
        printf("\tMemory: %.6lfMB\n", w * 1.0 / 1024 / 1024);
    }

    virtual ~CMSketch()
    {
        for (int i = 0; i < d; i++)
            delete hash[i];
    }

    void insert(uint32_t key, int f)
    {
        for (int i = 0; i < d; i++) {
            int index = (hash[i]->run((const char *)&key, 4)) % w;
            counters[index] += f;
            counters[index] = min(counters[index], counter_max);
        }
    }

    void apply_all(uint32_t key, int f)
    {
        f = min(f, counter_max);
        for (int i = 0; i < d; i++) {
            int index = (hash[i]->run((const char *)&key, 4)) % w;
            if(counters[index] < f){
                counters[index] = f;
            }
        }
    }

	int query(uint32_t key)
    {
        int ret = counter_max;
        for (int i = 0; i < d; i++) {
            int tmp = counters[(hash[i]->run((const char *)&key, 4)) % w];
            ret = min(ret, tmp);
        }
        return ret;
    }

    int batch_query(uint32_t * data, int n) {
        int ret = 0;
        for (int i = 0; i < n; ++i) {
            ret += query(data[i]);
        }
        return ret;
    }

    void build(uint32_t * data, int n)
    {
        for (int i = 0; i < n; ++i) {
            insert(data[i], 1);
        }
    }
};

#endif //_CMSKETCH_H