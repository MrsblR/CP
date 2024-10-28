#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int compare(const void *a, const void *b) { // comparar 2 enteros
    return (*(int *)a - *(int *)b);
}

void merge(int *a, int n, int *b, int m, int *result) { // mezclar dos listas ordenadas
    int i = 0, j = 0, k = 0;
    while (i < n && j < m) {
        if (a[i] < b[j]) {
            result[k++] = a[i++];
        } else {
            result[k++] = b[j++];
        }
    }
    while (i < n) {
        result[k++] = a[i++];
    }
    while (j < m) {
        result[k++] = b[j++];
    }
}

int main(int argc, char *argv[]) {
    int rank, comm_sz, n;
    int *local_data = NULL, *recv_data = NULL;
    int local_n;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if (rank == 0) {
        printf("Ingrese el numero de elementos (n): ");
        scanf("%d", &n);
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD); // compartir n a todos los procesos

    local_n = n / comm_sz; // calcular el número de elementos que cada proceso manejará

    local_data = (int *)malloc(local_n * sizeof(int)); // asignar memoria para el arreglo local y llenarlo con números aleatorios

    srand(rank + 1); // generador de números aleatorios
    for (int i = 0; i < local_n; i++) {
        local_data[i] = rand() % 100; // números aleatorios entre 0 y 99
    }

    qsort(local_data, local_n, sizeof(int), compare); // ordenar el arreglo local usando qsort

    if (rank == 0) { // reunir subarreglos ordenados en el proceso 0
        recv_data = (int *)malloc(n * sizeof(int));
    }
    MPI_Gather(local_data, local_n, MPI_INT, recv_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        int step = 1; // Comenzar a mezclar los arreglos en una estructura de árbol
        while (step < comm_sz) {
            for (int i = 0; i < comm_sz; i += 2 * step) {
                if (i + step < comm_sz) {
                    int merge_n = local_n * step * 2;
                    int *temp = (int *)malloc(merge_n * sizeof(int));
                    merge(recv_data + i * local_n * step, local_n * step,
                          recv_data + (i + step) * local_n * step, local_n * step, temp);
                    for (int j = 0; j < merge_n; j++) {
                        recv_data[i * local_n * step + j] = temp[j];
                    }
                    free(temp);
                }
            }
            step *= 2;
        }

        printf("Arreglo ordenado:\n");
        for (int i = 0; i < n; i++) {
            printf("%d ", recv_data[i]);
        }
        printf("\n");

        free(recv_data);
    }

    free(local_data);
    MPI_Finalize();
    return 0;
}
