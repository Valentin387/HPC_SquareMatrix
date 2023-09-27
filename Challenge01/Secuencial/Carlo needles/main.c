#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Define a structure to represent the floor
struct Floor {
    double l;
};

// Define a structure to represent a needle
struct Needle {
    double x;
    double theta;
    double L;
};

// Function to toss a needle on the floor
struct Needle toss_needle(double L, struct Floor floor) {
    struct Needle needle;
    needle.x = ((double)rand() / RAND_MAX) * floor.l;
    needle.theta = ((double)rand() / RAND_MAX) * M_PI;
    needle.L = L;
    return needle;
}

// Function to check if a needle crosses a line on the floor
int cross_line(struct Needle needle, struct Floor floor) {
    double x_right_tip = needle.x + (needle.L / 2) * sin(needle.theta);
    double x_left_tip = needle.x - (needle.L / 2) * sin(needle.theta);
    return x_right_tip > floor.l || x_left_tip < 0.0;
}

// Function to estimate the probability of a needle crossing a line
double estimate_prob_needle_crosses_line(int nb_tosses, struct Floor floor, double L) {
    int nb_crosses = 0;
    int t;

    for (t = 0; t < nb_tosses; t++) {
        struct Needle needle = toss_needle(L, floor);
        if (cross_line(needle, floor)) {
            nb_crosses++;
        }
    }

    // Return the fraction of needles that cross a line
    return (double)nb_crosses / nb_tosses;
}

int main() {
    srand(time(NULL)); // Seed the random number generator with the current time

    struct Floor floor;
    floor.l = 2.0; // Set the distance between parallel lines

    double L = 1.0; // Set the length of the needle

    int nb_tosses = 2000; // Set the number of needle tosses

    double probability = estimate_prob_needle_crosses_line(nb_tosses, floor, L);
    //using the formula, this probability should be pi powered to -1
    
    //printf("Expected probability: %lf\n", 2 * L / (M_PI * floor.l));
    printf("Estimated probability: %lf\n", 1/probability);

    return 0;
}
