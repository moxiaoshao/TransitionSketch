# AlignSketch

## Introduction
- The `./TransitionSketch/` directory contains the implementation and tests of AlignSketch.
- The main experiments and their comparison algorithms are located in the `final` folder.

## Information on the Dataset Used in This Project
- **[WebDocs](http://fimi.uantwerpen.be/data/)**
  - Total stream size: \(3.3 × 10^7\) 
  - Distinct item number: \(9.5 × 10^5\)
  - Max frequency: 158,740

- **[CAIDA](https://www.caida.org/catalog/datasets/passive_dataset/)**
  - Total stream size: \(4.2 × 10^8\)
  - Distinct item number: \(1.6 × 10^6\)
  - Max frequency: 3,694,272

- **Zipf 1.0**
  - Total stream size: \(1.6 × 10^7\)
  - Distinct item number: \(8.5 × 10^5\)
  - Max frequency: 1,110,233

## How to Build and Run the Tests

- **Compilation**: To compile the test code for AlignSketch, run:

  ```shell
  cd TransitionSketch
  g++ test.cpp -o test -O2
  ./test
  ```

### Multithreaded Version
To find information on running the multithreaded version of this algorithm, please refer to the **readme.txt** file.



