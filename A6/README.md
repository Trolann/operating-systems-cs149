# Author: Trevor Mathisen

email: trevor.mathisen@sjsu.edu

last modified: 11/28/2023


## **How to run test cases:**

### Run:

   * `gcc -pthread -o matrixmult_threaded matrixmult_threaded.c -D_REENTRANT -Wall -Werror` to compile A6 code
   * `gcc -o matrixmult_multiwa matrixmult_multiwa.c -Wall -Werror` to compile A5 code
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
       * Runtime 1: 0.039455648 seconds
       * Runtime 2: 0.042003900 seconds
       * Runtime 3: 0.033960829 seconds
       * Average:   0.038473459 seconds


## This repository contains the following files:

* `matrixmult_threaded.c` - The main code for A6 (threaded matrix multiplication)

* `matrixmult_multiwa.c` - The main code for from A5 used to test A6

* `README.md` - This file.

* `test/` - A directory containing the test case