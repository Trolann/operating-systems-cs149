#!/bin/bash

# Check if ./matrixmult exists
if [ ! -f "./matrixmult" ]; then
    # If it doesn't exist, compile it
    gcc -o matrixmult matrixmult.c -Wall -Werror
fi

# Check for execute permissions, and add them if not present
if [ ! -x "./matrixmult" ]; then
    chmod +x ./matrixmult
fi

# Loop through the numbers 1, 2, and 3 to test matrix multiplication
for i in 1 2 3; do
    echo "Running Test with A${i}.txt, W${i}.txt, B${i}.txt"
    ./matrixmult "test/A${i}.txt" "test/W${i}.txt" "test/B${i}.txt"
    echo "Completed Test with A${i}.txt, W${i}.txt, B${i}.txt"
    echo "----------------------------------------------"
done

# Test with fewer than 3 command line arguments
echo "Running Test with fewer than 3 arguments"
./matrixmult "test/A1.txt" "test/W1.txt"
echo "Completed Test with fewer than 3 arguments"
echo "----------------------------------------------"

# Test with more than 3 command line arguments
echo "Running Test with more than 3 arguments"
./matrixmult "test/A1.txt" "test/W1.txt" "test/B1.txt" "extra_arg"
echo "Completed Test with more than 3 arguments"
echo "----------------------------------------------"

# Test with a file that does not exist
echo "Running Test with a file that does not exist"
./matrixmult "doesnt_exist.txt" "test/W1.txt" "test/B1.txt"
echo "Completed Test with a file that does not exist"
echo "----------------------------------------------"

echo "Deleting the binary"
rm -f ./matrixmult

# Check if ./matrixmult exists
if [ ! -f "./matrixmult" ]; then
    # If it doesn't exist, compile it
    echo "Properly deleted, exiting."
fi
