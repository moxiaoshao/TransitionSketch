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
#include "CUSketch.h"
#include "SF_CUSketch.h"
#include "SC_CUSketch.h"
#include "LF_CUSketch.h"

// Bob hash
#include "BOBHash32.h"
#include "CUSketch.h"
#include "SPA.h"
// SIMD 
#include <immintrin.h>

using namespace std;

#define MAX_INSERT_PACKAGE 32768000
#define percent 10

unordered_map<uint32_t, int> ground_truth;
uint32_t insert_data[MAX_INSERT_PACKAGE];
uint32_t query_data[MAX_INSERT_PACKAGE];
FILE* SS_AAE;
FILE* SS_ARE;
FILE* SS_TPT;

int thres1=2;
int thres2=1;


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

template<uint32_t meminkb>
void demo_cu(int packet_num)
{
    auto sf_cu = new CUSketchWithSF<meminkb * 1024, percent>((meminkb * 1024*0.01*percent / (8 * 4)) * 0.99, (meminkb * 1024*0.01*percent  / (8 * 4)) * 0.01,
                                          8, 24, 8,
                                          thres1, thres2,
                                          750, 800);
    timespec dtime1, dtime2;
    long long delay = 0.0;

    double tot_ae;
    double tot_ar;

    // build and query for cu + LadderFilter
    clock_gettime(CLOCK_MONOTONIC, &dtime1);
    for (int i = 0; i < packet_num; ++i) {
        sf_cu->insert(insert_data[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &dtime2);
    delay = (long long)(dtime2.tv_sec - dtime1.tv_sec) * 1000000000LL + (dtime2.tv_nsec - dtime1.tv_nsec);
    sf_cu->synchronize(); // refresh SIMD buffer

    tot_ae = 0;
    tot_ar = 0;
    int cnt = 0;
    for (auto itr: ground_truth) {
        int report_val = sf_cu->query(itr.first);
        int old=tot_ae;
        tot_ae += abs(report_val - itr.second);
        if(report_val == itr.second) cnt ++;
        tot_ar += (double)(abs(report_val - itr.second))/(double)itr.second;
        if(tot_ae<old)
        {
            printf("overflow!!!\n");
        }
    }
    cout << "ARE = " << double(tot_ar) / ground_truth.size() << '\n';
    cout << "AAE = " << double(tot_ae) / ground_truth.size() << '\n'; 
    cout << "Zero Error Rate = " << cnt / ground_truth.size() << endl;
    cout << "Throughput = " << (double)(1000)*(double)(packet_num)/(double)delay << endl;
}

int main(){
    int packet_num = load_data("/share/datasets/CAIDA2018/dataset/130000.dat");
    demo_cu<int(1000)>(packet_num);
    return 0;
}
