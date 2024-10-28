#include <mpi.h>
#include <stdio.h>
#include <math.h>

int main(int argc, char** argv) {
    int comm_sz; // numero de procesos
    int my_rank; // rango de proceso
    int local_val, global_sum;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    local_val = my_rank;// cada proceso tiene un valor local inicial 

    int step = 1;
    while (step < comm_sz) {
        if (my_rank % (2 * step) == 0) {
            int received_val;
            MPI_Recv(&received_val, 1, MPI_INT, my_rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            local_val += received_val;
        } else if (my_rank % step == 0) {
            MPI_Send(&local_val, 1, MPI_INT, my_rank - step, 0, MPI_COMM_WORLD);
            break;
        }
        step *= 2;
    }

    if (my_rank == 0) { // el proceso 0 tendrá la suma global
        global_sum = local_val;
        printf("La suma global es %d\n", global_sum);
    }

    MPI_Finalize();
    return 0;
}
