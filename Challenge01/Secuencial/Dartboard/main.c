#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int main(int argc, char *argv[]) {
    int n = atoi(argv[1]); // Number of throws - parameter
    int t = 0; // Number of hits

    //printf("Enter the number of throws: ");
    //scanf("%d", &n);

    srand(time(NULL)); // Seed the random number generator with the current time

    // I write down the machine time
    clock_t start_time = clock();

    int i;
    for (i = 0; i < n; i++) {
        double x = (double)rand() / RAND_MAX; // Random x-coordinate between 0 and 1
        double y = (double)rand() / RAND_MAX; // Random y-coordinate between 0 and 1

        double distance = x * x + y * y; // Calculate the distance from the origin

        if (distance < 1.0) {
            t++; // Increment the number of hits if the point is inside the circle
        }
    }

    // I write down the machine time
    clock_t end_time = clock();

    // I substract the 2 times to find out the computing duration
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    double pi_approximation = 4.0 * (double)t / n; // Approximate pi using the hits and throws

    printf("Approximated value of pi: %lf Time: %.6f\n", pi_approximation, elapsed_time);

    return 0;
}
