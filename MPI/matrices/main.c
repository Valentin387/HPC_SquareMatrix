#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

void mostrarMatriz(int *matrix, int length) {
    int j;
    for (int i = 0; i < length; i++) {
        for (j = 0; j < length; j++) {
            printf("%4d |", matrix[i * length + j]);
        }
        printf("\n");
        for (j = 0; j < length - 1; j++) {
            printf("------+");
        }
        printf("\n");
    }
}

void fill_matrix(int *matrix, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i * N + j] = rand() % 10;
        }
    }
}

void save_matrix_to_file(int *matrix, int length, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (file != NULL) {
        fwrite(matrix, sizeof(int), length * length, file);
        fclose(file);
    } else {
        fprintf(stderr, "Error opening file for writing: %s\n", filename);
    }
}

void read_matrix_from_file(int *matrix, int length, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file != NULL) {
        fread(matrix, sizeof(int), length * length, file);
        fclose(file);
    } else {
        fprintf(stderr, "Error opening file for reading: %s\n", filename);
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    struct timespec start, end;
    double elapsed_time;

    if (argc < 3) {
        if (rank == 0) {
            fprintf(stderr, "Usage: %s <matrix_size> <verbose>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    int length = atoi(argv[1]);
    int verbose = atoi(argv[2]);
    int max_number = 100;

    if (length % size != 0) {
        if (rank == 0) {
            fprintf(stderr, "Error: matrix size must be divisible by the number of processes, (idk why)\n");   
        }
        MPI_Finalize();
        return 1;
    }

    if ( length < size){
        if (rank == 0) {
            fprintf(stderr, "Error: matrix size must be equal or greater than the number of processes\n");
        }
        MPI_Finalize();
        return 1;
    }

    srand(time(NULL));

    int *matrixA = (int *)malloc(length * length * sizeof(int));
    int *matrixB = (int *)malloc(length * length * sizeof(int));
    int *result = (int *)malloc(length * length * sizeof(int));
    int *gathered_result = NULL;

    if (rank == 0) {
        // Process 0 fills matrices A and B and saves them to a file
        fill_matrix(matrixA, length);
        fill_matrix(matrixB, length);

        // Save matrices to file
        save_matrix_to_file(matrixA, length, "/home/cluser/wd/matrixA.bin");
        save_matrix_to_file(matrixB, length, "/home/cluser/wd/matrixB.bin");
    }

    MPI_Barrier(MPI_COMM_WORLD);
    //clock_gettime(CLOCK_MONOTONIC, &start);

    // All processes read matrices A and B from files
    read_matrix_from_file(matrixA, length, "/home/cluser/wd/matrixA.bin");
    read_matrix_from_file(matrixB, length, "/home/cluser/wd/matrixB.bin");

    int rows_per_process = length / size;
    int extra_rows = length % size;
    int start_row = rank * rows_per_process;
    int end_row = start_row + rows_per_process + (rank == size - 1 ? extra_rows : 0);
    //printf("Rank %d: start_row = %d, end_row = %d\n", rank, start_row, end_row);

    if (rank == 0){
        clock_gettime(CLOCK_MONOTONIC, &start);
    }

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < length; j++) {
            result[i * length + j] = 0;
            for (int k = 0; k < length; k++) {
                result[i * length + j] += matrixA[i * length + k] * matrixB[k * length + j];
            }
        }
    }

    //mostrarMatriz(result, length);
    MPI_Barrier(MPI_COMM_WORLD);

    // Gather the results to the root process
    if (rank == 0) {
        gathered_result = (int *)malloc(length * length * sizeof(int));
    }

    // Calculate the number of rows for the current process
    int local_rows = (rank == size - 1) ? rows_per_process + extra_rows : rows_per_process;
    //printf("Rank %d: local_rows = %d\n", rank, local_rows);
    //printf("Rank %d: local_rows * length = %d\n", rank, local_rows * length);

    MPI_Gather(
        result + start_row * length,   // Send buffer: the local portion of the result array for the current process
        local_rows * length,           // Number of elements to send from the send buffer
        MPI_INT,                       // Data type of each element in the send buffer (integer in this case)
        gathered_result,               // Receive buffer: the buffer where the root process will gather data
        local_rows * length,           // Number of elements to receive from each process
        MPI_INT,                       // Data type of each element in the receive buffer (integer in this case)
        0,                             // Root process (the process that will receive the gathered data)
        MPI_COMM_WORLD                 // Communicator (in this case, the world communicator)
    );

    if (rank == 0) {
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

        printf("Time: %f seconds\n", elapsed_time);

        if (verbose && length < 20) {
            printf("Matrix A:\n");
            mostrarMatriz(matrixA, length);

            printf("Matrix B:\n");
            mostrarMatriz(matrixB, length);

            printf("Result:\n");
            mostrarMatriz(gathered_result, length);
        }
    }

    MPI_Finalize();
    free(matrixA);
    free(matrixB);
    free(result);
    free(gathered_result);

    return 0;
}
