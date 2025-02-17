#include <cstring>
#include <iostream>
#include <map>
#include <vector>
#include <assert.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>
#include <functional>

using namespace std;

using TheoreticalProbabilityFunc = function<double(double)>;

// double theoretical_probability_ours_tower(double epsilon) {
//     double phi_P_miss = 1;
//     double l = 238933;
//     return 1 - pow((phi_P_miss / epsilon / l), 3) / 8;
//     // level 0, w 955733, h 1, counter_len 2
//     // level 1, w 477866, h 1, counter_len 4
//     // level 2, w 238933, h 1, counter_len 8
// }

double theoretical_probability_ours_tower(double epsilon, double phi_P_miss, double l) {
    cout << phi_P_miss << " " << epsilon << " " << l << endl;
    return 1 - pow((phi_P_miss / epsilon / l), 3) / 8;
}

double theoretical_probability_ours_cm(double epsilon, double phi_P_miss, double l) {
    return 1 - pow((phi_P_miss / epsilon / l), 3);
}

double theoretical_probability_elastic_cm(double epsilon, double eta, double l) {
    return 1 - pow((eta / epsilon / l), 3);
}

double theoretical_probability_elastic_tower(double epsilon, double eta, double l) {
    return 1 - pow((eta / epsilon / l), 3) / 8;
}

double theoretical_probability_ladderfilter_cu(double epsilon, double n, double w_prime) {
    return 1 - 1.0 / (epsilon * epsilon * n * w_prime);
}


class error_distribution_record {
public:
// cost query time. dont use if need to estimate time comsumption
    int total_stream_size;
    vector<double> error_ratios;
    map<int, int> error_freqs;
    string save_path;
    TheoreticalProbabilityFunc TPF;

    error_distribution_record() {
        total_stream_size = 0;
        error_ratios.clear();
        for (int i = 6; i <= 20; i+=1) {
            double ratio = pow(10, -6);
            error_ratios.push_back(ratio * i);
        }
        error_freqs.clear();
        save_path = "./error_distribution.csv";
        // TPF = funcPtr;
    }

    void clear() {
        total_stream_size = 0;
        error_freqs.clear();
    }

    void set_func(TheoreticalProbabilityFunc funcPtr) {
        TPF = funcPtr;
    }

    void insert_error(double error) {
        assert(false);
    }

    void insert_error(int error) {
        error_freqs[error]++;
        total_stream_size++;
    }

    void dump_error_distribution(int packet_num) {
        FILE *pf = fopen(save_path.c_str(), "w");
        // column: different error ratio
        // row 1: estimated probability
        // row 2: theoretical probability
        fprintf(pf, "P_type\\epsilon,");
        for (auto &itr: error_ratios) {
            fprintf(pf, "%.7lf,", itr);
        }
        fprintf(pf, "\n");
        // calculate estimated probability
        fprintf(pf, "Estimated,");
        int sum = 0, current_error_ratio_index = 0;
        for (auto &itr: error_freqs) {
            if (current_error_ratio_index >= error_ratios.size()) {
                break;
            }
            // printf("current itr error_freq: %d %d\n", itr.first, itr.second);
            int error_ratio_to_freq = packet_num * error_ratios[current_error_ratio_index];
            sum += itr.second;
            while (itr.first >= error_ratio_to_freq) {
                fprintf(pf, "%.7lf,", (double)sum / total_stream_size);
                current_error_ratio_index++;
                if (current_error_ratio_index >= error_ratios.size()) {
                    break;
                }
                error_ratio_to_freq = packet_num * error_ratios[current_error_ratio_index];
                // cout << "freq" << " " << error_ratio_to_freq << endl;
            }
        }
        while (current_error_ratio_index < error_ratios.size()) {
            fprintf(pf, "%.7lf,", (double)sum / total_stream_size);
            current_error_ratio_index++;
        }
        // calculate theoretical probability
        fprintf(pf, "\nTheoretical,");
        for (auto &itr: error_ratios) {
            fprintf(pf, "%.7lf,", TPF(itr));
            // printf("Theoretical: %lf -> %lf\n", itr, TPF(itr));
        }
        fprintf(pf, "\n");
        fclose(pf);
    }
}record;
