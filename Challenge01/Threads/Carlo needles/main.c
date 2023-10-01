#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#define NUM_THREADS		16

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// Define a structure to represent the floor
struct Floor
{
    double l;
};

// Define a structure to represent a needle
struct Needle
{
    double x;
    double theta;
    double L;
};

// structure for storing the data that each thread needs
struct thread_data{
    int thread_id;
	int hits;
    int lowerLimit; //the range of values from matrix A which I'm responsible of
	int upperLimit;
    double L;
    struct Floor floor;
    
};

//I create as many structures of this type as threads there will be
struct thread_data thread_data_array[NUM_THREADS];

// Function to toss a needle on the floor
struct Needle toss_needle(double L, struct Floor floor)
{
    struct Needle needle;
    needle.x = ((double)rand() / RAND_MAX) * floor.l;
    needle.theta = ((double)rand() / RAND_MAX) * M_PI;
    needle.L = L;
    return needle;
}

// Function to check if a needle crosses a line on the floor
int cross_line(struct Needle needle, struct Floor floor)
{
    double x_right_tip = needle.x + (needle.L / 2) * sin(needle.theta);
    double x_left_tip = needle.x - (needle.L / 2) * sin(needle.theta);
    return x_right_tip > floor.l || x_left_tip < 0.0;
}


void *estimationf(void *threadarg) {

	//I need to decode the arguments through the structure I created above
	struct thread_data *my_data;
	
	//bringing the arguments for this thread can work
	my_data= (struct thread_data *) threadarg;
	int taskID = my_data->thread_id;
	int taskLowerLimit = my_data->lowerLimit;
	int taskUpperLimit = my_data->upperLimit;
    int nb_crosses = my_data->hits;
    double L = my_data->L;
    struct Floor floor = my_data->floor;

    int t;

    for (t = taskLowerLimit; t < taskUpperLimit; t++)
    {
        struct Needle needle = toss_needle(L, floor);
        if (cross_line(needle, floor))
        {
            nb_crosses++;
        }
    }

    //printf("th: %d\n my_data->hits: %d\nnb_crosses: %d\n",taskID,my_data->hits,nb_crosses);
    my_data->hits = nb_crosses;
    //printf("th: %d\n my_data->hits: %d\nnb_crosses: %d\n",taskID,my_data->hits,nb_crosses);
    pthread_exit((void *)(intptr_t)taskID);

}


// Function to estimate the probability of a needle crossing a line
double estimate_prob_needle_crosses_line(int nb_tosses, struct Floor floor, double L)
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
        thread_data_array[th].L = L;
        thread_data_array[th].floor = floor;
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
        nb_crosses += threadHits;
      	//printf("Main: completed join with thread %d having a status of %ld\n",t,(long)status);
    }
   //printf("b_crosses: %d \n",nb_crosses);

    // Return the fraction of needles that cross a line
    return (double)nb_crosses / nb_tosses;
}

int main(int argc, char *argv[])
{                      // floor, L, nb_tosses
    srand(time(NULL)); // Seed the random number generator with the current time

    struct Floor floor;
    floor.l = 2; // Set the distance between parallel lines - parameter

    double L = 1; // Set the length of the needle - parameter

    int nb_tosses = atoi(argv[1]); // Set the number of needle tosses - parameter

    // I write down the machine time
    clock_t start_time = clock();

    double probability = estimate_prob_needle_crosses_line(nb_tosses, floor, L);
    // using the formula, this probability should be pi powered to -1

    // I write down the machine time
    clock_t end_time = clock();

    // I substract the 2 times to find out the computing duration
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // printf("Expected probability: %lf\n", 2 * L / (M_PI * floor.l));
    printf("Estimated probability: %lf Time: %.6f\n", 1 / probability, elapsed_time);
    //printf("\nEnd of line");

    return 0;
}
