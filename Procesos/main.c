#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h> // Include for fork()
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define NUM_PROCESSES	2

void printMatrix(int** matrix, int rows, int cols);

// structure for storing the data that each process needs
struct shared_data{
	int process_id; //who am I
	int lowerLimit; //the range of values from matrix A which I'm responsible of
	int upperLimit;
	int Bcols; //how big are your square matrices
	int** A; //the square matrices
	int** B;
	int** subResult; //where I store the result of my execution
};

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

//deterministic matrix filling method for debugging
void fillMatrixTest(int** matrix, int N) {
	int i;
	int j;
	int cont=1;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            matrix[i][j] =cont;
			cont++;
        }
    }
}

// Function to multiply two matrices
void multiplyMatrices(struct shared_data *threadarg){
	//local variables for the multiplication
	int i;
	int j;
	int k;
	//I need to decode the arguments through the structure I created above
	struct shared_data *my_data;
	
	//bringing the arguments for this thread can work
	my_data= (struct shared_data *) threadarg;
	int taskID = my_data->process_id;
	int** A = my_data->A;
	int** B = my_data->B;
	int taskLowerLimit = my_data->lowerLimit;
	int taskUpperLimit = my_data->upperLimit;
	int Bcols = my_data->Bcols;

    int pieceOfA = taskUpperLimit - taskLowerLimit + 1;
    //I allocate space fo the subResult matrix
    int** subResult=(int**)malloc(pieceOfA * sizeof(int*));
	for (i = 0; i < pieceOfA; i++) {
        subResult[i] = (int*)malloc(Bcols * sizeof(int));
    }

    //I take into account that both A and B have the same number of cols than B.
    //I will checkout row by row, my assigned range of the A matrix
    int ai; //remember that subResult is an abstraction of A, and therefore its indexes start in 0
    int aj;
    for (ai=0,i = taskLowerLimit; i <= taskUpperLimit; ai++,i++) {
    	//I traverse the B matrix
        for (aj=0,j = 0; j < Bcols; aj++,j++) {
            subResult[ai][aj] = 0;
            for (k = 0; k < Bcols; k++) { //here it should be (k < Acols) in a general case
                subResult[ai][aj] += A[i][k] * B[k][j];	//linear combination
            }
        }
    }
    my_data->subResult = subResult;
    printMatrix(my_data->subResult,Bcols,Bcols);
}

// Function to deallocate memory for a matrix
void deallocateMatrix(int** matrix, int N) {
	int i;
    for (i = 0; i < N; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

/*function to print matrices with a nice style*/
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

//main function
int main(int argc, char* argv[]) {
	//exitprompt arguments
    if (argc != 3) {
        printf("Usage: %s N Verbose\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int verbose = atoi(argv[2]);
    //int N = 5;
    //int verbose = 1;
    int C = N / NUM_PROCESSES;	//quotient of N/NUM_PROCESSES
    int R = N % NUM_PROCESSES;	//remainder of N/NUM_PROCESSES

	//I allocate memory for the matrices
    int** A = allocateMatrix(N);
    int** B = allocateMatrix(N);
    int** result = allocateMatrix(N);
    
    //I initialize the random numbers
    srand(time(NULL));

    // Initialize and create the shared memory segment
    int shmid;
    struct shared_data* shared_data_array;

    key_t key = ftok("shared_memory_key", 'R');
    shmid = shmget(key, sizeof(struct shared_data) * NUM_PROCESSES, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    // Attach the shared memory segment to the parent process's address space
    shared_data_array = (struct shared_data*)shmat(shmid, NULL, 0);
    if ((int)(intptr_t)shared_data_array == -1) {
        perror("shmat");
        exit(1);
    }
    
    //I fill the matrices
    fillMatrixTest(A, N);
    fillMatrixTest(B, N);
    
	int t;  //variable for counting threads
    int lowerLimit; //variables for assigning which thread will work on what range of A matrix (what rows)
    int upperLimit;
    
    //I write down the machine time
    clock_t start_time = clock();
	
	for(t=0,lowerLimit=0, upperLimit=C-1; t < NUM_PROCESSES; t++) {

        pid_t child_pid = fork();

        if (child_pid < 0) {
            perror("Fork failed");
            exit(1);
        }else if (child_pid == 0) {
            // Child process
            // Access the shared data from shared_data_array[process_id]
            struct shared_data* my_data = &shared_data_array[t];

            // Perform matrix multiplication using my_data
            // I instantiate a structure that will be assigned to this new thread
            shared_data_array[t].process_id = t;
            shared_data_array[t].A=A;
            shared_data_array[t].B=B;
            shared_data_array[t].lowerLimit=lowerLimit;
            shared_data_array[t].upperLimit=upperLimit;
            shared_data_array[t].Bcols=N;
            printf("after assigning values to the structure of Process %d\n", t);
        
            multiplyMatrices(my_data);
            printf("after the multiplication of Process %d\n", t);        
            //I can only use the given amount of processes, and therefore I need to check if the next thread is
            //the last one in order to assign all the remaining rows to it
        
            // Detach the shared memory segment from the child process
            shmdt(shared_data_array);
            exit(0); // Exit the child process
	    }else{
            printf("Parent: created child with ID %ld\n", (long)child_pid);
            if (t+1==NUM_PROCESSES-1){
                upperLimit=upperLimit+C+R;
                lowerLimit+=C;
            }
            else if (upperLimit+C < N){
                upperLimit+=C;
                lowerLimit+=C;
            }else{
                upperLimit+=R;
                lowerLimit+=C;
            }
        }
    }
	
    for(t=0; t<NUM_PROCESSES; t++) { //for every process, I will check its homework
        wait(NULL);
        printf("Parent: completed join with child %d\n",t);

        //Now I need to build the final matrix based on the subMatrices that each thread has made
        int initialIndex=shared_data_array[t].lowerLimit;
        int finalIndex=shared_data_array[t].upperLimit;
        int** subResult= shared_data_array[t].subResult;
      
	    //local variables to set my final result matrix
	    int numRow;
	    int localNumRow;
	    //for every row assigned to that thread
		for (numRow = initialIndex, localNumRow=0; numRow <= finalIndex; numRow++, localNumRow++) {
	        result[numRow] = subResult[localNumRow];
		}
      	//printf("Main: completed join with thread %ld having a status of %ld\n",t,status);
   }
    
	//I write down the machine time
    clock_t end_time = clock();
    
    //I substract the 2 times to find out the computing duration
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Elapsed time calculated: %f\n", elapsed_time);   

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
    printf("%.6f\n", elapsed_time);

	//I free the memory once allocated to these matrices
    deallocateMatrix(A, N);
    deallocateMatrix(B, N);
    deallocateMatrix(result, N);

    // Detach the shared memory segment from the parent process
    shmdt(shared_data_array);

    // Remove the shared memory segment (optional, use IPC_RMID carefully)
    shmctl(shmid, IPC_RMID, NULL);
	
    return 0;
}


