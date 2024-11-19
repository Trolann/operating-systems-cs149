/*
 * Description: The module implements matrix multiplication using parallelization
 * Author names: Trevor Mathisen
 * Author emails: trevor.mathisen@sjsu.edu
 * Last modified date: 10/30/2023
 * Creation date: 10/18/2023
 */

/*
 * 1) Get A.txt from command line
 * 2) Input W matrices can come from commandline or stdin in a single line
 * 3) Create an rSum matrix in the deep parent to store which is the position sum of each child R matrix
 * 4) For each line of input (once for commandline at start, once per stdin):
 *    - Create a child process to run matrixmult_parallel A.txt = rSum, W.txt = Wi (from input)
 *    - A2 is modified to send the R matrix via a pipe to the parent
 *    - A2 should still write pid.out and pid.err files for each child
 *    - All children R matricies are position summed into the new rSum
 * 5) When the user presses Ctrl-D (^D or EOF-end-of-file) the input of W matrices from stdin stops
 * 6) parent process will print its current Rsum matrix to stdout and exit
 * 7) check you have A and at least one W in your argvs. Else exit(1)
 */

/*
 * Computing Rsum in the parent is something new in this assignment.
 * To send result Ri from the ith child to the parent after exec, in the child before exec you could use dup2 to
 * change stdout to the pipe write end it inherited (or, similarly you could use dup2 in the parent to change stdout
 * to the pipe write end and a child will inherit it). Remember that open files get preserved when you call fork or
 * exec. Thus, even after the child calls exec, when it prints to stdout it will be sending things to the parent
 * (you could also replace stdin with a pipe read end to read things from the pipe as stdin, but you dont need that
 * since the child inherits what it needs). Thus, you will need to modify Assgt2 code to write the Ri to the parent.
 *
 * Ensure there are no memory leaks (e.g. from the array of strings you made to store the W filenames).
 * Use valgrind to detect memory leaks. Your code should work on inputs with up to a dozen lines.
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
int matrixMultParallel(char *const *wFiles, size_t n, int childRMatrixPipe[2], int parentRMatrixPipe[2]);
void printArrayContents(int rows, int cols, int matrix[][cols], char name[]);
void childManager(int numChildren, char *const *wFiles, int (*rSum)[SIZE]);

int main(int argc, char* argv[]) {
    // Keep track of runtime
    struct timespec start, finish;
    time_t elapsed_sec = 0.0;
    long elapsed_nsec = 0.0;
    // Dynamic elements to process files/input
    char **wFiles = malloc(sizeof(char *) * (argc - 1));
    char *line = NULL;  // For getline
    size_t len = 0;  // For getline
    char *token;  // For getline
    size_t numWFiles = 0;
    int savedStdIn = dup(0);
    // Dynamically create rSum matrix to pass address to to childManager using mmap
    int (*rSum)[SIZE] = malloc(sizeof(int[SIZE][SIZE]));

    // zero out rsum
    for(size_t i = 0; i < SIZE; i++)
        for(size_t j = 0; j < SIZE; j++)
            rSum[i][j] = 0;
    rSum[0][0] = -3; // Flag for exec'd child to know to use rSum or A.txt

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
    for (size_t i = 1; i < argc - 1; i++) {
        wFiles[i] = malloc(sizeof(char) * strlen(argv[i + 1]));
        strcpy(wFiles[i], argv[i + 1]);
    }

    // Pass w Matrix filenames to childManager
    childManager(argc - 2, wFiles, rSum);


    clock_gettime(CLOCK_MONOTONIC, &finish); // Stop the clock before we get user input
    elapsed_sec += (double) (finish.tv_sec - start.tv_sec);
    elapsed_nsec += (double) (finish.tv_nsec - start.tv_nsec);

    dup2(savedStdIn, 0);
    fflush(stdin);

    // Get a line from stdin, tokenize it, then realloc space for wFiles and add the token to wFiles
    while (getline(&line, &len, stdin) > 0) {
        clock_gettime(CLOCK_MONOTONIC, &start); // restart the clock
        // Replace trailing \n with \0
        if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = '\0';
        // Clear wFiles, A.txt remains until the end
        for (size_t i = 1; i < numWFiles; i++) {
            free(wFiles[i]);
            wFiles[i] = NULL;
        }
        // Reset numWFiles for A.txt only
        numWFiles = 1;
        token = strtok(line, " ");
        while ((token) != NULL) {
            // Realloc wFiles to hold argv[1] + numWFiles + new token
            if ((wFiles = realloc(wFiles, sizeof(char *) * numWFiles + 1)) == NULL) {
                // If realloc fails, print error and exit
                fprintf(stderr, "error: realloc failed\n");
                exit(1);
            }
            wFiles[numWFiles] = malloc(sizeof(char) * strlen(token)); // Keep wFiles[0] as A.txt
            strcpy(wFiles[numWFiles], token);

            // Get ready for the next token
            token = strtok(NULL, " ");
            numWFiles++;
        }
        // Call childManager for this line of stdinput
        childManager(numWFiles - 1, wFiles, rSum);

        // Update the clock for this runtime
        clock_gettime(CLOCK_MONOTONIC, &finish);
        elapsed_sec += (double) (finish.tv_sec - start.tv_sec);
        elapsed_nsec += (double) (finish.tv_nsec - start.tv_nsec);
        dup2(savedStdIn, 0);
        fflush(stdin);
    }

    // Print final RSUM
    printArrayContents(SIZE, SIZE, rSum, "Final rSum Matrix");

    // Print runtime to 15 digits
    fprintf(stdout, "Parent runtime: %13.9f seconds\n", elapsed_sec + elapsed_nsec / 1000000000.0);

    // Be safe, free memory
    free(rSum);
    free(line);
    free(token);
    for (size_t i = 0; i < numWFiles; i++) {
        free(wFiles[i]);
        wFiles[i] = NULL;
    }
    line = NULL;
    token = NULL;
    return 0;
}

/*
 * This function manages everything to get ready for the children, from pipes, thru forks and collecting the results
 * Assumption: wFiles[0] will be A.txt, rSum[0][0] is -3 if using A.txt, otherwise as seen
 * Input parameters: Number of children to launch, an array of W files for children to use and a pointer to rSum
 * Returns: None, void. Updates rSum with the results of the children
*/
void childManager(int numChildren, char *const *wFiles, int (*rSum)[SIZE]) {
    // Setup a pipe for children to write to
    int childRToParent[2];
    int parentRToChild[2];
    pipe(childRToParent);
    pipe(parentRToChild);

    pid_t pidArray[numChildren]; // Array of child pids for waitpid/writing to out files/status

    for (size_t n = 0; n < numChildren; n++) {
        fflush(stdin);
        // Spawn a child process
        pid_t pid = fork();
        pidArray[n] = pid; // Store the pid
        if (pid < 0) {
            fprintf(stderr, "error: fork failed\n");
            perror("fork");
            exit(1);
        }
        dup2(parentRToChild[0], 0);
        //fflush(0);

        // If parent, write to pipe then continue to next child
        if (pid != 0) {
            // Write rSum to the pipe
            write(parentRToChild[1], rSum, sizeof(int) * SIZE * SIZE);
            //close(parentRToChild[0]);
            usleep(3000);  // Non-performant syncing mechanism.
            continue;
        }

        // Close the read end of the pipe
        close(childRToParent[0]);
        close(parentRToChild[1]);
        exit(matrixMultParallel(wFiles, n, childRToParent, parentRToChild)); // Exit with the return code of the child function
    }

    //close(parentRToChild[1]);

    int currentChild;
    int status;
    // wait for all children in pidArray and write Finished child xxxx pid of parent xxxx to child_pid.out
    //for (size_t i = 0; i < numChildren; i++) {
    while ((currentChild = wait(&status)) > 0) {
        // pipe has 64 ints and need to be read into and summed into rSum where the first 8 ints are the first row, etc
        int rMatrix[SIZE][SIZE];
        char parentLine[100];  // Exit line from the parent
        char exitLine[100]; // Exit code from the child
        char filename[100];

        // Get the right pid
        int i = 0;
        for (i = 0; i < numChildren; i++) {
            if (pidArray[i] == currentChild)
                break;
        }

        sprintf(filename, "%d.out", pidArray[i]);
        int outFile = open(filename, O_RDWR | O_APPEND, 0777);

        // For each child, read the pipe into rMatrix
        for (size_t j = 0; j < SIZE; j++) {
            for (size_t k = 0; k < SIZE; k++) {
                read(childRToParent[0], &rMatrix[j][k], sizeof(int));
                if (i) // If not the first child, add to rSum
                    rSum[j][k] += rMatrix[j][k];
                else // If first child, set rSum to rMatrix
                    rSum[j][k] = rMatrix[j][k]; // This resets the -3 flag set earlier
            }
        }

        // Handle exit codes and signals, buffer a string to write to file
        sprintf(parentLine, "Finished child %d pid of parent %d\n", pidArray[i], getpid());
        if (WIFSIGNALED(status)) {
            sprintf(exitLine, "Killed with signal %d\n", WTERMSIG(status));
        }
        else {
            sprintf(exitLine, "Exited with exitcode = %d\n", WEXITSTATUS(status));
        }

        //append to outfile
        write(outFile, parentLine, strlen(parentLine));
        write(outFile, exitLine, strlen(exitLine));
        close(outFile);
    }

}

/*
 * This function suitcases all child code in a single function. Extracted/refactored by PyCharm
 * Assumption: Will only be called by a newly forked child process
 * Input parameters: A pointer to the argv array, and the index of the current child to call matrixmult_parallel
 * Returns: int (1) if execvp fails, otherwise the exit code of the child process
*/
int matrixMultParallel(char *const *wFiles, size_t n, int childRMatrixPipe[2], int parentRMatrixPipe[2]) {
    // Child only code below here
    // for each child, redirect stdout and stderr to a file
    char out[100];
    char err[100];
    char pipeVar[100];

    sprintf(out, "%d.out", getpid());
    sprintf(err, "%d.err", getpid());
    sprintf(pipeVar, "PIPE=%d", childRMatrixPipe[1]); // This allows the child to setup a pipe after execxx()

    // Open the file for writing
    int newStdOut = open(out, O_RDWR | O_CREAT | O_APPEND, 0666);
    int newStdErr = open(err, O_RDWR | O_CREAT | O_APPEND, 0666);

    // Redirect stdout and stderr to the file
    dup2(newStdOut, 1);
    dup2(newStdErr, 2);
    //dup2(parentRMatrixPipe[0], 0);

    // Close the file descriptors
    close(newStdOut);
    close(newStdErr);
    //close(parentRMatrixPipe[0]);

    fprintf(stdout, "Starting command %d: child %d pid of parent %d\n", (int) n, getpid(), getppid());
    fflush(stdout);

    // create args and call with execvp
    putenv(pipeVar); // Set environment variable for the child to use
    char *args[] = {"./matrixmult_parallel", wFiles[0], wFiles[n + 1], NULL};
    execvp(args[0], args); // Execute the program

    // child process ends under normal conditions
    fprintf(stderr, "error: exec failed\n");
    fprintf(stderr, "\n");
    perror("execvp");
    return 1; // Will exit child with exit code 1
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