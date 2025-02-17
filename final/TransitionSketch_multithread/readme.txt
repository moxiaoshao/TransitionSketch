Compilation Command:

    g++ -pthread test.cpp -o test -std=c++17

Execution:

    ./test

mutex_pool.h

    _LOCK_STAGE_LEVEL_

        Controls the level of locking:
            0: No control over multithreading conflicts, leading to high error and high throughput.
            1: Controls resource conflicts only in the LRU and LFU parts, not in the tower part.
            2: Strict control over resource conflicts in both LRU, LFU, and tower parts, resulting in low error and low throughput.

test.cpp

    Line 152

        Enumerates the number of threads:
            Currently tested with 1, 2, and 4 threads.
            Line 69: When test_thread_number equals 1, it measures the original single-threaded result without additional semaphore operations.

    Line 148, Line 154

        Enumerates the granularity of locking:
            Currently tested with 2^0, 2^1, 2^2, ..., 2^19.
            mutex_num=m means m semaphores are used at the LRU LFU level to control resource conflicts.
            More semaphores mean fewer conflicts and higher throughput, but more additional memory usage.
            Currently, _LOCK_STAGE_LEVEL_=1 is clearly superior to 0 and 2 (0 sacrifices too much accuracy, 2 sacrifices too much throughput).
            Therefore, only the semaphore parameters for LRU LFU are tested. When testing level=2, consider further enumerating the number of semaphores for the tower to find the optimal setting.
            Currently, the default number of semaphores for the tower is 64 per layer, which is still slow due to the use of atomic locking of each tower layer to avoid deadlocks [TSketch_multithread: line 83, line 98].

    Line 98

        Records throughput under different parameters. During accuracy experiments, it can be changed to record accuracy.
