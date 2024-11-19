# Author: Trevor Mathisen

email: trevor.mathisen@sjsu.edu

last modified: 9/25/2023


## **How to run test cases:**

### Run:

   * `gcc -o matrixmult_parallel matrixmult_parallel.c -Wall -Werror`
   * `./matrixmult_parallel test/A1.txt test/W1.txt test/B1.txt`
     * Expected output: 
        ```
       Result of A*W:
        11  7 0 0 0 11 0 0
        21 13 0 0 0 21 0 0
        16 10 0 0 0 16 0 0
        16 10 0 0 0 16 0 0
        21 13 0 0 0 21 0 0
        26 16 0 0 0 26 0 0
        26 16 0 0 0 26 0 0
        16 10 0 0 0 16 0 0

        Runtime:   0.003262508 seconds
       ```
     * Runtimes:
       * Runtime 1: 0.001308565 seconds
       * Runtime 2: 0.003262508 seconds
       * Runtime 3: 0.008084905 seconds
       * Average:   0.004218659 seconds

# Optionally

   * you can run `./run_tests.sh` which:
      * does compilation using `gcc -o matrixmult_parallel matrixmult_parallel.c -Wall -Werror`
      * run test with given input files
      * does not validate output.
      * checks for error handling of an invalid file.
      * checks if != 2 arguments are passed to the program.

## This repository contains the following files:

* `matrixmult_parallel.c` - The main program that performs the matrix multiplication.

* `run_tests.sh` - A bash script that runs all 3 tests with console outputs in 1 go.

* `README.md` - This file.

* `test/` - A directory containing the test case