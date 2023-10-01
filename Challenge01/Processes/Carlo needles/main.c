#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NUM_PROCESSES 8

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

// Function to estimate the probability of a needle crossing a line
double estimate_prob_needle_crosses_line(int nb_tosses, struct Floor floor, double L)
{
    int C = nb_tosses / NUM_PROCESSES;	//quotient of N/num_threads
    int R = nb_tosses % NUM_PROCESSES;	//remainder of N/num_threads

    int shmid;
    int lowerLimit=0;
    int upperLimit=C;
    double nb_crosses[NUM_PROCESSES]; // Use an array of type double
    int t;

    shmid = shmget(IPC_PRIVATE, NUM_PROCESSES * sizeof(double), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    double *shared_nb_crosses = (double *)shmat(shmid, NULL, 0);

    int i;
    pid_t pid;
    for (i = 0; i < NUM_PROCESSES; i++) {
        pid = fork();

        if(pid < 0){
            printf("Error al crear el proceso hijo.\n");
            return 1;
        }
        if (pid == 0) {  //Proceso Hijo
            unsigned int seed = time(NULL) ^ getpid();
            srand(seed); // Seed the random number generator with the current time
            double local_nb_crosses = 0; // Each process has its local count

            for (t = lowerLimit; t < upperLimit; t++)
            {
                //printf("Process: %d toss: %d rand test: %f\n",i,t,(double)rand());
                struct Needle needle = toss_needle(L, floor);
                if (cross_line(needle, floor))
                {
                    local_nb_crosses++;
                    //printf("Process: %d - local_nb_crosses: %f\n",i,local_nb_crosses);
                }
            }

            //printf("Process: %d\nlowerLimit: %d\nupperLimit: %d\nlocal_nb_crosses: %f\n",i,lowerLimit,upperLimit,local_nb_crosses);
            shared_nb_crosses[i] = local_nb_crosses; // Store the local count in the shared array}
            //printf("shared_nb_crosses[%d]: %f \n",i,local_nb_crosses);

            //int h;
            //I print the result shared array to check its state
            //for (h = 0; h < NUM_PROCESSES; h++) {
                //printf("\t Element %d: %lf\n", h, shared_nb_crosses[h]);
            //}

            shmdt(shared_nb_crosses);  
            exit(42);
        }else{
            //I can only use the given amount of threads, and therefore I need to check if the next thread is
            //the last one in order to assign all the remaining rows to it
            if (i+1==NUM_PROCESSES-1){
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
    }
    
    for (i = 0; i < NUM_PROCESSES; i++) {
        wait(NULL);
    }

    // Sum all the local counts from different processes
    double total_nb_crosses = 0;
    for (i = 0; i < NUM_PROCESSES; i++)
    {
        total_nb_crosses += shared_nb_crosses[i];
    }


    shmctl(shmid, IPC_RMID, NULL); // Release shared memory
    //printf("total: %f \n",total_nb_crosses);

    // Return the fraction of needles that cross a line
    return (double)total_nb_crosses / nb_tosses;
}

int main(int argc, char *argv[])
{                      // floor, L, nb_tosses
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

    return 0;
}
