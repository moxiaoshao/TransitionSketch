#ifndef HEAVY_CHANGE_H
#define HEAVY_CHANEG_H

#include "utils.h"
#include "data.h"
#include "task_utils.h"
using namespace std;

/*
 * To test the performance (F_1 Score) of elastic sketch over reporting heavy change flows.
 * Here, "heavy change flows" are defined as: {e_i| |f_i - f'_i| >= total * threshold}
 */
template <int memory>
void heavy_change_test(TRACE* traces, FLOW_ITEM* items, int item_cnt, int sketch_id, int opt, int threshold, FILE* output_file, char* extra_file_name){
    double avg_PR = 0;
    double avg_RC = 0;
    double avg_ARE = 0;
    double avg_F1 = 0;

    char sketch_name[100];
    get_sketch_name(sketch_name, sketch_id);
    printf("%s\n", sketch_name);

    TRACE* traces2;
    FLOW_ITEM* items2;
    int item_cnt2;

    ReadInTraces(extra_file_name, traces2, items2, item_cnt2);

    for (int rep = 0; rep < REP_TIME; rep++){
        printf("%d\n", rep);
        ElasticSketch<memory / 256, memory>* sketch = new ElasticSketch<memory / 256, memory>();

        insert_all_packets(sketch, traces);
        
        ElasticSketch<memory / 256, memory>* sketch2 = new ElasticSketch<memory / 256, memory>();

        insert_all_packets(sketch2, traces2);

        vector<pair<string, int> > result1;
        vector<pair<string, int> > result2;
        unordered_map<FIVE_TUPLE, int, Hash_Fun>* result = new unordered_map<FIVE_TUPLE, int, Hash_Fun>;
        result->clear();
        sketch->get_heavy_hitters(threshold, result1);
        sketch2->get_heavy_hitters(threshold, result2);

        unordered_map<FIVE_TUPLE, int, Hash_Fun>::iterator it;
        for (int i = 0; i < result1.size(); i++){
            FIVE_TUPLE tmp;
            for (int j = 0; j < 13; j++){
                tmp.key[j] = 0;
            }
            memcpy(tmp.key, result1[i].first.c_str(), 4);
            (*result)[tmp] = sketch->query((uint8_t*)tmp.key) - sketch2->query((uint8_t*)tmp.key);
        }
        for (int i = 0; i < result2.size(); i++){
            FIVE_TUPLE tmp;
            for (int j = 0; j < 13; j++){
                tmp.key[j] = 0;
            }
            memcpy(tmp.key, result2[i].first.c_str(), 4);
            (*result)[tmp] = sketch->query((uint8_t*)tmp.key) - sketch2->query((uint8_t*)tmp.key);
        }

        unordered_map<FIVE_TUPLE, int, Hash_Fun>* answer = new unordered_map<FIVE_TUPLE, int, Hash_Fun>;
        answer->clear();

        for (int i = 0; i < item_cnt; i++){
            (*answer)[items[i].key] += items[i].freq;
        }
        for (int i = 0; i < item_cnt2; i++){
            (*answer)[items2[i].key] -= items2[i].freq;
        }

        int tot_real = 0, tot_report = 0, hit = 0;
        double ARE = 0;
        for (it = answer->begin(); it != answer->end(); it++){
            int real_freq = abs(it->second);
            if (real_freq >= threshold){
                tot_real++;
                int est_freq = abs((*result)[it->first]);
                if (est_freq >= threshold){
                    hit++;
                    ARE += fabs(est_freq - real_freq) / (double)real_freq;
                }
            }
        }

        for (it = result->begin(); it != result->end(); it++){
            if (abs(it->second) >= threshold){
                tot_report++;
            }
        }

        avg_PR += (double)(hit) / (double)tot_report / (double)REP_TIME;
        avg_RC += (double)(hit) / (double)tot_real / (double)REP_TIME;
        avg_ARE += ARE / (double)REP_TIME / (double)hit;
        avg_F1 += 2 * (double)hit / (double)(tot_report + tot_real) / (double)REP_TIME;

        delete(sketch);
        delete(result);
        delete(sketch2);
        delete(answer);
    }

    printf("\n");
    fprintf(output_file, "%s\n%.9f %.9f %.9f %.9f\n", sketch_name, avg_PR, avg_RC, avg_F1, avg_ARE);

    delete(traces2);
    delete[] items2;
}

#endif
