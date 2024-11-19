/*
 * Description: The module implements matrix multiplication such that R = A * W + B
 * Author names: Trevor Mathisen
 * Author emails: trevor.mathisen@sjsu.edu
 * Last modified date: 9/11/2023
 * Creation date: 9/1/2023
 */

/* PROF NOTES:
 * When you read a new line from stdin with fgets, it will contain a
 * newline '\n'. You should replace it with NULL (or 0 or '\0') with
 * this command: if (buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = '\0';
 * Other methods to replace newlines with NULL can be seen on this page:
 * https://aticleworld.com/remove-trailing-newline-character-from-fgets/
 *
 *
 * For this project, we recommend using the following routines to do
 * file input and output: fopen(), fgets(), fscanf(), fclose(). Whenever
 * you use a new function like this, the first thing you should do is read
 * about it -- how else will you learn to use it properly?
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void checkFile(FILE *file, const char *filename);
void readFile(FILE *file, int rows, int cols, int matrix[][cols]);
void printArrayContents(int rows, int cols, int matrix[][cols], char name[]);
void matrix_mult(int matrixA[1][3], int matrixW[3][5], int matrixB[1][5], int matrixR[1][5]);

int main(int argc, char* argv[]) {
    /* AS1 requirements:
     * 1) Program completes matrix multiplication such that R = A * W + B
     * x2) Matricies will have fixed sizes: A[1][3], B[1][5], W[3][5], R[1][5]
     * x3) Initialize all values to 0 (for inputs with < rows than above)
     * x4) Get 3 filenames from command line arguments
     * x5) If 3 args are not provided, print 'Error - expecting exactly 3 files as input', exit 1;
     * x6) If files don't exist, print 'error: cannot open file <filename>', exit 1;
     * x7) Read rows and columns up to the size of the matrix, then ignore the rest
     *    x7a) Example of B1.txt:1 1 1 1 1
     *    x7b) Assume no empty lines between non-empty lines
     * x8) Make the matrixmult binary
     * x9) Test executable with A1/test
     * x10) Write a bash script to run the program with all the test in 1 go
     * x11) Close the files
     */

    // Initialize to 0
    int A[1][3] = {0};
    int B[1][5] = {0};
    int W[3][5] = {0};
    int R[1][5] = {0};

    // Check if 3 args are provided
    if (argc != 4) { // argv[0] is program name
        printf("Error - expecting exactly 3 files as input\n");
        return 1;
    }

    // Open the files
    FILE *fileA = fopen(argv[1], "r");
    FILE *fileW = fopen(argv[2], "r");
    FILE *fileB = fopen(argv[3], "r");

    // Check if files exist: will exit if needed
    checkFile(fileA, argv[1]);
    checkFile(fileW, argv[2]);
    checkFile(fileB, argv[3]);

    // Read the files Example of B1.txt:1 1 1 1 1
    readFile(fileA, 1, 3, A);
    readFile(fileB, 1, 5, B);
    readFile(fileW, 3, 5, W);


    // Close the files
    fclose(fileA);
    fclose(fileB);
    fclose(fileW);

    // Test print
    // printArrayContents(1, 3, A, "A");
    // printArrayContents(1, 5, B, "B");
    // printArrayContents(3, 5, W, "W");

    // Do the math
    matrix_mult(A, W, B, R);

    // Print the result
    printf("\nResult of A*W+B = [%d %d %d %d %d]\n\n", R[0][0], R[0][1], R[0][2], R[0][3], R[0][4]);

    return 0;
}

/*
 * This function checks the file and prints errors if needed
 * Assumption: file is not null, there is a filename
 * Input parameters: FILE *file, const char *filename
 * Returns: void, exits if needed
*/
void checkFile(FILE *file, const char *filename) {
    if (file == NULL) { // If there is no file, or you can't access it
        printf("error: cannot open file %s\n", filename);
        printf("Terminating, exit code 1.");
        exit(1); // End the program here, do not return
    }
}

/*
 * This function reads the file and populates the given matrix.
 * Assumption: file has been checked, matrix is already initialized, and rows and columns are known
 * Input parameters: FILE *file, int rows, int cols, int matrix[][cols]
 * Returns: void, updates matrix by reference
*/
void readFile(FILE *file, int rows, int cols, int matrix[][cols]) {
    // Initialize row and column counters
    size_t i = 0;
    size_t j = 0;

    // Read the file line by line
    char buf[100];
    while (getline(buf, sizeof(buf), file) != NULL) {

        // Remove trailing newline character
        if (buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = '\0';

        // Tokenize the line and populate the matrix
        char *token = strtok(buf, " ");
        while (token != NULL) { // Until the end
            if (i < rows && j < cols) { // Ignore other values
                matrix[i][j] = atoi(token); // Convert string to int
                j++; // Next column
            }
            token = strtok(NULL, " "); // Next one
        }

        i++; // Next row
        j = 0; // Reset column count for the new row
    }
}

/*
 * Utility function to test reading matrix values
 * Assumption: you're passing a valid matrix
 * Input parameters: int rows, int cols, int matrix[][cols], char name[]
 * Returns: void, prints rows and columns of matrix with name
*/
void printArrayContents(int rows, int cols, int matrix[][cols], char name[]) {
    size_t i, j;
    // Print the matrix
    printf("%s:\n", name);
    for (i = 0; i < rows; i++) { // For each row
        for (j = 0; j < cols; j++) { // For each column
            printf("%d ", matrix[i][j]); // Print the value
        }
        printf("\n"); // New line for each row
    }
    printf("\n");
}

/*
 * Performs R = A * W + B matrix operations
 * Assumption: you have 3 valid matrices with proper dimensions
 * Input parameters: 3 input matrices and 1 output matrix
 * Returns: void, updates matrixR by reference
*/
void matrix_mult(int matrixA[1][3], int matrixW[3][5], int matrixB[1][5], int matrixR[1][5]) {
    size_t i, j;

    // BigO go brrrrrr
    // For each row in A
    for (i = 0; i < 1; i++) {
        // For each column in W
        for (j = 0; j < 5; j++) {
            // For each column in A
            for (size_t k = 0; k < 3; k++) {
                // math math math
                matrixR[i][j] += matrixA[i][k] * matrixW[k][j];
            }
            // Add B while we're here
            matrixR[i][j] += matrixB[i][j];
        }
    }
}