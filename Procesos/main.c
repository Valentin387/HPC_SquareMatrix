#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#define NUM_THREADS		16

void printMatrix(int** matrix, int rows, int cols);

// structure for storing the data that each thread needs
struct thread_data{
	int thread_id; //who am I
	int lowerLimit; //the range of values from matrix A which I'm responsible of
	int upperLimit;
	int Bcols; //how big are your square matrices
	int** A; //the square matrices
	int** B;
	int** subResult; //where I store the result of my execution
};

//I create as many structures of this type as threads there will be
struct thread_data thread_data_array[NUM_THREADS];

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
void *multiplyMatrices(void *threadarg) {
	//local variables for the multiplication
	int i;
	int j;
	int k;
	//I need to decode the arguments through the structure I created above
	struct thread_data *my_data;
	
	//bringing the arguments for this thread can work
	my_data= (struct thread_data *) threadarg;
	int taskID = my_data->thread_id;
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
    pthread_exit((void *)(intptr_t)taskID);
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
    int C = N / NUM_THREADS;	//quotient of N/num_threads
    int R = N % NUM_THREADS;	//remainder of N/num_threads

	//I allocate memory for the matrices
    int** A = allocateMatrix(N);
    int** B = allocateMatrix(N);
    int** result = allocateMatrix(N);
    
    //I initialize the random numbers
    srand(time(NULL));
    
    //I fill the matrices
    fillMatrix(A, N);
    fillMatrix(B, N);
    
    //thread science
    pthread_t threads[NUM_THREADS]; //I create the threads I need
    pthread_attr_t attr; //an argument just to specify that I need my threads to be joinable
    void *status; // same as above
    
    //I create and keep prepared the IDs for each thread
	int *taskIDS[NUM_THREADS];
	int rc; //in case of an error in thread creation
	int t;  //variable for counting threads
    int lowerLimit; //variables for assigning which thread will work on what range of A matrix (what rows)
    int upperLimit;
    
    /* Initialize and set thread detached attribute for joining*/
   	pthread_attr_init(&attr);
   	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    //I write down the machine time
    clock_t start_time = clock();
	
	for(t=0,lowerLimit=0, upperLimit=C-1; t < NUM_THREADS; t++) {
		//I instantiate a structure that will be assigned to this new thread
		thread_data_array[t].thread_id = t;
		thread_data_array[t].A=A;
		thread_data_array[t].B=B;
		thread_data_array[t].lowerLimit=lowerLimit;
		thread_data_array[t].upperLimit=upperLimit;
		thread_data_array[t].Bcols=N;
	
		//printf("Creating thread %d\n", t);
		
		//in case something goes wrong
		rc = pthread_create(&threads[t], &attr, multiplyMatrices, (void *) 
		   &thread_data_array[t]);
		   
		if (rc) {
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
		
		//I can only use the given amount of threads, and therefore I need to check if the next thread is
        //the last one in order to assign all the remaining rows to it
        if (t+1==NUM_THREADS-1){
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
	
	
	/* Free attribute and wait for the other threads */
   	pthread_attr_destroy(&attr);
    for(t=0; t<NUM_THREADS; t++) { //for every thread, I will check its homework
        rc = pthread_join(threads[t], &status);
        if (rc) {
           printf("ERROR; return code from pthread_join() is %d\n", rc);
           exit(-1);
        }
        //Now I need to build the final matrix based on the subMatrices that each thread has made
        int initialIndex=thread_data_array[t].lowerLimit;
        int finalIndex=thread_data_array[t].upperLimit;
        int** subResult= thread_data_array[t].subResult;
      
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
	
	pthread_exit(NULL);
    return 0;
}


