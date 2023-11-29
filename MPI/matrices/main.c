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

    srand(time(NULL));

    int *matrixA = (int *)malloc(length * length * sizeof(int));
    int *matrixB = (int *)malloc(length * length * sizeof(int));
    int *result = (int *)malloc(length * length * sizeof(int));

    fill_matrix(matrixA, length);
    fill_matrix(matrixB, length);

    MPI_Barrier(MPI_COMM_WORLD);
    clock_gettime(CLOCK_MONOTONIC, &start);

    int rows_per_process = length / size;
    int extra_rows = length % size;
    int start_row = rank * rows_per_process + (rank < extra_rows ? rank : extra_rows);
    int end_row = start_row + rows_per_process + (rank < extra_rows ? 1 : 0);

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < length; j++) {
            result[i * length + j] = 0;
            for (int k = 0; k < length; k++) {
                result[i * length + j] += matrixA[i * length + k] * matrixB[k * length + j];
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

        printf("Time: %f seconds\n", elapsed_time);

        if (verbose && length < 15) {
            printf("Matrix A:\n");
            mostrarMatriz(matrixA, length);

            printf("Matrix B:\n");
            mostrarMatriz(matrixB, length);

            printf("Result:\n");
            mostrarMatriz(result, length);
        }
    }

    MPI_Finalize();
    free(matrixA);
    free(matrixB);
    free(result);

    return 0;
}