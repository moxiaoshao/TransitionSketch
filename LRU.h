#ifndef LRU_H
#define LRU_H


#include "BOBHash32.h"
#include "utils.h"
#include <bits/stdc++.h>
#include "mutex_pool.h"

using namespace std;



class LRUBucket
{
public:
    uint32_t *fp;
    uint32_t *counter;

    LRUBucket() {}



    LRUBucket(int cols)
    {
        fp = new uint32_t[cols];
        counter = new uint32_t[cols];
        memset(fp, 0, sizeof(uint32_t) * cols);
        memset(counter, 0, sizeof(uint32_t) * cols);
    }

    void permutation(int p) // permute the p-th item to the first
    {
        for (int i = p; i > 0; --i)
        {
            swap(fp[i], fp[i - 1]);
            swap(counter[i], counter[i - 1]);
        }
    }
};

class LRU
{
public:
    LRUBucket *Bucket1;
    int bucket_num;
    int cols, counter_len;

    LRU() {}



    LRU(int _bucket_num, int _cols, int _counter_len, int rand_seed): bucket_num(_bucket_num), cols(_cols), counter_len(_counter_len)
    {
        Bucket1 = new LRUBucket[bucket_num];
        for (int i = 0; i < bucket_num; ++i)
            Bucket1[i] = LRUBucket(_cols);

    }

    ~LRU() { clear(); }

    void clear(){
        if (Bucket1)
            delete Bucket1;
    }

    int insert(uint32_t key, int count = 1, int idx = -1) // return dequeued item
    {
        /* if the item is in B1 */
        auto &B1 = Bucket1[idx];
        for (int i = 0; i < cols; ++i){
            if (B1.fp[i] == key)
            {
                B1.permutation(i);
                B1.counter[0] = min(B1.counter[0] + count, (1u << counter_len) - 1);
                return B1.counter[0];
            }
            if (B1.fp[i] == 0)
            {
                B1.permutation(i);
                B1.fp[0] = key;
                B1.counter[0] = count;
                return B1.counter[0];
            }
        }
        

        return -1;
    }



    pair<int, int> pop(uint32_t key, int idx = -1){
        auto &B1 = Bucket1[idx];
        pair<int, int> res = {B1.fp[cols - 1], B1.counter[cols - 1]};
        B1.fp[cols - 1] = B1.counter[cols - 1] = 0;
        return res;
    }

    int find_id(uint32_t key, int idx = -1)
    {
        auto &B1 = Bucket1[idx];
        for (int i = 0; i < cols; ++i)
            if (B1.fp[i] == key)
            {
                return B1.counter[i];
            }
        return -1;
    }

    void print(){
        cout << "LRU:" << endl; 
        for(int i = 0; i < bucket_num; i ++){
            cout << "Bucket " << i << " : ";
            for(int j = 0; j < cols; j ++){
                cout << "( " << Bucket1[i].fp[j] << ", " << Bucket1[i].counter[j] << " )" << ", ";
            }
            cout << endl;
        }
        cout << endl;
    }

    vector<pair<int, int>> extract(){
        vector<pair<int, int>> res;
        for(int i = 0; i < bucket_num; i ++){
            auto &B1 = Bucket1[i];
            for(int j = 0; j < cols; j ++){
                res.push_back({B1.fp[j], B1.counter[j]});
            }
        }
        return res;
    }
};

#endif