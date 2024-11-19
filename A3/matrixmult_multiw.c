/*
 * Description: The module implements matrix multiplication using parallelization
 * Author names: Trevor Mathisen
 * Author emails: trevor.mathisen@sjsu.edu
 * Last modified date: 10/9/2023
 * Creation date: 9/26/2023
 */

/*
 * x1) Accept 1 matrix A and N matrix W in the commandline arguments
 * 2) Spawn N child processes using exec for matrixmult_parallel for each pair of A + Wi matrices
 * x3) You can adjust A2 code and binary if needed
 * x4) Assume fixed sized arrays of 8x8
 * x5) Child should print the input matricies and resulting Ri, with filenames, in pid.out (pid is the child pid)
 * x6) Override stdout/stderr in children to facilitate logging to pid.out and pid.err.
 * x6a) You can use dup2() to make this happen. Either the parent or child can open and write this to the log file
 *     for each child process.
 * x6b) You can open the files with flags O_RDWR | O_CREAT | O_APPEND. Remember to set file permissions to be
 *     open for everyone, such as 0777.
 * x7)  the parent process should write to the output file PID.out the string "Finished child PID pid of parent PPID"
 *     (replace PID, PPID)
 * x8) the message "Exited with exitcode = X" should also be written by the parent to the output file PID.out
 * x9) If exec fails have exit code of 1.
 * x10) Error handling for opening of files and arguments
 * x11) In case exec fails and the program cannot be started, write a descriptive error message with the command name
 *     and arguments to stderr (to the child command's PID.err error file); for this purpose you may use
 *     fprintf(stderr...) or perror("name of command")
 * x11a)  In other words, if the child processes encounter an invalid command (exec fails due to an invalid command)
 *       they should have an exit code of 1. Same if a file cannot be opened or the wrong number of arguments is given
 *       to matrixmult. This is the same requirement as the previous assignments, except you will use dup2 to replace
 *       stderr with PID.err.
 */

/* Example:
 *
    $ ./matrixmult_multiw A1.txt W1.txt W2.txt W3.txt

    $ cat 2353.out
    Starting command 1: child 2353 pid of parent 2234
     A1.txt=[...]
     W1.txt=[...]
     R=[
    110 190 390  50 10 0 0 0
    0 0 0....
    ]
    Finished child 2353 pid of parent 2234
    Exited with exitcode = 0

    $ cat 2353.err

    $ cat 2363.out
    Starting command 2: child 2363 pid of parent 2234
    A1.txt=[...]
    W2.txt=[...]
    R=[...]
    Finished child 2363 pid of parent 2234
    Exited with exitcode = 0

    $ cat 2363.err

    $ cat 2377.out
    Starting command 3: child 2377 pid of parent 2234
    A1.txt=[...]
    W3.txt=[...]
    R=[...]
    Finished child 2377 pid of parent 2234
    Exited with exitcode = 0

     $ cat 2377.err

     $ cat 2389.err
    Killed with signal 15
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SIZE 8

// Function prototypes
int matrixMultParallel(char *const *argv, size_t n);
void checkFile(FILE *file, const char *filename);
void readFile(FILE *file, int rows, int cols, int matrix[][cols]);
void printArrayContents(int rows, int cols, int matrix[][cols], char name[]);

int main(int argc, char* argv[]) {
    struct timespec start, finish;
    time_t elapsed;
    int savedStdOut = dup(1);
    int savedStdErr = dup(2);
    pid_t pidArray[argc - 2]; // Array of child pids for waitpid/writing to out files/status
    clock_gettime(CLOCK_MONOTONIC, &start); // Start the clock

    // Check if > 2 args are provided
    if (argc < 3) { // argv[0] is program name
        printf("Error - expecting at least 2 files as input\n");
        return 1;
    }

    for (size_t n = 1; n < argc - 1; n++) {
        // Spawn a child process
        pid_t pid = fork();
        pidArray[n - 1] = pid; // Store the pid
        // If parent, continue to next child
        if (pid < 0) {
            fprintf(stderr, "error: fork failed\n");
            perror("fork");
            exit(1);
        }
        if (pid != 0) continue;
        exit(matrixMultParallel(argv, n)); // Exit with the return code of the child function
    }

    // set stdout and stderr back to normal (just for fun)
    dup2(savedStdOut, 1);
    dup2(savedStdErr, 2);

    // wait for all children in pidArray and write Finished child xxxx pid of parent xxxx to child_pid.out
    for (size_t i = 0; i < argc - 2; i++) {
        int status;
        char filename[100];
        waitpid(pidArray[i], &status, 0);
        sprintf(filename, "%d.out", pidArray[i]);
        int outFile = open(filename, O_RDWR | O_APPEND, 0777);
        char parentLine[100];
        char exitLine[100];

        // Handle exit codes and signals, buffer a string to write to file
        sprintf(parentLine, "Finished child %d pid of parent %d\n", pidArray[i], getpid());
        if (WIFSIGNALED(status))
            sprintf(exitLine, "Killed with signal %d\n", WTERMSIG(status));
        else
            sprintf(exitLine, "Exited with exitcode = %d\n", WEXITSTATUS(status));


        //append to outfile
        write(outFile, parentLine, strlen(parentLine));
        write(outFile, exitLine, strlen(exitLine));
        close(outFile);
    }

    clock_gettime(CLOCK_MONOTONIC, &finish);
    elapsed = (finish.tv_sec - start.tv_sec);

    // Print runtime to 15 digits
    fprintf(stdout, "Parent runtime: %13.9f seconds\n", (double)elapsed + (double)(finish.tv_nsec - start.tv_nsec) / 1000000000.0);

    return 0;
}

/*
 * This function suitcases all child code in a single function. Extracted/refactored by PyCharm
 * Assumption: Will only be called by a newly forked child process
 * Input parameters: A pointer to the argv array, and the index of the current child to call matrixmult_parallel
 * Returns: int (1) if execvp fails, otherwise the exit code of the child process
*/
int matrixMultParallel(char *const *argv, size_t n) {
    // Child only code below here
    // for each child, redirect stdout and stderr to a file
    char out[100];
    char err[100];
    sprintf(out, "%d.out", getpid());
    sprintf(err, "%d.err", getpid());

    // Open the file for writing
    int newStdOut = open(out, O_RDWR | O_CREAT | O_APPEND, 0666);
    int newStdErr = open(err, O_RDWR | O_CREAT | O_APPEND, 0666);

    // Redirect stdout and stderr to the file
    dup2(newStdOut, 1);
    dup2(newStdErr, 2);

    // Close the file descriptors
    close(newStdOut);
    close(newStdErr);

    fprintf(stdout, "Starting command %d: child %d pid of parent %d\n", (int) n, getpid(), getppid());
    fflush(stdout);

    // create args and call with execvp
    char *args[] = {"./matrixmult_parallel", argv[1], argv[n + 1], NULL};
    execvp(args[0], args); // Execute the program
    // child process ends under normal conditions
    fprintf(stderr, "error: exec failed\n");
    perror("execvp");
    return 1; // Will exit child with exit code 1
}

/*
 * This function checks the file and prints errors if needed
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