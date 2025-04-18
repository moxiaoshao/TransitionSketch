## Introduction

- The `./TransitionSketch/` directory contains the implementation and tests of AlignSketch.
- The `./TransitionSketch_multithread/` directory contains the implementation and tests of multithread version of AlignSketch.
- The `./elastic/` directory contains all the tests of Elastic.
- The `./LadderFilter/` directory contains all the tests of LadderFilter.
- The `./SpaceSaving/` directory contains all the tests of SS.
- The `./UnbiasedSpaceSaving/` directory contains all the tests of USS.
- The `./Tower/` directory contains all the tests of Tower.



## How to make and run the test

- Compilations: To compile test codes of AlignSketch (multithread), just run: 

  ```shell
  cd TransitionSketch
  g++ test.cpp -o test -O2
  ./test
  ```
  ```shell
  cd TransitionSketch_multithread
  g++ -lpthread test.cpp -o test -O2 -std=c++17
  ./test
  ```

  Similarly, to compile test codes of Elastic, LadderFilter, SS, USS, Tower, just run:

  ```shell
  cd elastic/elastic
  g++ test.cpp -o test -O2
  ./test
  ```

  ```shell
  cd SpaceSaving
  g++ test.cpp -o test -O2
  ./test
  ```

  ```shell
  cd UnbiasedSpaceSaving
  g++ test.cpp -o test -O2
  ./test
  ```

  ```shell
  cd Tower
  g++ test.cpp -o test -O2
  ./test
  ```

  ```shell
  cd LadderFilter/CUSketch/src
  g++ test.cpp -o test -O2 -march=native
  ./test
  ```

  

