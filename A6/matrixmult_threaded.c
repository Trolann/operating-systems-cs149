#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define SIZE 8
#define READ_END 0
#define WRITE_END 1
#define MATRIX_SIZE sizeof(int) * SIZE * SIZE

/*
 * This structure is used to pass thread data
 * Assumption: Will be used by a thread and stored in a 2D array
 * Input parameters: as below
 * Returns: Nothing
*/
// Mutex for critical sections
pthread_mutex_t mutex;
struct threadData {
    int row;
    int col;
    int (*A)[SIZE];
    int (*W)[SIZE];
    int (**R);
    int iterationNum;
} typedef threadData;

// Function prototypes
void checkFile(FILE *file, const char *filename);
void readFile(FILE *file, int rows, int cols, int matrix[][cols]);
void printArrayContents(int rows, int cols, int matrix[][cols], char name[]);
void* computeCell(void* givenData);

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

    // Initialize mutex
    pthread_mutex_init(&mutex, NULL);
    pthread_t threads[SIZE][SIZE];
    threadData data[SIZE][SIZE];

    while (read(STDIN_FILENO, &A, MATRIX_SIZE) > 0) {
        // Realloc R to be (SIZE * SIZE) * iterationNum
        int oldSize = SIZE * iterationNum;
        pthread_mutex_lock(&mutex);
        R = realloc(R, sizeof(int *) * SIZE * (++iterationNum));
        // Allocate memory for the new rows
        for (int i = oldSize; i < SIZE * iterationNum; i++) {
            R[i] = malloc(sizeof(int) * SIZE);
        }
        pthread_mutex_unlock(&mutex);

        char filename[100];
        sprintf(filename, " x %s\n", argv[2]);
        fprintf(stdout, "%s", filename);
        fflush(stdout);

        // Create threads for each cell
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                data[i][j].row = i;
                data[i][j].col = j;
                data[i][j].A = A;
                data[i][j].W = W;
                data[i][j].R = R;
                data[i][j].iterationNum = iterationNum;
                pthread_create(&threads[i][j], NULL, computeCell, &data[i][j]);
            }
        }

        // Join threads
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                pthread_join(threads[i][j], NULL);
            }
        }

        // Zero out A
        memset(A, 0, MATRIX_SIZE);

        fflush(stdin);
        fflush(stdout);

    }
    pthread_mutex_lock(&mutex);
    fprintf(stdout, "\nrMatrix for %d A matrices=[\n", iterationNum);
    fflush(stdout);
    for (int i = 0; i < SIZE * iterationNum; i++) {
        for (int j = 0; j < SIZE; j++)
            fprintf(stdout, "%d ", R[i][j]);
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "]\n");
    fflush(stdout);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);

    // Free memory
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
 * This function computes a single cell.
 * Assumption: To be ran as a thread.
 * Input parameters: FILE *file, const char *filename
 * Returns: void, exits if needed
*/
void* computeCell(void* givenData) {
    // pthread_create throws a fit if not a void then cast correctly within the function
    threadData *data = (threadData*) givenData;
    int r = data->row;
    int c = data->col;
    int offset = (SIZE * (data->iterationNum - 1));

    // Compute the cell value
    int sum = 0;
    for (int k = 0; k < SIZE; k++) {
        sum += data->A[r][k] * data->W[k][c];
    }

    // Lock before modifying R
    pthread_mutex_lock(&mutex);
    data->R[r + offset][c] = sum;
    pthread_mutex_unlock(&mutex);

    return NULL; // Nullptr
}