#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

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
double estimate_prob_needle_crosses_line(int nb_tosses, struct Floor floor, double L, int rank)
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

    //printf("Rank %d: tosses: %d crosses: %d \n", rank, nb_tosses, nb_crosses);

    // Return the fraction of needles that cross a line
    //return (double)nb_crosses / nb_tosses;
    return (double)nb_crosses;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(time(NULL) + rank); // Seed the random number generator with the current time and rank

    struct Floor floor;
    floor.l = 2; // Set the distance between parallel lines - parameter

    double L = 1; // Set the length of the needle - parameter

    int nb_tosses = atoi(argv[1]); // Set the number of needle tosses - parameter

    // Calculate the number of tosses for each process
    int tosses_per_process = nb_tosses / size;
    int remaining_tosses = nb_tosses % size;

    // I write down the machine time only for rank 0
    double start_time;
    if (rank == 0)
    {
        start_time = MPI_Wtime();
    }

    double probability = estimate_prob_needle_crosses_line(tosses_per_process + (rank == size - 1 ? remaining_tosses : 0),
                                                           floor, L, rank);

    // Add a barrier to synchronize processes
    MPI_Barrier(MPI_COMM_WORLD);
    //printf("Rank %d: %lf\n", rank, probability);

    // Reduce probabilities across all processes
    double global_probability;
    MPI_Reduce(&probability, &global_probability, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // I write down the machine time only for rank 0
    if (rank == 0)
    {
        double end_time = MPI_Wtime();
        // I subtract the 2 times to find out the computing duration
        double elapsed_time = end_time - start_time;

        double realProbability = global_probability / nb_tosses;

        // Print the results only from the master process
        printf("Estimated probability: %lf Time: %.6f\n", 1 / realProbability, elapsed_time);
    }

    MPI_Finalize();

    return 0;
}