#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int rank, comm_sz, n;
    double *matrix = NULL, *vector = NULL, *local_matrix, *local_result, *result;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    n = 4; // asumir que n es divisible de manera uniforme por comm_sz
    int local_n = n / comm_sz;

    // asignar memoria para el bloque de matriz local y el vector
    local_matrix = (double *)malloc(local_n * n * sizeof(double));
    vector = (double *)malloc(n * sizeof(double));
    local_result = (double *)malloc(local_n * sizeof(double));
    if (rank == 0) result = (double *)malloc(n * sizeof(double));

    if (rank == 0) { // el proceso 0 lee la matriz y el vector
        matrix = (double *)malloc(n * n * sizeof(double));
        for (int i = 0; i < n; i++) { // inicializar la matriz y el vector para demostración
            vector[i] = 1.0;
            for (int j = 0; j < n; j++) {
                matrix[i * n + j] = i * n + j;
            }
        }

        for (int p = 0; p < comm_sz; p++) { // distribuir los bloques de matriz a todos los procesos
            if (p == 0) {
                for (int i = 0; i < local_n * n; i++) { // copiar bloque a local_matrix para el proceso 0
                    local_matrix[i] = matrix[i];
                }
            } else {
                MPI_Send(matrix + p * local_n * n, local_n * n, MPI_DOUBLE, p, 0, MPI_COMM_WORLD);
            }
        }
        free(matrix);
    } else {
        MPI_Recv(local_matrix, local_n * n, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // otros procesos reciben su bloque de la matriz

    }

    MPI_Bcast(vector, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);// transmitir el vector a todos los procesos

    for (int i = 0; i < local_n; i++) {// realizar la multiplicación de matriz-vectores local

        local_result[i] = 0.0;
        for (int j = 0; j < n; j++) {
            local_result[i] += local_matrix[i * n + j] * vector[j];
        }
    }

    int recvcounts[comm_sz];
    for (int i = 0; i < comm_sz; i++) {
        recvcounts[i] = local_n;
    }

    MPI_Reduce_scatter(local_result, result, recvcounts, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Vector resultado:\n");
        for (int i = 0; i < n; i++) {
            printf("%f ", result[i]);
        }
        printf("\n");
    }

    free(local_matrix);
    free(vector);
    free(local_result);
    if (rank == 0) free(result);

    MPI_Finalize();
    return 0;
}
