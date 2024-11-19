#!/bin/bash

echo "Deleting the binary"
rm -f ./matrixmult_parallel

# Check if ./matrixmult_parallel exists
if [ ! -f "./matrixmult_parallel" ]; then
    # If it doesn't exist, compile it
    gcc -o matrixmult_parallel matrixmult_parallel.c -Wall -Werror
fi

# Check for execute permissions, and add them if not present
if [ ! -x "./matrixmult_parallel" ]; then
    chmod +x ./matrixmult_parallel
    chmod +x ./run_tests.sh
fi

# Loop through the numbers 1, 2, and 3 to test matrix multiplication
echo "Running Test with A.txt, W.txt"
./matrixmult_parallel "test/A.txt" "test/W.txt"
echo "Completed Test with A.txt, W.txt"
echo "----------------------------------------------"


# Test with fewer than 3 command line arguments
echo "Running Test with fewer than 2 arguments"
./matrixmult_parallel "test/A1.txt"
echo "Completed Test with fewer than 3 arguments"
echo "----------------------------------------------"

# Test with more than 3 command line arguments
echo "Running Test with more than 2 arguments"
./matrixmult_parallel "test/A1.txt" "test/W1.txt" "extra_arg"
echo "Completed Test with more than 3 arguments"
echo "----------------------------------------------"

# Test with a file that does not exist
echo "Running Test with a file that does not exist"
./matrixmult_parallel "doesnt_exist.txt" "test/W1.txt"
echo "Completed Test with a file that does not exist"
echo "----------------------------------------------"

echo "Deleting the binary"
rm -f ./matrixmult_parallel

# Check if ./matrixmult_parallel exists
if [ ! -f "./matrixmult_parallel" ]; then
    # If it doesn't exist, compile it
    echo "Properly deleted, exiting."
fi
