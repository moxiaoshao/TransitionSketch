编译命令

    g++ -pthread test.cpp -o test -std=c++17

执行

    ./test

mutex_pool.h

    _LOCK_STAGE_LEVEL_

        控制加锁的程度
            0：完全不控制多线程冲突，意味着高误差和高吞吐
            1：只控制在lru lfu部分的资源冲突，不控制tower部分的冲突
            2：严格控制lru lfu以及tower部分的资源冲突，意味着低误差和低吞吐

test.cpp

    line 152

        枚举线程数
            目前测试了1，2，4线程的结果
            line 69：test_thread_number等于1的时候测的就是原本的单线程的结果，不包含任何多余的信号量操作

    line 148, line 154

        枚举加锁的粒度
            目前测试了2^0, 2^1, 2^2, ..., 2^19
            mutex_num=m意味着在lru lfu层开了m个信号量控制资源冲突
            信号量越多意味着冲突越少吞吐量越高，但占用更多的额外memory
            因为目前_LOCK_STAGE_LEVEL_=1比0和2明显优（0牺牲精度太多，2牺牲吞吐量太多）
            所以只测试了lru lfu的信号量参数。测level=2时可以考虑进一步枚举tower的信号量数量找最优
            （目前tower的默认信号量每层开到64仍然很慢，因为为了避免死锁使用了原子化地加tower各层的锁【TSketch_multithread：line 83, line 98】）

    line 98

        记录不同参数下的吞吐量。在精度实验时可以改为记录精度