# Author: Trevor Mathisen

email: trevor.mathisen@sjsu.edu

last modified: 9/7/2023


## **How to run test cases:**

### Run each of these:

   * `./matrixmult test/A1.txt test/W1.txt test/B1.txt`
     * Expected output: `Result of A*W+B = [111 191 391 51 11]`
   

   * `./matrixmult test/A2.txt test/W2.txt test/B2.txt`
     * Expected output: `Result of A*W+B = [701 151 191 51 11]`
   

   * `./matrixmult test/A3.txt test/W3.txt test/B3.txt`
     * Expected output: `Result of A*W+B = [11 19 39 55 55]`


   * One liner:
     * `./matrixmult test/A1.txt test/W1.txt test/B1.txt && ./matrixmult test/A2.txt test/W2.txt test/B2.txt && ./matrixmult test/A3.txt test/W3.txt test/B3.txt`
       * Expected output:
         ```
           Result of A*W+B = [111 191 391 51 11]
           
           Result of A*W+B = [701 151 191 51 11]
           
           Result of A*W+B = [11 19 39 55 55]
         ```
# Optionally

   * you can run `./run_tests.sh` which:
      * does compilation using `gcc -o matrixmult matrixmult.c -Wall -Werror`
      * run all 3 tests A1/B1/W1, A2/B2/W2, A3/B3/W3.
      * does not validate output.
      * checks for error handling of an invalid file.
      * checks if != 3 arguments are passed to the program.

## This repository contains the following files:

* `matrixmult.c` - The main program that performs the matrix multiplication.

* `run_tests.sh` - A bash script that runs all 3 tests with console outputs in 1 go.

* `README.md` - This file.

* `test/` - A directory containing the 3 test cases (9 .txt files)