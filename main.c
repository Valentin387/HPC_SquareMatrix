#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

//Valentín Valencia Valencia. UTP Pereira, Risaralda, Colombia

void fillMatrix(int matrix[], int rows, int cols) {

    srand(time(NULL));
    int i = 0;
    int j = 0;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            matrix[i * cols + j] = rand() % 101;
        }
    }
    
}

void multiplyMatrices(int result[], int matrix1[], int matrix2[], int rows1, int cols1, int cols2) {
	int i = 0;
    int j = 0;
    int k = 0;
    for (i = 0; i < rows1; i++) {
        for (j = 0; j < cols2; j++) {
            result[i * cols2 + j] = 0;
            for (k = 0; k < cols1; k++) {
                result[i * cols2 + j] += matrix1[i * cols1 + k] * matrix2[k * cols2 + j];
            }
        }
    }
}

void printMatrix(int *matrix, int rows, int cols) {
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
    int rows1=N;
	int cols1=N; 
	int rows2=N;
	int cols2=N;
    
    int matrix1[rows1][cols1];
    int matrix2[rows2][cols2];
    int result[rows1][cols2];
    
    fillMatrix(&matrix1[0][0], rows1, cols1);
    fillMatrix(&matrix2[0][0], rows2, cols2);
    
	clock_t start_time = clock();
    
    multiplyMatrices(&result[0][0], &matrix1[0][0], &matrix2[0][0], rows1, cols1, cols2);
    
    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    printf("Matrix multiplication took %.6f seconds.\n", elapsed_time);

    
    if (verbose){
    	printf("\nMatrix 1:\n");
	    printMatrix(&matrix1[0][0], rows1, cols1);
	    
	    printf("\nMatrix 2:\n");
	    printMatrix(&matrix2[0][0], rows2, cols2);
	    
	    printf("\nResulting Matrix:\n");
	    printMatrix(&result[0][0], rows1, cols2);
	}

    system("PAUSE");
    return 0;
}

