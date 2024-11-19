#!/bin/bash

echo "Deleting the binary"
rm -f ./matrixmult_multiw

# Check if ./matrixmult_parallel exists
if [ ! -f "./matrixmult_multiw" ]; then
    # If it doesn't exist, compile it
    gcc -o matrixmult_multiw matrixmult_multiw.c -Wall -Werror
fi

# Check for execute permissions, and add them if not present
if [ ! -x "./matrixmult_multiw" ]; then
    chmod +x ./matrixmult_multiw
    chmod +x ./run_tests.sh
fi

for i in 1 2 3; do
    echo "Running Test with A${i}.txt, W1.txt, W2.txt, W3.txt"
    ./matrixmult_multiw "test/A${i}.txt" "test/W1.txt" "test/W2.txt" "test/W3.txt"
    echo "Running Test with A${i}.txt, W1.txt, W2.txt, W3.txt"
    echo "----------------------------------------------"
done

# Test with fewer than 3 command line arguments
echo "Running Test with fewer than 2 arguments"
./matrixmult_parallel "test/A1.txt"
echo "Completed Test with fewer than 3 arguments"
echo "----------------------------------------------"

# Test with a file that does not exist
echo "Running Test with a file that does not exist"
./matrixmult_parallel "doesnt_exist.txt" "test/W1.txt"
echo "Completed Test with a file that does not exist"
echo "----------------------------------------------"

# Test with a file that does not exist (child)
echo "Running Test with a file that does not exist (child)"
./matrixmult_parallel "test/A1.txt" "test/W1.txt" "test/W2.txt" "test/doesnt_exist.txt"
echo "Completed Test with a file that does not exist (child)"
echo "----------------------------------------------"

echo "Deleting the binary"
rm -f ./matrixmult_multiw

# Check if ./matrixmult_multiw exists
if [ ! -f "./matrixmult_multiw" ]; then
    # If it doesn't exist, compile it
    echo "Properly deleted matrixmult_multiw, exiting."
fi
