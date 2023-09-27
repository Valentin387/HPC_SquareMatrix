#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int main() {
    int n = 3000; // Number of throws
    int t = 0; // Number of hits

    //printf("Enter the number of throws: ");
    //scanf("%d", &n);

    srand(time(NULL)); // Seed the random number generator with the current time

    int i;
    for (i = 0; i < n; i++) {
        double x = (double)rand() / RAND_MAX; // Random x-coordinate between 0 and 1
        double y = (double)rand() / RAND_MAX; // Random y-coordinate between 0 and 1

        double distance = x * x + y * y; // Calculate the distance from the origin

        if (distance < 1.0) {
            t++; // Increment the number of hits if the point is inside the circle
        }
    }

    double pi_approximation = 4.0 * (double)t / n; // Approximate pi using the hits and throws

    printf("Approximated value of pi: %lf\n", pi_approximation);

    return 0;
}
