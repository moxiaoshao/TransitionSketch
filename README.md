# TransitionSketch

## Introduction
- The `./TransitionSketch/` directory contains the implementation and tests of TransitionSketch.

## Information on the dataset used in this project:
- [WebDocs 95W](http://fimi.uantwerpen.be/data/)
Total stream size = 32768000
Distinct item number = 953165
Max frequency = 158740

- [CAIDA 160W](https://www.caida.org/catalog/datasets/passive_dataset/)
Total stream size = 4.2x10^8
Distinct item number = 1.6x10^6
Max frequency = 3694272

- Zipf1.0
Total stream size = 16000000
Distinct item number = 874919
Max frequency = 1110233

## How to make and run the test

- Compilations: To compile test codes of TransitionSketch, just run: 

  ```shell
  cd TransitionSketch
  g++ test.cpp -o test -O2
  ./test
  ```




