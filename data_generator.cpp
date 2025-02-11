#include <vector>
#include <fstream>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

void save_flow_data(const vector<int>& flow_ids, const char* filename) {
    ofstream fout(filename, ios::binary | ios::out);
    
    for (int id : flow_ids) {
        uint32_t flow_id = static_cast<uint32_t>(id);
        fout.write(reinterpret_cast<const char*>(&flow_id), sizeof(flow_id));
    }
    
    fout.close();
}

void load_flow_data(vector<int>& flow_ids, const char* filename) {
    ifstream fin(filename, ios::binary | ios::in);

    while (true) {
        uint32_t flow_id;
        fin.read(reinterpret_cast<char*>(&flow_id), sizeof(flow_id));
        if (fin.eof()) {
            break;
        }
        flow_ids.push_back(flow_id);
    }

    fin.close();
}

// usage:
// vector<int> flows = {1, 2, 3, 1, 2};
// save_flow_data(flows, "output.bin");

template <typename T>
std::vector<T> generateNormalDistribution(T mean, T var, int num_samples) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(mean, std::sqrt(var));

    std::vector<T> samples;
    samples.reserve(num_samples);

    for (int i = 0; i < num_samples; ++i) {
        T sample = d(gen);
        // limit the sample value to [0, 1e7]
        sample = std::max(T(0.0), std::min(sample, T(1e7)));
        samples.push_back(sample);
    }

    return samples;
}
// std::vector<double> generateNormalDistribution(double mean, double var, int num_samples) {
//     std::random_device rd;
//     std::mt19937 gen(rd());
//     std::normal_distribution<> d(mean, std::sqrt(var));

//     std::vector<double> samples;
//     samples.reserve(num_samples);

//     for (int i = 0; i < num_samples; ++i) {
//         double sample = d(gen);
//         // limit the sample value to [0, 1e7]
//         sample = std::max(0.0, std::min(sample, 1e7));
//         samples.push_back(sample);
//     }

//     return samples;
// }

std::vector<int> generateZipfDistribution(int n, double alpha, int num_samples) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    double sum = 0.0;
    for (int i = 1; i <= n; ++i) {
        sum += 1.0 / std::pow(i, alpha);
    }
    double zeta = 1.0 / sum;

    std::vector<int> samples;
    samples.reserve(num_samples);

    for (int i = 0; i < num_samples; ++i) {
        double u = dis(gen);
        double cumulative_prob = 0.0;
        int k = 1;

        while (k <= n) {
            cumulative_prob += zeta / std::pow(k, alpha);
            if (u <= cumulative_prob) {
                break;
            }
            ++k;
        }

        samples.push_back(k);
    }

    return samples;
}

int main() {
    int mean = 5e6;
    int var = 1e6;
    int num_samples = 1000;

    std::vector<int> samples = generateNormalDistribution(mean, var, num_samples);

    // output first 10 samples
    for (int i = 0; i < 10; ++i) {
        std::cout << samples[i] << " ";
    }
    std::cout << std::endl;

    // output last 10 samples
    for (int i = num_samples - 10; i < num_samples; ++i) {
        std::cout << samples[i] << " ";
    }
    std::cout << std::endl;

    // save to file
    save_flow_data(samples, "normal_distribution.bin");

    // load from file
    std::vector<int> loaded_samples;
    load_flow_data(loaded_samples, "normal_distribution.bin");

    // output first 10 loaded samples
    for (int i = 0; i < 10; ++i) {
        std::cout << loaded_samples[i] << " ";
    }
    std::cout << std::endl;

    // output last 10 loaded samples
    for (int i = num_samples - 10; i < num_samples; ++i) {
        std::cout << loaded_samples[i] << " ";
    }
    std::cout << std::endl;

    return 0;
}