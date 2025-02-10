#include <iostream>
#include <string>
#include <queue>
#include <stack>
#include <list>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <memory>
#include <numeric>
#include <cmath>
#include <stdexcept>
#include <cassert>
#include <cstring>
#include <fstream>
#include<chrono>
#include "SpaceSaving.h"


// Bob hash
#include "BOBHash32.h"
#include "SPA.h"
// SIMD 
#include <immintrin.h>
#define UNIX
#ifdef UNIX
#include <x86intrin.h>
#else
#include <intrin.h>
#endif

using namespace std;

#define MAX_INSERT_PACKAGE 32768000

unordered_map<uint32_t, int> ground_truth;
uint32_t insert_data[MAX_INSERT_PACKAGE];
uint32_t query_data[MAX_INSERT_PACKAGE];
FILE* SS_ACC;
FILE* SS_AAE;
FILE* SS_TPT;


// load data
int load_data(const char *filename) {
    FILE *pf = fopen(filename, "rb");
    if (!pf) {
        cerr << filename << " not found." << endl;
        exit(-1);
    }
    memset(insert_data, 0, sizeof insert_data);
    memset(query_data, 0, sizeof query_data);
    ground_truth.clear();
    char ip[13];
    char ts[8];
    int ret = 0;
    while (1) {
        size_t rsize;
        rsize = fread(ip, 1, 13, pf);
        if(rsize != 13) break;
        rsize = fread(ts, 1, 8, pf);
        if(rsize != 8) break;
        uint32_t key = *(uint32_t *) ip;
        insert_data[ret] = key;
        ground_truth[key]++;
        ret++;
    }
    fclose(pf);
    printf("Total stream size = %d\n", ret);
    printf("Distinct item number = %ld\n", ground_truth.size());
    int max_freq = 0;
    for (auto itr: ground_truth) {
        max_freq = std::max(max_freq, itr.second);
    }
    printf("Max frequency = %d\n", max_freq);
    vector<int> s;
    for (auto itr: ground_truth) {
        s.push_back(itr.second);
    }
    sort(s.begin(), s.end());
    cout << s[s.size() - 1] << " " << s[s.size() - 1000] << endl;

    return ret;
}

pair<double, double> ss_compare_value_with_ground_truth(uint32_t k, vector<pair<uint32_t, uint32_t>> & result)
{
    // prepare top-k ground truth
    
    vector<pair<uint32_t, int>> gt_ordered;
    for (auto itr: ground_truth) {
        gt_ordered.emplace_back(itr);
    }
    unordered_map<uint32_t, int> ground_truth_topk, predicted_topk;
    for(auto i : result){
        predicted_topk[i.first] = i.second;
    }
    sort(gt_ordered.begin(), gt_ordered.end(), [&](pair<uint32_t, int> a, pair<uint32_t, int> b){
        if(a.second != b.second)
            return a.second > b.second;
        return a.first > b.first;
    });
    for(int i = 0; i < k; i ++) ground_truth_topk[gt_ordered[i].first] = gt_ordered[i].second;
    int hit = 0, tot = 0;
    double topk_are = 0, topk_aae = 0;
    for(auto itr : ground_truth_topk){
        int freq = predicted_topk[itr.first];
        if(freq > 0){
            hit ++;
            topk_are += fabs((double)itr.second - freq) / (double)itr.second;
            topk_aae += fabs((double)itr.second - freq);
        }
        tot += (itr.second > 0);
    }
    cout << "TopkF1 Score = " << (double)hit / (double)tot << endl;
    cout << "Topk ARE  = " << topk_are / (double)tot << endl;
    cout << "Topk AAE  = " << topk_aae / (double)tot << endl;
    return make_pair((double)hit / (double)tot, topk_are / (double)tot);
}
vector<double> pr = {0.1,0.01,0.001,0.0001,0.00001,0.000001};
template<uint32_t topk>
double demo_ss(const int mem_in_byte,int packet_num)
{
    printf("\nExp for top-k query:\n");
    printf("\ttest top %d\n", topk);
    printf("\tmem_in_byte %d\n",mem_in_byte);
    auto ss = new SpaceSaving((int)(topk*1.5)+mem_in_byte/100);
    
    
    timespec dtime1, dtime2;
    long long delay = 0.0;
    
    int k = topk;
    vector<pair<uint32_t, uint32_t>> result(k);
    
    printf("-----------------------------------------------\n");
    
    vector< pair<double, double> > res;
    
                   


    // SpaceSaving
    clock_gettime(CLOCK_MONOTONIC, &dtime1);
    
    ss->build(insert_data, packet_num);
    clock_gettime(CLOCK_MONOTONIC, &dtime2);
    delay = (long long)(dtime2.tv_sec - dtime1.tv_sec) * 1000000000LL + (dtime2.tv_nsec - dtime1.tv_nsec);
    unordered_map<int, int> predicted_topk;
    ss->get_top_k_with_frequency(topk, predicted_topk);
    vector<pair<int, int>>  tmp;
    unordered_map<int, int> ground_truth_topk;
    for(auto itr: ground_truth){
        tmp.push_back({itr.first, itr.second});
    }
    sort(tmp.begin(), tmp.end(), [&](pair<int, int> a, pair<int, int> b){
        if(a.second != b.second)
            return a.second > b.second;
        return a.first > b.first;
    });
    for(int i = 0; i < topk; i ++) ground_truth_topk[tmp[i].first] = tmp[i].second;
    int hit = 0, tot = 0, zero_error = 0;
    double topk_are = 0, topk_aae = 0;
    for(auto itr : ground_truth_topk){
        int freq = predicted_topk[itr.first];
        if (freq > 0){
            hit ++;
            if(ground_truth_topk[itr.first] == freq) zero_error ++;
            topk_are += fabs(ground_truth_topk[itr.first] - freq) / (double)ground_truth_topk[itr.first];
            topk_aae += fabs(ground_truth_topk[itr.first] - freq);
        } 
        else {
            topk_are += fabs(ground_truth_topk[itr.first] - 0) / (double)ground_truth_topk[itr.first];
            topk_aae += fabs(ground_truth_topk[itr.first] - 0);
        }
        tot += (itr.second > 0);
    }
    cout << "TopkF1 Score = " << (double)hit / (double)tot << endl;
    cout << "Topk ARE  = " << topk_are / (double)tot << endl;
    cout << "Topk AAE  = " << topk_aae / (double)tot << endl;
    cout << "Topk Zero Error Rate  = " << zero_error / (double)tot << endl;
    cout << "Throughput  = " <<(double)(1000)*(double)(packet_num)/(double)delay << endl; 
    return topk_are / (double)tot;
}
int main(){

    int packet_num = load_data("/share/datasets/CAIDA2018/dataset/130000.dat");
    double mem_in_bytes = 100 * 1024;
    double res = demo_ss<1000>(mem_in_bytes, packet_num);
    return 0;
}
