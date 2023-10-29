#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>
double start; 
double end;

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
            matrix[i][j] = rand() % 10; // Filling with random values from 0 to 9
        }
    }
}

// Function to multiply two matrices
int** multiplyMatrices(int** A, int** B, int N, int num_threads) {
	int i;
	int j;
	int k;
	omp_set_num_threads(num_threads);

    int** result = allocateMatrix(N);
    
    #pragma omp parallel for private(i, j, k)

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
	int i;
	int j;
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


int main(int argc, char* argv[]) {
	//prompt arguments
    if (argc != 4) {
        printf("Usage: %s N Verbose num_threads\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int verbose = atoi(argv[2]);
    int num_threads = atoi(argv[3]); // Get the number of threads from the command line
    
	//I initialize the random numbers
    srand(time(NULL));

	//I allocate memory for the matrices
    int** A = allocateMatrix(N);
    int** B = allocateMatrix(N);
    
    //I fill the matrices
    fillMatrix(A, N);
    fillMatrix(B, N);
    
    
	//I write down the machine time
    //clock_t start_time = clock();
    start = omp_get_wtime(); 
    
	//I carry on the multiplication
    int** result = multiplyMatrices(A, B, N, num_threads);
    
	//I write down the machine time
    //clock_t end_time = clock();
    end = omp_get_wtime(); 
    
    //I substract the 2 times to find out the computing duration
    //double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

	//printing logic
    if (verbose==1 && N<20) {
        printf("\n\nMatrix A:\n");
        printMatrix(A,N,N);
        
        printf("\n\nMatrix B:\n");
        printMatrix(B,N,N);
        
        printf("\n\nResult Matrix:\n");
        printMatrix(result,N,N);
    }

	//Result
    //printf("%.6f\n", elapsed_time);
    printf("%.6f\n", end-start);

	//I free the memory once allocated to these matrices
    deallocateMatrix(A, N);
    deallocateMatrix(B, N);
    deallocateMatrix(result, N);

    return 0;
}


