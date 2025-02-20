#ifndef FREQ_TEST_H
#define FREQ_TEST_H

#include "utils.h"
#include "data.h"
#include "task_utils.h"
using namespace std;

/*
 * To test the performance (AAE, ARE) of elastic sketch over reporting heavy change flows.
 */
template <int memory>
void frequency_test(TRACE *traces, FLOW_ITEM *items, int item_cnt, int sketch_id, int opt, FILE *output_file)
{
    double avg_ARE = 0;
    double avg_AAE = 0;
    char sketch_name[100];
    get_sketch_name(sketch_name, sketch_id);
    printf("%s\n", sketch_name);

    for (int rep = 0; rep < REP_TIME; rep++)
    {
        printf("%d\n", rep);
        ElasticSketch<memory / 256, memory> *sketch = new ElasticSketch<memory / 256, memory>();

        insert_all_packets(sketch, traces);

        double ARE = 0;
        double AAE = 0;

        for (int i = 0; i < item_cnt; i++)
        {
            double freq = sketch->query((uint8_t *)items[i].key.key);
            ARE += fabs(freq - items[i].freq) / (double)items[i].freq;
            AAE += fabs(freq - items[i].freq);

#ifdef DEBUG_MODE
            if (items[i].freq > freq)
                printf("est_val < real_val !!!\n");
#endif
        }

        ARE /= (double)item_cnt;
        AAE /= (double)item_cnt;

        avg_ARE += ARE / (double)REP_TIME;
        avg_AAE += AAE / (double)REP_TIME;

        delete (sketch);
    }

    printf("\n");
    fprintf(output_file, "%s\n%.9f %.9f\n", sketch_name, avg_ARE, avg_AAE);
}

#endif