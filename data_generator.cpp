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

// 示例用法：
// vector<int> flows = {1, 2, 3, 1, 2};
// save_flow_data(flows, "output.bin");


std::vector<double> generateNormalDistribution(double mean, double var, int num_samples) {
    std::random_device rd;  // 随机数种子
    std::mt19937 gen(rd()); // 使用Mersenne Twister引擎
    std::normal_distribution<> d(mean, std::sqrt(var)); // 正态分布，标准差是方差的平方根

    std::vector<double> samples;
    samples.reserve(num_samples);

    for (int i = 0; i < num_samples; ++i) {
        double sample = d(gen);
        // 确保生成的样本在0到1e7之间
        sample = std::max(0.0, std::min(sample, 1e7));
        samples.push_back(sample);
    }

    return samples;
}

// 生成Zipf分布的随机数
std::vector<int> generateZipfDistribution(int n, double alpha, int num_samples) {
    std::random_device rd;  // 随机数种子
    std::mt19937 gen(rd()); // 使用Mersenne Twister引擎
    std::uniform_real_distribution<> dis(0.0, 1.0); // 均匀分布

    // 计算Zipf分布的归一化常数
    double sum = 0.0;
    for (int i = 1; i <= n; ++i) {
        sum += 1.0 / std::pow(i, alpha);
    }
    double zeta = 1.0 / sum;

    // 生成Zipf分布的样本
    std::vector<int> samples;
    samples.reserve(num_samples);

    for (int i = 0; i < num_samples; ++i) {
        double u = dis(gen); // 生成均匀分布的随机数
        double cumulative_prob = 0.0;
        int k = 1;

        // 通过逆变换法生成Zipf分布的样本
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
    double mean = 5e6; // 均值
    double var = 1e6;  // 方差
    int num_samples = 1000; // 生成的样本数量

    std::vector<double> samples = generateNormalDistribution(mean, var, num_samples);

    // 输出前10个样本
    for (int i = 0; i < 10; ++i) {
        std::cout << samples[i] << std::endl;
    }

    return 0;
}