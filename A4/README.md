# Author: Trevor Mathisen

email: trevor.mathisen@sjsu.edu

last modified: 11/62023

## **How to run test cases:**

### Run:

   * `gcc -o matrixmult_parallel matrixmult_parallel.c -Wall -Werror` to compile A2 code
   * `gcc -o matrixmult_multiw_deep matrixmult_multiw_deep.c -Wall -Werror` to compile A4 code
   * `./matrixmult_multiw_deep test/A1.txt test/W1.txt test/W2.txt test/W3.txt < cmds.txt`
   * Expected output (in each .out): 
      ```
          Starting command 1: child xxxx pid of parent xxxx
          test/A1.txt (OR RSUM)=[
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
       * Runtime 1: 0.026356162 seconds
       * Runtime 2: 0.138733711 seconds
       * Runtime 3: 0.027252993 seconds
       * Average:   0.064114289 seconds


## This repository contains the following files:

* `matrixmult_deep.c` - The main code for completing matrix multiplication (A4)

* `matrixmult_parallel.c` - The code for each matrix multiplication child

* `cmds.txt` - The given commands from the professor for test case running

* `README.md` - This file.

* `test/` - A directory containing the test case