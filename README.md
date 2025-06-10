# AlignSketch

## Introduction
- The `final` folder contains the implementation and tests of AlignSketch.
- The main experiments and the algorithms used for comparison are presented here.

## Information on the Dataset Used in This Project

- **[WebDocs]**
The Webpage dataset consists of a large number of HTML web documents~\cite{web-page}, with a total of approximately 3.3×10^7 items, among which there are 9.5×10^5 distinct items. The maximum frequency of any distinct item is 1.6×10^5.
    
- **[CAIDA]**
We use the CAIDA dataset~\cite{caida}, a public dataset containing anonymized real-world network traces from high-speed Internet backbone links. The test data contains a total of 4.2×10^8 items, with 1.6×10^6 distinct items. The maximum frequency of any distinct item is 3.7×10^6.

- **[Zipf 1.0]**
We generate a synthetic dataset following a Power-law distributed~\cite{web-polygraph}, with a total of 1.6×10^7 items. The dataset contains 8.7×10^5 distinct items, and the maximum frequency of distinct items is 1.1×10^6. The dataset with skewness 𝛼=1.0.

- **[Normal]**
 A synthetic dataset with 𝑋 ∼ N (3.2×10^3, 1.5× 10^7) is generated following a Normal distribution, containing a total of 2×10^8 items. The dataset contains 6.3×10^4 distinct items, and the maximum frequency of distinct item is 1.2×10^4. 


## How to Build and Run the Tests

- **Compilation**: To compile the test code for AlignSketch, run:

  ```shell
  g++ test.cpp -o test -O2
  ./test
  ```

### Multithreaded Version
To find information on running the multithreaded version of this algorithm, please refer to the **readme.txt** file.



