#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//Valentín Valencia Valencia. UTP Pereira, Risaralda, Colombia

void fillMatrix(int matrix[], int rows, int cols, int fillMethod) {
    if (fillMethod == 1) {
        printf("Enter the elements of the matrix:\n");
        int i = 0;
        int j = 0;
        for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {
                scanf("%d", &matrix[i * cols + j]);
            }
        }
    } else if (fillMethod == 2) {
        srand(time(NULL));
        int i = 0;
        int j = 0;
        for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {
                matrix[i * cols + j] = rand() % 101;
            }
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

void printMatrix(int matrix[], int rows, int cols) {
	int i = 0;
    int j = 0;
    for ( i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            printf("%d ", matrix[i * cols + j]);
        }
        printf("\n");
    }
}

int main() {
    int option;
    int rows1, cols1, rows2, cols2;
    int fillMethod;
    
    while (1) {
        printf("\nMatrix Multiplication Program\n");
        printf("1) Generate new multiplication\n");
        printf("2) End program\n");
        printf("Enter your choice: ");
        scanf("%d", &option);
        
        if (option == 1) {
            printf("Enter the dimensions (rows and columns) of the first matrix: ");
            scanf("%dx%d", &rows1, &cols1);
            printf("Enter the dimensions (rows and columns) of the second matrix: ");
            scanf("%dx%d", &rows2, &cols2);
            
            if (cols1 != rows2) {
                printf("Matrix dimensions are not suitable for multiplication.\n");
                continue;
            }
            
            printf("Choose filling method:\n");
            printf("1) Fill element by element\n");
            printf("2) Generate random values (0 to 100)\n");
            printf("Enter your choice: ");
            scanf("%d", &fillMethod);
            
            int matrix1[rows1][cols1];
            int matrix2[rows2][cols2];
            int result[rows1][cols2];
            
            fillMatrix(&matrix1[0][0], rows1, cols1, fillMethod);
            fillMatrix(&matrix2[0][0], rows2, cols2, fillMethod);
            
            multiplyMatrices(&result[0][0], &matrix1[0][0], &matrix2[0][0], rows1, cols1, cols2);
            
            printf("\nMatrix 1:\n");
            printMatrix(&matrix1[0][0], rows1, cols1);
            
            printf("\nMatrix 2:\n");
            printMatrix(&matrix2[0][0], rows2, cols2);
            
            printf("\nResulting Matrix:\n");
            printMatrix(&result[0][0], rows1, cols2);
        } else if (option == 2) {
            printf("Program ended.\n");
            break;
        } else {
            printf("Invalid option. Please enter 1 or 2.\n");
        }
    }
    
    system("PAUSE");
    return 0;
}

