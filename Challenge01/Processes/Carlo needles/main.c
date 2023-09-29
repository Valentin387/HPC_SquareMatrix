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
    int nb_crosses = 0;
    int t;

    for (t = 0; t < nb_tosses; t++)
    {
        struct Needle needle = toss_needle(L, floor);
        if (cross_line(needle, floor))
        {
            nb_crosses++;
        }
    }

    // Return the fraction of needles that cross a line
    return (double)nb_crosses / nb_tosses;
}

int main(int argc, char *argv[])
{                      // floor, L, nb_tosses
    srand(time(NULL)); // Seed the random number generator with the current time

    int numProcesses = 2;

    struct Floor floor;
    floor.l = 2; // Set the distance between parallel lines - parameter

    double L = 1; // Set the length of the needle - parameter

    int nb_tosses = atoi(argv[1]); // Set the number of needle tosses - parameter

    int C = nb_tosses / numProcesses;	//quotient of N/num_threads
    int R = nb_tosses % numProcesses;	//remainder of N/num_threads

    int shmid;
    int lowerLimit;
    int upperLimit;
    int *shared_result;

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
