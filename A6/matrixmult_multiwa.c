/*
 * Description: The module implements matrix multiplication using parallelization
 * Author names: Trevor Mathisen
 * Author emails: trevor.mathisen@sjsu.edu
 * Last modified date: 11/15/2023
 * Creation date: 11/13/2023
 */

/*
 *  1) Expand your Assgt3 to develop a matrixmult_multiwa.c program that accepts continuously new A matrices from stdin.
 *  x2) Given the command line parameters you will do the same as Assgt3, which is to multiply A with all W arrays in
 *     children processes.
 *  x3) Same as you did in Assgt3, you will use fork for the children and exec matrixmult_parallel (Assgt2, but modified).
 *  x4) At each line of stdin, one new matrix Anew filename is expected, which the parent process will send to all
 *     children that it forked/spawned (from the command line argv, as in Assgt3) through a pipe
 *  x5) Thus the child processes keep waiting for new As to come (this is unlike previous assignments and you will
 *     need to modify Assgt2 to read new As from a read end of a pipe). Each child remembers its Wi, which the ith
 *     child inherited when the child was forked/spawned, and multiplies the new A matrices that keep coming in
 *     sequentially with Wi . The ith child computes A*Wi.
 *  x6) When the next line with a new Anew filename is received, the child multiplies the new Anew with Wi and
 *     appends the result to the bottom of Ri as new rows
 *  x7) Each child prints (writes) all of the input filenames A and Wi, as well as the resulting matrix Ri to a
 *     file PID.out (you will replace PID with the child's getpid()). You do not need to print the A and Wi values,
 *     but just the filenames.
 *  x8) The parent can either send the A filenames, or the 8x8 A matrix data, such that each child doesn't have to
 *     reread A from disk, this is your design decision (in the first case assume filenames of 100 chars max, while
 *     in the second case assume the same matrix sizes of 8*8=64 ints always)
 *  x9) When the user presses Ctrl-D (^D or EOF-end-of-file) the input of A matrices stops. When user presses ^D the
 *     parent terminates stdin input and all child processes will write their R arrays to their own PID.out
 *     (replace PID with getpid()) and exit.
 */

/* Example:
    Test with the same input as Assgt 1 test and give A1, A2, A3 files in sequence many times to ensure it works.

    ./matrixmult_multiwa A1.txt W1.txt W2.txt W3.txt

    A2.txt

    A3.txt

    A1.txt

    A2.txt

    A3.txt

    ....
    ^D <--Ctrl-D is the EOF value, which terminates your input
    If you had your commands in a text file cmds.txt, you could also run the above with redirection or with a pipe:
    $ ./matrixmult_multiwa A1.txt W1.txt W2.txt W3.txt < cmds.txt
    $ cat cmds.txt | ./matrixmult_multiwa A1.txt W1.txt W2.txt W3.txt
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
#define READ_END 0
#define WRITE_END 1
#define MATRIX_SIZE sizeof(int) * SIZE * SIZE

/*
 * This structure is used to store the pid information
 * Assumption: Will store pid and pipe info
 * Input parameters: a child's pid and the associated pipe
 * Returns: Nothing
*/
struct pidInfo {
    pid_t pid;
    int pipe[2];
} typedef pidInfo;

// Function prototypes
int matrixMultParallel(char *const *wFiles, size_t n, pidInfo child);
void checkFile(FILE *file, const char *filename);
void readFile(FILE *file, int rows, int cols, int matrix[][cols]);
void printArrayContents(int rows, int cols, int matrix[][cols], char name[]);

int main(int argc, char* argv[]) {
    struct timespec start, finish;
    time_t elapsed;
    char *line = NULL;  // For getline
    size_t len = 0;  // For getlin
    int numChildren = argc - 2;
    char **wFiles = malloc(sizeof(char *) * (numChildren + 1)); // A.txt stored, so +1
    int A[SIZE][SIZE] = {0};

    // Open up A.txt which will be passed via pipes
    FILE *fileA = fopen(argv[1], "r");
    checkFile(fileA, argv[1]);
    readFile(fileA, SIZE, SIZE, A);
    fclose(fileA);

    pidInfo pidArray[numChildren]; // Array of child pids for waitpid/writing to out files/status and parent > child pipe
    clock_gettime(CLOCK_MONOTONIC, &start); // Start the clock

    // Check if > 2 args are provided
    if (argc < 3) { // argv[0] is program name
        printf("Error - expecting at least 2 files as input\n");
        return 1;
    }
    // Copy argv[1] to wFiles[0].
    wFiles[0] = malloc(sizeof(char) * strlen(argv[1]));
    strcpy(wFiles[0], argv[1]);

    // Copy W matrix filenames to wFiles array starting with argv[1] (argv[0] is program name)
    for (size_t i = 1; i < numChildren + 1; i++) { // +1 accounts for A.txt
        wFiles[i] = malloc(sizeof(char) * strlen(argv[i + 1]));
        strcpy(wFiles[i], argv[i + 1]);
    }

    // This loop spawns all the children and passes the initial A.txt to them
    for (size_t n = 0; n < numChildren; n++) {
        // Spawn a child process
        pidInfo child;
        pipe(child.pipe);
        pid_t pid = fork();
        child.pid = pid;
        pidArray[n] = child; // Store the pid
        // If parent, continue to next child
        if (pid < 0) {
            fprintf(stderr, "error: fork failed\n");
            perror("fork");
            exit(1);
        }
        // If parent, write to pipe then continue to next child
        if (pid != 0) {
            // Write rSum to the pipe
            write(pidArray[n].pipe[WRITE_END], A, MATRIX_SIZE);
            continue;
        }
        // Child only code below here
        exit(matrixMultParallel(argv, n + 1, pidArray[n])); // Exit with the return code of the child function
    }

    // This loop is within the parent and reads from stdin, writing to the pipes of all children
    while (getline(&line, &len, stdin) > 0) {
        char *token;  // For getline
        if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = '\0';
        token = strtok(line, " "); // Strip whitespace, get the first token as a C-string
        if(token) {
            // Zero out A
            memset(A, 0, MATRIX_SIZE);

            // Open the file
            FILE *newFileA = fopen(line, "r");
            checkFile(newFileA, line);
            readFile(newFileA, SIZE, SIZE, A);
            fclose(newFileA);

            // Write to every pipe in pidArray
            for (size_t i = 0; i < numChildren; i++) {
                // Append filename to the end of every child's .out file
                char filename[100];
                sprintf(filename, "%d.out", pidArray[i].pid);
                int outFile = open(filename, O_RDWR | O_APPEND, 0777);
                write(outFile, line, strlen(line));
                close(outFile);

                // Write to the pipe
                write(pidArray[i].pipe[WRITE_END], A, MATRIX_SIZE);
            }
        }
    }

    // Close the write end of all the pipes
    for (size_t i = 0; i < numChildren; i++) {
        close(pidArray[i].pipe[WRITE_END]);
    }

    // wait for all children in pidArray and write Finished child xxxx pid of parent xxxx to child_pid.out
    int currentChild;
    int status;
    while ((currentChild = wait(&status)) > 0) {
    //for (size_t i = 0; i < numChildren; i++) {
        // Get the right pid
        int i = 0;
        for (i = 0; i < numChildren; i++) {
            if (pidArray[i].pid == currentChild)
                break;
        }

        char filename[100];
        sprintf(filename, "%d.out", pidArray[i].pid);
        int outFile = open(filename, O_RDWR | O_APPEND, 0777);
        char parentLine[100];
        char exitLine[100];

        // Handle exit codes and signals, buffer a string to write to file
        sprintf(parentLine, "Finished child %d pid of parent %d\n", pidArray[i].pid, getpid());
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

    // Free memory
    for (size_t i = 0; i < numChildren + 1; i++) {
        free(wFiles[i]);
    }
    free(wFiles);
    free(line);

    return 0;
}

/*
 * This function suitcases all child code in a single function. Extracted/refactored by PyCharm
 * Assumption: Will only be called by a newly forked child process
 * Input parameters: A pointer to the argv array, and the index of the current child to call matrixmult_parallel
 * Returns: int (1) if execvp fails, otherwise the exit code of the child process
*/
int matrixMultParallel(char *const *wFiles, size_t n, pidInfo child) {
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
    dup2(newStdOut, STDOUT_FILENO);
    dup2(newStdErr, STDERR_FILENO);
    dup2(child.pipe[READ_END], STDIN_FILENO);
    close(child.pipe[WRITE_END]);

    // Close the file descriptors
    close(newStdOut);
    close(newStdErr);

    fprintf(stdout, "Starting command %d: child %d pid of parent %d\n", (int) n, getpid(), getppid());
    fprintf(stdout, "%s", wFiles[1]);
    fflush(stdout);

    // create args and call with execvp
    char *args[] = {"./matrixmult_threaded", wFiles[0], wFiles[n + 1], NULL};
    execvp(args[0], args); // Execute the program

    // child process ends under normal conditions
    fprintf(stderr, "error: exec failed\n");
    fprintf(stderr, "\n");
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