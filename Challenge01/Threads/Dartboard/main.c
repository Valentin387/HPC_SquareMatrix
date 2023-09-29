#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#define NUM_THREADS		2

// structure for storing the data that each thread needs
struct thread_data{
    int thread_id;
	int hits;
    int lowerLimit; //the range of values from matrix A which I'm responsible of
	int upperLimit;
    
};

//I create as many structures of this type as threads there will be
struct thread_data thread_data_array[NUM_THREADS];


void *estimationf(void *threadarg) {

	//I need to decode the arguments through the structure I created above
	struct thread_data *my_data;
	
	//bringing the arguments for this thread can work
	my_data= (struct thread_data *) threadarg;
	int taskID = my_data->thread_id;
	int taskLowerLimit = my_data->lowerLimit;
	int taskUpperLimit = my_data->upperLimit;
    int t = my_data->hits;

    int i;
    for (i = taskLowerLimit; i <= taskUpperLimit-1; i++) {
        double x = (double)rand() / RAND_MAX; // Random x-coordinate between 0 and 1
        double y = (double)rand() / RAND_MAX; // Random y-coordinate between 0 and 1

        double distance = x * x + y * y; // Calculate the distance from the origin

        if (distance < 1.0) {
            t++; // Increment the number of hits if the point is inside the circle
        }
    }

    //printf("th: %d\n my_data->hits: %d\nnb_crosses: %d\n",taskID,my_data->hits,nb_crosses);
    my_data->hits = t;
    //printf("th: %d\n my_data->hits: %d\n t: %d\n",taskID,my_data->hits,t);
    pthread_exit((void *)(intptr_t)taskID);

}


// Function to estimate the probability of a needle crossing a line
double estimate_prob_hits(int nb_tosses)
{
    int C = nb_tosses / NUM_THREADS;	//quotient of N/num_threads
    int R = nb_tosses % NUM_THREADS;	//remainder of N/num_threads
    int nb_crosses = 0;


    //thread science
    pthread_t threads[NUM_THREADS]; //I create the threads I need
    pthread_attr_t attr; //an argument just to specify that I need my threads to be joinable
    void *status; // same as above

    int rc; //in case of an error in thread creation
	int th;  //variable for counting threads

    int lowerLimit; //variables for assigning which thread will work on what range of A matrix (what rows)
    int upperLimit;

    // Initialize and set thread detached attribute for joining
   	pthread_attr_init(&attr);
   	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (th = 0,lowerLimit=0, upperLimit=C-1; th < NUM_THREADS; th++){
        thread_data_array[th].thread_id = th;
        thread_data_array[th].lowerLimit = lowerLimit;
        thread_data_array[th].upperLimit = upperLimit;
        //printf("th: %d\nlowerLimit: %d\nupperLimit: %d \n",th,lowerLimit,upperLimit);


        rc = pthread_create(&threads[th], &attr, estimationf, (void *) 
		   &thread_data_array[th]);
		   
		if (rc) {
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}

        //I can only use the given amount of threads, and therefore I need to check if the next thread is
        //the last one in order to assign all the remaining rows to it
        if (th+1==NUM_THREADS-1){
            upperLimit=upperLimit+C+R;
            lowerLimit+=C;
        }
		else if (upperLimit+C < nb_tosses){
			upperLimit+=C;
			lowerLimit+=C;
		}else{
			upperLimit+=R;
			lowerLimit+=C;
		}

    }

    // Free attribute and wait for the other threads
   	pthread_attr_destroy(&attr);
    for(th=0; th<NUM_THREADS; th++) { //for every thread, I will check its homework
        rc = pthread_join(threads[th], &status);
        if (rc) {
           printf("ERROR; return code from pthread_join() is %d\n", rc);
           exit(-1);
        }
        //Now I need to build the count of crosses I had
        int threadHits=thread_data_array[th].hits;
        //printf("\nth: %d\n threadHits: %d", th, threadHits);
        nb_crosses += threadHits;
        //printf("nb_crosses: %d \n",nb_crosses);
      	//printf("Main: completed join with thread %d having a status of %ld\n",t,(long)status);
    }
   //printf("final nb_crosses: %d \n",nb_crosses);

    // Return the fraction of needles that cross a line
    return nb_crosses;
}


int main(int argc, char *argv[]) {
    int n =  atoi(argv[1]); // Number of throws - parameter
    int t = 0; // Number of hits

    //printf("Enter the number of throws: ");
    //scanf("%d", &n);

    srand(time(NULL)); // Seed the random number generator with the current time
    // I write down the machine time
    clock_t start_time = clock();

    t = estimate_prob_hits(n);
    //printf("t = %d", t);

    double pi_approximation = 4.0 * (double)t / n; // Approximate pi using the hits and throws

    // I write down the machine time
    clock_t end_time = clock();

    // I substract the 2 times to find out the computing duration
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    printf("Approximated value of pi: %lf time: %.6f \n", pi_approximation, elapsed_time);

    return 0;
}
