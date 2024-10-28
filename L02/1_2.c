#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

int main(int argc, char* argv[]) {
    long long int number_of_tosses;
    long long int number_in_circle = 0;
    long long int local_number_in_circle = 0;
    int my_rank, comm_sz;
    long long int toss;
    double x, y, distance_squared;
    unsigned int seed;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if (my_rank == 0) {
        if (argc != 2) {
            fprintf(stderr, "Uso: %s <number_of_tosses>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        number_of_tosses = atoll(argv[1]);
    }

    MPI_Bcast(&number_of_tosses, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);

    long long int local_tosses = number_of_tosses / comm_sz;

    seed = (unsigned int) time(NULL) + my_rank;

    for (toss = 0; toss < local_tosses; toss++) {
        x = (double) rand_r(&seed) / RAND_MAX * 2.0 - 1.0; 
        y = (double) rand_r(&seed) / RAND_MAX * 2.0 - 1.0; // aleaotrio: -1  1
        distance_squared = x * x + y * y;
        if (distance_squared <= 1.0) {
            local_number_in_circle++;
        }
    }

    MPI_Reduce(&local_number_in_circle, &number_in_circle, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        double pi_estimate = 4 * ((double) number_in_circle) / ((double) number_of_tosses);
        printf("Estimated value of pi: %f\n", pi_estimate);
    }

    MPI_Finalize();
    return 0;
}
