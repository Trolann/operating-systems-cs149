# Author: Trevor Mathisen

email: trevor.mathisen@sjsu.edu

last modified: 11/13/2023


## **How to run test cases:**

### Run:

   * `gcc -o matrixmult_parallel matrixmult_parallel.c -Wall -Werror` to compile A2 code
   * `gcc -o matrixmult_multiwa matrixmult_multiwa.c -Wall -Werror` to compile A3 code
   * `./matrixmult_multiwa test/A1.txt test/W1.txt test/W2.txt test/W3.txt < cmds.txt`
   * Expected output (in each .out): 
      ```
        Starting command 1: child xxx pid of parent xxx
        test/A1.txt x test/W1.txt
        test/A2.txt x test/W1.txt
        test/A3.txt x test/W1.txt
        test/A1.txt x test/W1.txt
        test/A2.txt x test/W1.txt
        test/A3.txt x test/W1.txt
        
        rMatrix for 6 A matrices=[
                   ...
        ]
        Finished child xxx pid of parent xxx
        Exited with exitcode = 0

     ```
     * Runtimes:
       * Runtime 1: 0.020153406 seconds
       * Runtime 2: 0.048500313 seconds
       * Runtime 3: 0.089253280 seconds
       * Average:   0.052635666 seconds


## This repository contains the following files:

* `matrixmult_multiwa.c` - The main code for completing A5

* `matrixmult_parallel.c` - The code for each matrix multiplication child

* `README.md` - This file.

* `test/` - A directory containing the test case