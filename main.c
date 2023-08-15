#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

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
	int i;
	int j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            matrix[i][j] = (int)rand() % 10; // Filling with random values from 0 to 9
        }
    }
}

// Function to multiply two matrices
int** multiplyMatrices(int** A, int** B, int N) {
	int i;
	int j;
	int k;
	
    int** result = allocateMatrix(N);
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            result[i][j] = 0;
            for (k = 0; k < N; k++) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return result;
}

// Function to deallocate memory for a matrix
void deallocateMatrix(int** matrix, int N) {
	int i;
    for (i = 0; i < N; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void printMatrix(int** matrix, int rows, int cols) {
    printf("\nMatrix:\n");
    int i = 0;
    int j = 0;
    for (i = 0; i < rows; i++) {
        printf("|");
        for (j = 0; j < cols; j++) {
            printf(" %6d ", matrix[i * cols + j]); // Use a width of 4 for each element
            if (j != cols - 1) {
                printf("|");
            }
        }
        printf("|\n");
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s N Verbose\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    bool verbose = atoi(argv[2]);

    srand(time(NULL));

    int** A = allocateMatrix(N);
    int** B = allocateMatrix(N);
    
    fillMatrix(A, N);
    fillMatrix(B, N);

    clock_t start_time = clock();

    int** result = multiplyMatrices(A, B, N);

    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    if (verbose) {
        printf("Matrix A:\n");
        printMatrix(A,N,N);
        
        printf("Matrix B:\n");
        printMatrix(B,N,N);
        
        printf("Result Matrix:\n");
        printMatrix(result,N,N);
    }

    printf("Matrix multiplication took %.6f seconds.\n", elapsed_time);

    deallocateMatrix(A, N);
    deallocateMatrix(B, N);
    deallocateMatrix(result, N);

    return 0;
}


