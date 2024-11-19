# Author: Trevor Mathisen

email: trevor.mathisen@sjsu.edu

last modified: 10/4/2023


## **How to run test cases:**

### Run:

   * `gcc -o matrixmult_parallel matrixmult_parallel.c -Wall -Werror` to compile A2 code
   * `gcc -o matrixmult_multiw matrixmult_multiw.c -Wall -Werror` to compile A3 code
   * `./matrixmult_multiw test/A1.txt test/W1.txt test/B1.txt`
   * Expected output (in each .out): 
      ```
          Starting command 1: child xxxx pid of parent xxxx
          test/A1.txt=[
           ...            
          ]
          test/W1.txt=[
          ...
          ]
          R=[
          ...
          ]
          Finished child xxxx pid of parent xxxx
          Exited with exitcode = 0
     ```
     * Runtimes:
       * Runtime 1: 0.001941909 seconds
       * Runtime 2: 0.002514671 seconds
       * Runtime 3: 0.002747567 seconds
       * Average:   0.002401382 seconds

# Optionally

   * you can run `./run_tests.sh` which:
      * does compilation using `gcc -o matrixmult_multiw matrixmult_multiw.c -Wall -Werror`
      * run test with given input files
      * does not validate output.
      * checks for error handling of an invalid file.
      * checks if < 2 arguments are passed to the program.

## This repository contains the following files:

* `matrixmult_multiw.c` - The main code for completing A * Wi matrix multiplication (A3)

* `matrixmult_parallel.c` - The code for each matrix multiplication child

* `matrixmult_parallel` - Pre-compiled binary

* `run_tests.sh` - A bash script that runs all 3 tests with console outputs in 1 go.

* `README.md` - This file.

* `test/` - A directory containing the test case