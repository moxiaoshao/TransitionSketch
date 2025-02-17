#ifndef LFU_H
#define LFU_H

#include "BOBHash32.h"
#include "utils.h"
#include <bits/stdc++.h>
#include "mutex_pool.h"

using namespace std;



class LFUBucket
{
public:
    uint32_t *fp;
    uint32_t *counter;

    LFUBucket() {}


    LFUBucket(int cols)
    {
        fp = new uint32_t[cols];
        counter = new uint32_t[cols];
        memset(fp, 0, sizeof(uint32_t) * cols);
        memset(counter, 0, sizeof(uint32_t) * cols);
    }


    int permutation(int p) // permute the p-th item to the first
    {
        int i;
        for (i = p; i > 0; --i)
        {
            if(counter[i] > counter[i - 1]){
                swap(fp[i], fp[i - 1]);
                swap(counter[i], counter[i - 1]);
            }
            else{
                break;
            }
        }
        return i;
    }

    pair<int, int> remove(int p, int cols, int count){
        int i, cnt = counter[p], fpt = fp[p];
        for(i = p; i + 1 < cols; i ++){
            if(counter[i + 1] != 0){
                swap(fp[i], fp[i + 1]);
                swap(counter[i], counter[i + 1]);
            }
            else{
                break;
            }
        }
        fp[i] = counter[i] = 0;
        return {fpt, cnt + count};
    }
};

class LFU
{
public:
    LFUBucket *Bucket1;
    int bucket_num;
    int cols, counter_len;

    LFU() {}

    ~LFU() { clear(); }

    void clear(){
        if (Bucket1)
            delete Bucket1;
    }

    LFU(int _bucket_num, int _cols, int _counter_len, int rand_seed): bucket_num(_bucket_num),cols(_cols), counter_len(_counter_len)
    {
        Bucket1 = new LFUBucket[bucket_num];
        for (int i = 0; i < bucket_num; ++i)
            Bucket1[i] = LFUBucket(_cols);

    }

    int find(uint32_t key, int count = 1, int idx = -1) // return dequeued item
    {
        /* if the item is in B1 */
        auto &B1 = Bucket1[idx];
        for (int i = 0; i < cols; ++i)
            if (B1.fp[i] == key)
            {
                auto res = B1.remove(i, cols, count);
                return res.second;
            }
        return -1;
    }

    pair<int, int> insert(uint32_t key, int count = 1, int op = 1, int idx = -1)
    {
        auto &B1 = Bucket1[idx];
        for(int i = 0; i < cols; ++i)
            if(B1.fp[i] == 0){
                B1.fp[i] = key;
                B1.counter[i] = count;
                B1.permutation(i);
                return {0, count};
            }
        if(op == 0) return {-1, -1};
        auto res = B1.remove(cols - 1, cols, 0);
        B1.fp[cols - 1] = key;
        B1.counter[cols - 1] = count;
        B1.permutation(cols - 1);
        return res;
    }
    int insert_back(uint32_t key, int count = 1, int idx = -1)
    {
        /* if the item is in B1 */
        auto &B1 = Bucket1[idx];

        B1.fp[cols - 1] = key;
        B1.counter[cols - 1] = count;
        B1.permutation(cols - 1);
        return B1.counter[0];
    }

    pair<int, int> pop(int idx = -1){
        auto &B1 = Bucket1[idx];
        pair<int, int> res = {B1.fp[cols - 1], B1.counter[cols - 1]};
        B1.fp[cols - 1] = B1.counter[cols - 1] = 0;
        return res;
    }

    pair<int, int> find_min(int idx = -1){
        auto &B1 = Bucket1[idx];
        pair<int, int> res = {B1.fp[cols - 1], B1.counter[cols - 1]};
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
        cout << "LFU:" << endl; 
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