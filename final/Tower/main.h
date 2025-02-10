#include <bits/stdc++.h>
#include "BOBHash32.h"
#include "tower.h"

using namespace std;


class Tower
{
public:

    TowerSketch *sketch;
    int bucket_num;
    ~Tower() { clear(); }

    void clear(){
        if (sketch)
            delete sketch;
    }

    Tower(double mem)
    {
        vector<uint32_t> width;
        for (int i = 0; i < width_mul_tower.size(); i++)
        {
            width.push_back((int)(mem * 1024 * width_mul_tower[i]));
        }
        sketch = new TowerSketch(width, 1, cs_tower, 0);
    }

    void build(uint32_t * items, int n){
       for(int i = 0; i < n; i ++)
            sketch->insert(items[i], 0, 1);
    }

    inline int query(uint32_t id)
    {
        return sketch->query(id, 0);
    }

  

};