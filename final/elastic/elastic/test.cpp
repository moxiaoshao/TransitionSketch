#include <bits/stdc++.h>
#include "elastic_sketch/ElasticSketch.h"
#include "BOBHash32.h"
#define MAX_INSERT_PACKAGE 32768000
using namespace std;
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

int main(){
    int packet_num = load_data("/share/datasets/CAIDA2018/dataset/130000.dat");
    const int memory = 100 * 1024; // bytes
    ElasticSketch sketch(memory / 256, memory);
    timespec dtime1, dtime2;
    long long delay = 0.0;
    clock_gettime(CLOCK_MONOTONIC, &dtime1);
    for (int i = 0; i < packet_num; i++){
        sketch.insert(insert_data[i], 1);
    }
    clock_gettime(CLOCK_MONOTONIC, &dtime2);
    delay = (long long)(dtime2.tv_sec - dtime1.tv_sec) * 1000000000LL + (dtime2.tv_nsec - dtime1.tv_nsec);
    double ARE = 0;
    double AAE = 0;
    int cnt = 0;
    for(auto itr: ground_truth){
        int res = sketch.query(itr.first);
        if(res == itr.second) cnt ++;
        ARE += fabs((double)res - (double)itr.second) / (double)itr.second;
        AAE += fabs((double)res - (double)itr.second);
    }
    ARE /= ground_truth.size();
    AAE /= ground_truth.size();
    cout << "ARE = " << ARE << '\n';
    cout << "AAE = " << AAE << '\n'; 
    int topk = 1000;
    unordered_map<int, int> predicted_topk;
    vector<pair<int, int>>  tmp;
    for(auto itr: ground_truth){
        int res = sketch.query(itr.first);
        tmp.push_back({itr.first, res});
    }
    sort(tmp.begin(), tmp.end(), [&](pair<int, int> a, pair<int, int> b){
        if(a.second != b.second)
            return a.second > b.second;
        return a.first > b.first;
    });
    for(int i = 0; i < topk; i ++) predicted_topk[tmp[i].first] = tmp[i].second;
    tmp.clear();
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
    cout << "Topk ARE = " << topk_are / (double)tot << endl;
    cout << "Topk AAE = " << topk_aae / (double)tot << endl;
    cout << "Topk Zero Error Rate = " << zero_error / (double)tot << endl;
    cout << "Throughput = " << (double)(1000)*(double)(packet_num)/(double)delay << endl;
    return 0;
}
