#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <mpi.h>

// Function to allocate memory for a square matrix
int** allocateMatrix(int N) {
    int i;
    int** matrix = (int**)malloc(N * sizeof(int*));
    for (i = 0; i < N; i++) {
        matrix[i] = (int*)malloc(N * sizeof(int));
    }
    return matrix;
}

// Function to fill a matrix with random values
void fillMatrix(int** matrix, int N) {
    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            matrix[i][j] = rand() % 10; // Filling with random values from 0 to 9
        }
    }
}

// Function to deallocate memory for a matrix
void deallocateMatrix(int** matrix, int N) {
    int i;
    for (i = 0; i < N; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

// Function to print matrix to a file
void saveMatrixToFile(int** matrix, int rows, int cols, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        int i, j;
        for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {
                fprintf(file, "%d ", matrix[i][j]);
            }
            fprintf(file, "\n");
        }
        fclose(file);
    } else {
        fprintf(stderr, "Error: Could not open file for writing.\n");
    }
}

// Function to read matrix from a file
int** readMatrixFromFile(int rows, int cols, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file != NULL) {
        int** matrix = allocateMatrix(rows);
        int i, j;
        for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {
                fscanf(file, "%d", &matrix[i][j]);
            }
        }
        fclose(file);
        return matrix;
    } else {
        fprintf(stderr, "Error: Could not open file for reading.\n");
        return NULL;
    }
}

// Function to print matrix for debugging
void printMatrix(int** matrix, int rows, int cols) {
    int i, j;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            printf("%4d |", matrix[i][j]); // Adjust the width (4) as needed
        }
        printf("\n");
        for (j = 0; j < cols-1; j++) {
            printf("------+"); // Print horizontal separators
        }
        printf("\n");
    }
}

// Function to perform matrix multiplication for a given range of rows
void multiplyMatrices(int** A, int** B, int** result, int startRow, int endRow, int N) {
    int i, j, k;
    for (i = startRow; i < endRow; i++) {
        for (j = 0; j < N; j++) {
            result[i][j] = 0;
            for (k = 0; k < N; k++) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s N Verbose\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int verbose = atoi(argv[2]);

    // Initialize MPI
    MPI_Init(&argc, &argv);

    // Get the number of processes and the rank of the current process
    int numProcesses, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Calculate the number of rows each process will handle
    int rowsPerProcess = N / numProcesses;
    int startRow = rank * rowsPerProcess;
    int endRow = (rank == numProcesses - 1) ? N : startRow + rowsPerProcess;

    // Allocate memory for matrices A, B, and the result
    int** A = allocateMatrix(N);
    int** B = allocateMatrix(N);
    int** result = allocateMatrix(N);

    // File paths
    const char* path = "/home/cluser/wd";
    const char* fileA = "matrixA.txt";
    const char* fileB = "matrixB.txt";
    const char* fileResult = "resultMatrix.txt";

    // File paths for matrices A, B, and the result
    char pathA[100], pathB[100], pathResult[100];
    sprintf(pathA, "%s/%s", path, fileA);
    sprintf(pathB, "%s/%s", path, fileB);
    sprintf(pathResult, "%s/%s", path, fileResult);

    if (rank == 0) {
        saveMatrixToFile(A, N, N, pathA);
        saveMatrixToFile(B, N, N, pathB);
    }else{
        // Read matrices A and B directly from the shared directory
        A = readMatrixFromFile(N, N, pathA);
        B = readMatrixFromFile(N, N, pathB);
    }

    //I write down the machine time
    clock_t start_time = clock();

    // Perform matrix multiplication for the assigned rows
    multiplyMatrices(A, B, result, startRow, endRow, N);
    // Gather the partial results from all processes to the master process
    MPI_Gather(&(result[startRow][0]), (endRow - startRow) * N, MPI_INT, &(result[0][0]), (endRow - startRow) * N, MPI_INT, 0, MPI_COMM_WORLD);

    //I write down the machine time
    clock_t end_time = clock();
    //I substract the 2 times to find out the computing duration
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    //printing logic
    if (rank == 0 && verbose==1 && N<20) {
        printf("\n\nMatrix A:\n");
        printMatrix(A,N,N);
        
        printf("\n\nMatrix B:\n");
        printMatrix(B,N,N);
        
        printf("\n\nResult Matrix:\n");
        printMatrix(result,N,N);
    }

    //Result
    printf("%.6f\n", elapsed_time);

    // Deallocate memory
    deallocateMatrix(A, N);
    deallocateMatrix(B, N);
    deallocateMatrix(result, N);

    // Finalize MPI
    MPI_Finalize();

    return 0;
}
