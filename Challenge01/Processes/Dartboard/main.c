#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define NUM_PROCESSES 16

int main(int argc, char *argv[]) {
    int nb_tosses = atoi(argv[1]); // Number of throws - parameter
    int local_nb_crosses = 0; // Number of hits

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

    // I write down the machine time
    clock_t start_time = clock();

    int i;
    pid_t pid;
    for (i = 0; i < NUM_PROCESSES; i++) {
        pid = fork();

        if(pid < 0){
            printf("Error al crear el proceso hijo.\n");
            return 1;
        }if (pid == 0) {  //Proceso Hijo
            unsigned int seed = time(NULL) ^ getpid();
            srand(seed); // Seed the random number generator with the current time
            double local_nb_crosses = 0; // Each process has its local count

            for (t = lowerLimit; t < upperLimit; t++) {
                double x = (double)rand() / RAND_MAX; // Random x-coordinate between 0 and 1
                double y = (double)rand() / RAND_MAX; // Random y-coordinate between 0 and 1

                double distance = x * x + y * y; // Calculate the distance from the origin

                if (distance < 1.0) {
                    local_nb_crosses++; // Increment the number of hits if the point is inside the circle
                }
            }
            //printf("Process: %d\nlowerLimit: %d\nupperLimit: %d\nlocal_nb_crosses: %f\n",i,lowerLimit,upperLimit,local_nb_crosses);
            shared_nb_crosses[i] = local_nb_crosses;
            //int h;
            //for (h = 0; h < NUM_PROCESSES; h++) {
              //  printf("\t Element %d: %lf\n", h, shared_nb_crosses[h]);
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

    double pi_approximation = 4.0 * (double)total_nb_crosses / nb_tosses; // Approximate pi using the hits and throws

    // I write down the machine time
    clock_t end_time = clock();

    // I substract the 2 times to find out the computing duration
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("Approximated value of pi: %lf Time: %f\n", pi_approximation, elapsed_time);

    return 0;
}

