/*
 * Description: The module implements matrix multiplication using parallelization and is modified from A2
 *              to facilitate proper printing to out stdout and stderr.
 * Author names: Trevor Mathisen
 * Author emails: trevor.mathisen@sjsu.edu
 * Last modified date: 10/11/2023
 * Creation date: 9/11/2023
 */

/*
 * x1) Extend matrixmul.c to compute array multiplication in parallel
 * x1a) For example, the ith child process will compute dot products of the ith row of Ai against W to return the vector Ai * W
 * x2) takes the input files A, W on the command line
 * x3) Fixed size arrays: 8x8
 * x4) Compute A & W using 8 processes
 * x4a) Process i will multiply the Ai row vector by all columns of W (it's called dot product).
 * x5) You should use fork() to start a process for each of the rows of A, such that we can compute a vector
 *    for each row of the result individually in parallel. To communicate between processes you may use pipe().
 *    You need to wait for the processes to finish using wait().
 * x6) In order to keep track of which row of the result a child process computed, you could either return i as an
 *    int in a pipe; or have an array that maps i to pid, since pid is returned from wait; or something else...
 *    NOTE: I did something else (struct)
 * x7) You can assume that the files contain only valid ASCII characters separated by newlines , which can be converted to integers.
 * x8: Error handling:
                    x1) In all non-error cases, matrixmult_parallel should exit with status code 0, usually by returning a
                       0 from main() (or by calling exit(0)). This is called the exit code of a program and is different
                       from what your program prints to stdout/stderr.
                    x2) If no file or other parameters are specified on the command line, matrixmult_parallel should
                       print the stderr message "error: expecting exactly 2 files as input" and exit and return 1.
                    x3) If the program tries to fopen() a file and fails, it should print the stderr message "error:
                       cannot open file myfile.txt" (followed by a newline) and exit with status code 1.
 */

/* EXAMPLE:
 * For example, running on these test files will output:

    $ ./matrixmult_parallel A.txt W.txt

    which will output to stdout:

    Result of A*W = [...]
    Runtime 0.01 seconds
    (it is fine if you wish to output additional details, like the absolute paths to A W)

    The error codes are the same as in assignment 1 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>

#define SIZE 8

/*
 * This structure is used to pass data between processes via a pipe
 * Assumption: Will be given the rowNum to store and will compute the vector
 * Input parameters: rowNum
 * Returns: Nothing, stores the rowNum and row vector
*/
struct processInfo {
    int rowNum;
    int row[SIZE];
};

// Function prototypes
void checkFile(FILE *file, const char *filename);
void readFile(FILE *file, int rows, int cols, int matrix[][cols]);
void printArrayContents(int rows, int cols, int matrix[][cols], char name[]);
struct processInfo computeRowDotProduct(int matrixA[SIZE][SIZE], int matrixW[SIZE][SIZE], int rowNum);

int main(int argc, char* argv[]) {
    struct timespec start, finish;
    char *pipeToParent = getenv("PIPE");  // Set by parent. Pipe used to send results to parent, clean
    int pipeWrite;
    // time_t elapsed; // Removed for A3
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Initialize to 0
    int A[SIZE][SIZE] = {0};
    int W[SIZE][SIZE] = {0};
    int R[SIZE][SIZE] = {0};

    // Check if 3 args are provided
    if (argc != 3) { // argv[0] is program name
        fprintf(stderr, "Error - expecting exactly 2 files as input\n");
        return 1;
    }

    if (!pipeToParent) {
        fprintf(stderr, "PIPE not set\n");
        return 1;
    }

    pipeWrite = atoi(pipeToParent);

    // Read in 64 ints from stdin into a new SIZExSIZE matrix
    int stdinMatrix[SIZE][SIZE] = {0};
    //fprintf(stdout, "reading from pipe: \n");
    read(STDIN_FILENO, &stdinMatrix, sizeof(int) * SIZE * SIZE);
    //for (size_t i = 0; i < SIZE; i++) {
    //    for (size_t j = 0; j < SIZE; j++) {
    //        read(STDIN_FILENO, &stdinMatrix[i][j], sizeof(int));
    //        //fprintf(stdout, "%d ", stdinMatrix[i][j]);
    //    }
    //    //fprintf(stdout, "\n");
    //    //fflush(stdout);
    //}

    // Open, check and read the files, close when done
    FILE *fileW = fopen(argv[2], "r");
    checkFile(fileW, argv[2]);
    readFile(fileW, SIZE, SIZE, W);
    fclose(fileW);

    if(stdinMatrix[0][0] < 0) {
        // Only open A.txt if this is the first iteration.
        FILE *fileA = fopen(argv[1], "r");
        checkFile(fileA, argv[1]);
        readFile(fileA, SIZE, SIZE, A);
        fclose(fileA);
        printArrayContents(SIZE, SIZE, A, argv[1]);
    } else {
        // Copy stdinMatrix to A, print it with a proper name
        for (size_t i = 0; i < SIZE; i++) {
            for (size_t j = 0; j < SIZE; j++) {
                A[i][j] = stdinMatrix[i][j];
            }
        }
        printArrayContents(SIZE, SIZE, A, "RSUM");
    }
    fflush(stdin);

    // Stream needs to be flushed so children don't write the stream. Added for A3
    printArrayContents(SIZE, SIZE, W, argv[2]);
    fflush(stdout);

    // Setup a pipe
    int p[2];
    pipe(p);

    // Spawn the children

    for (size_t i = 0; i < SIZE; i++) {
        int row = (int) i;
        int pid = fork();
        if (pid < 0) { // Error
            fprintf(stderr, "Error - fork failed\n");
            return 1;
        }
        if (pid == 0) { // Child
            // Close read end of pipe
            close(p[0]);
            // Compute the dot product
            struct processInfo info = computeRowDotProduct(A, W, row);
            info.rowNum = row;
            // Write the result to the pipe
            write(p[1], &info, sizeof(info));
            // Close write end of pipe
            close(p[1]);
            exit(0);
            // Child code ends
        }
    }

    // Close write end of pipe in parent
    close(p[1]);

    /*
     * We must first wait() for everything to clear PCB
     * We must also read pipes for all the rows
     * If there are no more children running but we haven't read all the pipes,
     * exit, we have a problem.
     */
    size_t rowsRead = 0;
    while(rowsRead < SIZE) {
        struct processInfo info;
        /*
         * If a child is running, this will block and wait for a child to finish
         * When the child finishes PCB will be cleared
         * If no child is running, this will continue
         */
        wait(NULL);

        /*
         * I could also put in some logic here to make sure:
         * - Have a finishedR[SIZE][1] array to store which rows have finished
         * - As a row is stored, store a 1 in the finishedR array for that row
         * - if wait(NULL) returns -1, then there are no more children running and then
         *   we can check how many times finishedR[i] is empty, then determine if we have
         *   a child who exited without writing to the pipe and now we have to have
         *   the parent exit with an error. Ex: if finishedR[i][0] > 10, we have
         *   tried with this row 10 times and it has not finished, so we have a problem.
         *
         *   With this small of dataset it doesn't matter, but with a larger dataset
         *   it would ensure that we don't have a child exit without writing to the pipe
         *   or the parent exit without an erorr if needed.
         */

        /*
         * Either a single process has finished or all processes have finished
         * Either way, read the pipe until it is dry
         */
        while (read(p[0], &info, sizeof(info)) > 0) {
            // Store the result in R
            for (size_t i = 0; i < SIZE; i++) {
                R[info.rowNum][i] = info.row[i];
            }
            rowsRead++;
        }
    }

    // Close read end of pipe in parent
    close(p[0]);

    clock_gettime(CLOCK_MONOTONIC, &finish);
    // elapsed = (finish.tv_sec - start.tv_sec);
    // Print the result
    printArrayContents(SIZE, SIZE, R, "R"); // added for A3
    fflush(stdout);


    // Write to pipe
    fprintf(stdout, "writing to pipe: \n");
    for (size_t i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            write(pipeWrite, &R[i][j], sizeof(int));
            fprintf(stdout, "%d ", R[i][j]);
        }
        fprintf(stdout, "\n");
    }
    // close every pipe
    close(pipeWrite);
    close(p[0]);
    close(p[1]);
    fflush(stdin);
    fflush(stdout);


    // Print runtime
    // Removed for A3
    // printf("Runtime: %13.9f seconds\n", (double)elapsed + (double)(finish.tv_nsec - start.tv_nsec) / 1000000000.0);

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
 * Returns: struct processInfo with pid, rowNum, and row vector
*/
struct processInfo computeRowDotProduct(int matrixA[SIZE][SIZE], int matrixW[SIZE][SIZE], int rowNum) {
    // Setup return struct
    struct processInfo returnInfo;
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