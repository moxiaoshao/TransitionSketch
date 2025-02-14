#include <bits/stdc++.h>
#include "main.h"
#include "BOBHash32.h"
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
        if (ret == MAX_INSERT_PACKAGE){
            cout << "MAX_INSERT_PACKAGE" << endl;
            break;
        }
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


void freq_test(double mem, double per, int packet_num, int cols, int key_len, int counter_len, int rand_seed){
    timespec dtime1, dtime2;
    int bucket_num = (mem * per) * 1024 / cols / (key_len / 8 + counter_len / 8) / 2;
    TransitionSketch *tower = new TransitionSketch(mem, bucket_num, cols, counter_len, rand_seed, 1 - per);
    clock_gettime(CLOCK_MONOTONIC, &dtime1);
    vector<int> tmp = tower->build(insert_data, packet_num);
    clock_gettime(CLOCK_MONOTONIC, &dtime2);
    double delay = (long long)(dtime2.tv_sec - dtime1.tv_sec) * 1000000000LL + (dtime2.tv_nsec - dtime1.tv_nsec);
    int cnt = 0;
    double are = 0, aae = 0;
    for(auto itr: ground_truth){
        int res =  tower->query(itr.first);
        if(res - itr.second == 0) cnt ++;
        are += fabs((double)res - (double)itr.second) / (double)itr.second;
        aae += fabs((double)res - (double)itr.second);
    }
    are = are / ground_truth.size();
    aae = aae / ground_truth.size();
    cout << "Frequency test result:" << '\n';
    cout << "ARE = " << are << '\n';
    cout << "AAE = " << aae << '\n';
    cout << "Zero Error Rate = " << (double)cnt / ground_truth.size() << '\n';
    cout << "Throughput = " << (double)(1000)*(double)(packet_num)/(double)delay << '\n';
    delete tower;
}
void topk_test(int topk, double mem, double per, int packet_num, int cols, int key_len, int counter_len, int rand_seed){
    timespec dtime1, dtime2;
    int bucket_num = (mem * per) * 1024 / cols / (key_len / 8 + counter_len / 8) / 2;
    TransitionSketch *tower = new TransitionSketch(mem, bucket_num, cols, counter_len, rand_seed, 1 - per);
    clock_gettime(CLOCK_MONOTONIC, &dtime1);
    tower->build(insert_data, packet_num);
    clock_gettime(CLOCK_MONOTONIC, &dtime2);
    double delay = (long long)(dtime2.tv_sec - dtime1.tv_sec) * 1000000000LL + (dtime2.tv_nsec - dtime1.tv_nsec);
    unordered_map<int, int> predicted_topk = tower->query_topk(topk);
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
    cout << "Topk test result:" << '\n';
    cout << "TopkF1 Score = " << (double)hit / (double)tot << endl;
    cout << "Topk ARE  = " << topk_are / (double)tot << endl;
    cout << "Topk AAE  = " << topk_aae / (double)tot << endl;
    cout << "Topk Zero Error Rate  = " << zero_error / (double)tot << endl;
    cout << "Throughput = " << (double)(1000)*(double)(packet_num)/(double)delay << '\n';
}

int main(){
    int topk = 1000;
    int packet_num = load_data("/share/datasets/CAIDA2018/dataset/130000.dat");
    double per = 0.3;
    int cols = 4;
    int key_len = 32;
    int counter_len = 24;
    int rand_seed = 750;
    double mem_in_kb = 1000;
    freq_test(mem_in_kb, per, packet_num, cols, key_len, counter_len, rand_seed);
    cout << '\n';
    topk_test(topk, mem_in_kb, per, packet_num, cols, key_len, counter_len, rand_seed);
    return 0;
}
