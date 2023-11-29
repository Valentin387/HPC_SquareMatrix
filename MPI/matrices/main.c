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
                printf("i=%d, j=%d, k=%d\n", i, j, k);
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

    srand(time(NULL)); // Use current time as seed for random generator

    // Initialize MPI
    MPI_Init(&argc, &argv);

    // Get the number of processes and the rank of the current process
    int numProcesses, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //MPI_Barrier(MPI_COMM_WORLD);
    //printf("Rank %d: Number of processes: %d\n", rank, numProcesses);

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

    // File paths for matrices A, B, and the result
    char pathA[100], pathB[100], pathResult[100];
    sprintf(pathA, "%s/%s", path, fileA);
    sprintf(pathB, "%s/%s", path, fileB);

    if (rank == 0) {
        // Head node generates matrices A and B
        printf("Rank %d: Generating matrices A and B\n", rank);
        fillMatrix(A, N);
        fillMatrix(B, N);
        printf("Rank %d: Saving matrix A to: %s\n", rank, pathA);
        saveMatrixToFile(A, N, N, pathA);
        printf("Rank %d: Saving matrix B to: %s\n", rank, pathB);
        saveMatrixToFile(B, N, N, pathB);
    }
    

    // Use a barrier to synchronize processes
    MPI_Barrier(MPI_COMM_WORLD);
    printf("Rank %d: After Barrier\n", rank);
    /*
    // Use MPI I/O to read matrices A and B from files
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_File fileA_handle, fileB_handle;
    MPI_File_open(MPI_COMM_WORLD, pathA, MPI_MODE_RDONLY, MPI_INFO_NULL, &fileA_handle);
    MPI_File_open(MPI_COMM_WORLD, pathB, MPI_MODE_RDONLY, MPI_INFO_NULL, &fileB_handle);

    MPI_File_set_view(fileA_handle, 0, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
    MPI_File_set_view(fileB_handle, 0, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);

    MPI_File_read_all(fileA_handle, &(A[0][0]), N * N, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_read_all(fileB_handle, &(B[0][0]), N * N, MPI_INT, MPI_STATUS_IGNORE);

    MPI_File_close(&fileA_handle);
    MPI_File_close(&fileB_handle);

    */

    // All nodes read matrices A and B directly from the shared directory
    printf("Rank %d: Reading matrix A from: %s\n", rank, pathA);
    A = readMatrixFromFile(N, N, pathA);
    printf("Rank %d: Reading matrix B from: %s\n", rank, pathB);
    B = readMatrixFromFile(N, N, pathB);
    

    // Allocate memory for the local result
    int localRows = endRow - startRow;
    int** localResult = allocateMatrix(localRows);
    printf("Rank %d: after allocating localResult \n", rank);

    //I write down the machine time
    clock_t start_time = clock();
    
    // Perform matrix multiplication for the assigned rows
    multiplyMatrices(A, B, localResult, startRow, endRow, N);
    printf("Rank %d: after multiplication \n", rank);

    printf("Rank %d: localResult:\n", rank);
    printMatrix(localResult, localRows, N);

    // Use a barrier to synchronize processes
    MPI_Barrier(MPI_COMM_WORLD);

    // Gather the partial results from all processes to the master process
    MPI_Gather(&(localResult[0][0]), localRows * N, MPI_INT,
        &(result[startRow][0]), localRows * N, MPI_INT, 0, MPI_COMM_WORLD);
    printf("Rank %d: after gathering \n", rank);

    //I write down the machine time
    clock_t end_time = clock();
    //I substract the 2 times to find out the computing duration
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    //printing logic
    if (rank == 0 && verbose == 1 && N < 20) {
        printf("\n\nMatrix A:\n");
        printMatrix(A, N, N);

        printf("\n\nMatrix B:\n");
        printMatrix(B, N, N);

        printf("\n\nResult Matrix:\n");
        printMatrix(result, N, N);
    }

    if (rank == 0) {
        // Print the elapsed time
        printf("%.6f\n", elapsed_time);
        printf("Rank %d: after time print \n", rank);
    }

    // Deallocate memory for local result
    deallocateMatrix(localResult, localRows);
    printf("Rank %d: after deallocation of localResult \n", rank);
    // Deallocate memory
    deallocateMatrix(A, N);
    deallocateMatrix(B, N);
    deallocateMatrix(result, N);

    MPI_Finalize();
    printf("Rank %d: after MIP finalize \n", rank);

    return 0;
}
