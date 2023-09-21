#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>

void mostrarMatriz(int *matrix, int length) {
    int j;
    for (int i = 0; i < length; i++) {
        for ( j = 0; j < length; j++) {
            printf("%4d |", matrix[i * length + j]);// Adjust the width (4) as needed
        }
        printf("\n");
        for ( j = 0; j < length-1; j++) {
            printf("------+"); // Print horizontal separators
        }
        printf("\n");
    }
}

// Function to fill a matrix with random values
void fill_matrix(int *matrix, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i * N + j] = rand() % 10;
        }
    }
}


//deterministic matrix filling method for debugging
void deterministic_fill_matrix(int *matrix, int N) {
    int cont;
    cont=1;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i * N + j] = cont;
            cont++;
        }
    }
}


int main(int argc, char *argv[]) {
    struct timespec start, end;
    double elapsed_time;
    clock_gettime(CLOCK_MONOTONIC, &start);
    //NUM_PROCESSES
    int num_processes = 2;

    int length = atoi(argv[1]);
    int max_number = 100;
    int verbose = atoi(argv[2]);
    int rows_per_process = length / num_processes;
    int start_row = 0;
    int shmid;
    int *shared_result;

    srand(time(NULL));

    /*if (argc > 3 && strcmp(argv[3], "-v") == 0) {
        verbose = 1;
    }*/

    int *matrixA = (int *)malloc(length * length * sizeof(int));
    int *matrixB = (int *)malloc(length * length * sizeof(int));

    fill_matrix(matrixA, length);
    fill_matrix(matrixB, length);

    //Memoria Compartida
    shmid = shmget(IPC_PRIVATE, length * length * sizeof(int), IPC_CREAT | 0666);
    if (verbose && shmid < 0) {
        perror("shmget");
        exit(1);
    }

    shared_result = shmat(shmid, NULL, 0);

    for (int i = 0; i < num_processes; i++) {
        int end_row = (i == num_processes - 1) ? length : start_row + rows_per_process;

        pid_t pid = fork();

        if (pid == 0) {  //Proceso Hijo
            for (int i = start_row; i < end_row; i++) {
                for (int j = 0; j < length; j++) {
                    shared_result[i * length + j] = 0;
                    for (int k = 0; k < length; k++) {
                        shared_result[i * length + j] += matrixA[i * length + k] * matrixB[k * length + j];
                    }
                }
            }
            shmdt(shared_result);//Libera Memoria compartida
            exit(0);
        } else if (pid < 0) {
            fprintf(stderr, "Error al crear el proceso hijo.\n");
            return 1;
        }

        start_row = end_row;
    }

    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    if(verbose && length<15){
        printf("Matriz A:\n\n");
        mostrarMatriz(matrixA, length);

        printf("Matriz B:\n\n");
        mostrarMatriz(matrixB, length);

        printf("Result:\n\n");
        mostrarMatriz(shared_result, length);
    }

    free(matrixA);
    free(matrixB);

    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("%f \n", elapsed_time);

    shmctl(shmid, IPC_RMID, NULL);//Libera memoria compartida

    return 0;
}