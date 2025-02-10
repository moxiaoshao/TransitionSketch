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
    int counter_len = 32;
    int key_len = 32;
    double mem_in_kb = 1000;
    timespec dtime1, dtime2;
    long long delay = 0.0;
    Tower *tower = new Tower(mem_in_kb);
    clock_gettime(CLOCK_MONOTONIC, &dtime1);
    tower->build(insert_data, packet_num);
    clock_gettime(CLOCK_MONOTONIC, &dtime2);
    delay = (long long)(dtime2.tv_sec - dtime1.tv_sec) * 1000000000LL + (dtime2.tv_nsec - dtime1.tv_nsec);
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
    cout << "ARE = " << are << '\n';
    cout << "AAE = " << aae << '\n';
    cout << "Zero Error Rate = " << (double)cnt / ground_truth.size() << '\n';
    cout << "Throughput = " << (double)(1000)*(double)(packet_num)/(double)delay << '\n';
    delete tower;
    return 0;
}
