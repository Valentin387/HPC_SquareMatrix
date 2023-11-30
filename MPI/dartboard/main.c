#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = atoi(argv[1]); // Total number of throws - parameter
    int local_n = n / size; // Number of throws per process
    int remaining_n = n % size; // Remaining throws to be distributed among processes

    int local_hits = 0; // Number of hits for each process
    int global_hits; // Total number of hits across all processes

    srand(time(NULL) + rank); // Seed the random number generator with the current time and rank

    // I write down the machine time only for rank 0
    double start_time;
    if (rank == 0) {
        start_time = MPI_Wtime();
    }

    // Calculate the number of throws for the current process
    int throws_for_current_process = local_n + (rank == size - 1 ? remaining_n : 0);
    //printf("Rank %d will throw %d darts\n", rank, throws_for_current_process);

    int i;
    for (i = 0; i < throws_for_current_process; i++) {
        double x = (double)rand() / RAND_MAX; // Random x-coordinate between 0 and 1
        double y = (double)rand() / RAND_MAX; // Random y-coordinate between 0 and 1

        double distance = x * x + y * y; // Calculate the distance from the origin

        if (distance < 1.0) {
            local_hits++; // Increment the number of hits if the point is inside the circle
        }
    }
    //printf("Rank %d has %d hits\n", rank, local_hits);

    // Sum the hits across all processes
    MPI_Reduce(&local_hits, &global_hits, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // I write down the machine time only for rank 0
    if (rank == 0) {
        double end_time = MPI_Wtime();
        // I subtract the 2 times to find out the computing duration
        double elapsed_time = end_time - start_time;

        //printf("Total number of hits: %d\n", global_hits);

        double pi_approximation = 4.0 * (double)global_hits / n; // Approximate pi using the hits and throws

        printf("Approximated value of pi: %lf Time: %.6f\n", pi_approximation, elapsed_time);
    }

    MPI_Finalize();

    return 0;
}
