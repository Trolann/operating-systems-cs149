/*
 * Description: The module implements matrix multiplication using parallelization and is modified from A2
 *              to facilitate proper printing to out stdout and stderr.
 * Author names: Trevor Mathisen
 * Author emails: trevor.mathisen@sjsu.edu
 * Last modified date: 11/15/2023
 * Creation date: 9/11/2023
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define SIZE 8
#define READ_END 0
#define WRITE_END 1
#define MATRIX_SIZE sizeof(int) * SIZE * SIZE

/*
 * This structure is used to pass data between processes via a pipe
 * Assumption: Will be given the rowNum to store and will compute the vector
 * Input parameters: rowNum
 * Returns: Nothing, stores the rowNum and row vector
*/
struct processInfo {
    int rowNum;
    int row[SIZE];
} typedef processInfo;

// Function prototypes
void checkFile(FILE *file, const char *filename);
void readFile(FILE *file, int rows, int cols, int matrix[][cols]);
void printArrayContents(int rows, int cols, int matrix[][cols], char name[]);
processInfo computeRowDotProduct(int matrixA[SIZE][SIZE], int matrixW[SIZE][SIZE], int rowNum);

int main(int argc, char* argv[]) {
    // Initialize to 0
    int A[SIZE][SIZE] = {0};
    int W[SIZE][SIZE] = {0};
    int iterationNum = 0;

    // Check if 3 args are provided
    if (argc != 3) { // argv[0] is program name
        fprintf(stderr, "Error - expecting exactly 2 files as input\n");
        return 1;
    }

    // Open, check and read the files, close when done
    FILE *fileW = fopen(argv[2], "r");
    checkFile(fileW, argv[2]);
    readFile(fileW, SIZE, SIZE, W);
    fclose(fileW);

    // declare R as a dynamic array of SIZE * SIZE
    int **R = NULL;

    while (read(STDIN_FILENO, &A, MATRIX_SIZE) > 0) {
        // Realloc R to be (SIZE * SIZE) * iterationNum
        int oldSize = SIZE * iterationNum;
        R = realloc(R, sizeof(int *) * SIZE * (++iterationNum));
        // Allocate memory for the new rows
        for (int i = oldSize; i < SIZE * iterationNum; i++) {
            R[i] = malloc(sizeof(int) * SIZE);
        }

        char filename[100];
        sprintf(filename, " x %s\n", argv[2]);
        fprintf(stdout, "%s", filename);
        fflush(stdout);

        // Setup a pipe
        int p[2];
        pipe(p);

        // Spawn the children to compute the dot product
        for (size_t i = 0; i < SIZE; i++) {
            int row = (int) i;
            int pid = fork();
            if (pid < 0) { // Error
                fprintf(stderr, "Error - fork failed\n");
                return 1;
            }
            if (pid == 0) { // Child
                // Close read end of pipe
                close(p[READ_END]);
                // Compute the dot product
                processInfo info = computeRowDotProduct(A, W, row);
                info.rowNum = row;
                // Write the result to the pipe
                write(p[WRITE_END], &info, sizeof(info));
                // Close write end of pipe
                close(p[WRITE_END]);
                exit(0);
                // Child code ends
            }
        }

        // Close write end of pipe in parent
        close(p[WRITE_END]);

        /*
         * We must first wait() for everything to clear PCB
         * We must also read pipes for all the rows
         * If there are no more children running but we haven't read all the pipes,
         * exit, we have a problem.
         */
        size_t rowsRead = 0;
        while (rowsRead < SIZE) {

            processInfo info;
            wait(NULL);

            /*
             * Either a single process has finished or all processes have finished
             * Either way, read the pipe until it is dry
             */
            while (read(p[READ_END], &info, sizeof(info)) > 0) {
                // Store the result in R
                for (size_t i = 0; i < SIZE; i++) {
                    // Account for iterationNum to store in the right row
                    R[info.rowNum + (SIZE * (iterationNum - 1))][i] = info.row[i];
                }
                rowsRead++;
            }

        }
        // Zero out A
        memset(A, 0, MATRIX_SIZE);

        // close every pipe
        close(p[READ_END]);
        close(p[WRITE_END]);
        fflush(stdin);
        fflush(stdout);

    }
    fprintf(stdout, "\nrMatrix for %d A matrices=[\n", iterationNum);
    fflush(stdout);
    for (int i = 0; i < SIZE * iterationNum; i++) {
        for (int j = 0; j < SIZE; j++)
            fprintf(stdout, "%d ", R[i][j]);
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "]\n");
    fflush(stdout);
    // Free R and every row in it
    for (int i = 0; i < SIZE * iterationNum; i++) {
        free(R[i]);
    }
    free(R);

    return 0;
}

/*
 * This function checks the file and prints errors if needed
 * This function was updated for A3 to print to stderr
 * Assumption: file is not null, there is a filename
 * Input parameters: FILE *file, const char *filename
 * Returns: void, exits if needed
*/
void checkFile(FILE *file, const char *filename) {
    if (file == NULL) { // If there is no file, or you can't access it
        fprintf(stderr, "error: cannot open file %s\n", filename);
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
    while (fgets(buf, sizeof(buf), file) != NULL) {

        // Remove trailing newline character
        if (buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = '\0';

        // Tokenize the line and populate the matrix
        char *token = strtok(buf, " ");
        while (token != NULL) { // Until tjhe end
            if (i < rows && j < cols) { // Ignore other values
                matrix[i][j] = atoi(token); // Convert string to int
                j++; // Next column
            }
            token = strtok(NULL, " "); // Last one
        }

        i++; // Next row
        j = 0; // Reset column count for the new row
    }
}

/*
 * Utility function to test reading matrix values
 * Function updates for A3: added fprint, changed printing to include []
 * Assumption: you're passing a valid matrix
 * Input parameters: int rows, int cols, int matrix[][cols], char name[]
 * Returns: void, prints rows and columns of matrix with name
*/
void printArrayContents(int rows, int cols, int matrix[][cols], char name[]) {
    size_t i, j;

    // Print the matrix
    fprintf(stdout, "%s=[\n", name);
    for (i = 0; i < rows; i++) { // For each row
        for (j = 0; j < cols; j++) { // For each column
            if (matrix[i][j] < 10 && matrix[i][j] > 0)
                fprintf(stdout, " "); // Print a space for single digits (for formatting
            fprintf(stdout, "%d ", matrix[i][j]); // Print the value
        }
        fprintf(stdout, "\n"); // New line for each row
    }
    fprintf(stdout, "\n]\n");
}

/*
 * This function computes the dot product of a 1xN matrix with an NxN matrix
 * Assumption: sizes are compatible
 * Input parameters: int matrixA[1][SIZE], int matrixW[SIZE][SIZE], int rowNum
 * Returns: processInfo with pid, rowNum, and row vector
*/
processInfo computeRowDotProduct(int matrixA[SIZE][SIZE], int matrixW[SIZE][SIZE], int rowNum) {
    // Setup return struct
    processInfo returnInfo;
    returnInfo.rowNum = rowNum;
    for (size_t i = 0; i < SIZE; i++) {
        returnInfo.row[i] = 0;
    }

    size_t i, j;
    int sum;
    for (i = 0; i < SIZE; i++) { // For each column in W
        sum = 0; // New dot product
        for (j = 0; j < SIZE; j++) { // For each element in A
            sum += matrixA[rowNum][j] * matrixW[j][i]; // Dot product
        }
        returnInfo.row[i] = sum; // Store the dot product
    }
    return returnInfo;
}