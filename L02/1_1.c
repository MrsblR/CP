#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void Usage(char prog_name[]);

void Get_args(char* argv[], int* bin_count_p, float* min_meas_p, float* max_meas_p, int* data_count_p);

void Gen_data(float min_meas, float max_meas, float data[], int data_count);

void Gen_bins(float min_meas, float max_meas, float bin_maxes[], int bin_counts[], int bin_count); // definir los límites de los bins y inicializar los contadores de cada bin

int Which_bin(float data, float bin_maxes[], int bin_count, float min_meas); // determinar a qué bin pertenece un dato específico

void Print_histo(float bin_maxes[], int bin_counts[], int bin_count, float min_meas);

int main(int argc, char* argv[]) {
    int bin_count, i, bin;
    float min_meas, max_meas;
    float* bin_maxes;
    int* bin_counts;
    int data_count;
    float* data;
    int my_rank, comm_sz;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);  // rango del proceso actual
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);  // número total de procesos

    // argumentos
    if (my_rank == 0) { // para el proceso 0
        if (argc != 5) Usage(argv[0]);
        Get_args(argv, &bin_count, &min_meas, &max_meas, &data_count);
    }

    MPI_Bcast(&bin_count, 1, MPI_INT, 0, MPI_COMM_WORLD); // boradcast entre los argumentos de todos los processo
    MPI_Bcast(&min_meas, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&max_meas, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&data_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

    bin_maxes = malloc(bin_count * sizeof(float)); // memoria para los limites de los bins y contadores de cada bin
    bin_counts = malloc(bin_count * sizeof(int));
    for (i = 0; i < bin_count; i++) bin_counts[i] = 0;  // inicializar contadores a 0

    if (my_rank == 0) { // para el proceso 0
        data = malloc(data_count * sizeof(float));
        Gen_data(min_meas, max_meas, data, data_count);  // generar datos aleatorios
        Gen_bins(min_meas, max_meas, bin_maxes, bin_counts, bin_count);  // definir límites de los bins
    }

    MPI_Bcast(bin_maxes, bin_count, MPI_FLOAT, 0, MPI_COMM_WORLD); // broadcast en tre limites de bins

    int local_data_count = data_count / comm_sz; // dividir datos entre porcesos
    float* local_data = malloc(local_data_count * sizeof(float));
    MPI_Scatter(data, local_data_count, MPI_FLOAT, local_data, local_data_count, MPI_FLOAT, 0, MPI_COMM_WORLD);

    int* local_bin_counts = malloc(bin_count * sizeof(int)); // memoria para contadores locales
    for (i = 0; i < bin_count; i++) local_bin_counts[i] = 0;  // Inicializar contadores locales a 0

    for (i = 0; i < local_data_count; i++) { // datos locales correspondientes a cada bin
        bin = Which_bin(local_data[i], bin_maxes, bin_count, min_meas);
        local_bin_counts[bin]++;
    }

    MPI_Reduce(local_bin_counts, bin_counts, bin_count, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD); // reducir locales y sumarloa

    if (my_rank == 0) { // histograma final
        Print_histo(bin_maxes, bin_counts, bin_count, min_meas);
        free(data);  // liberar la memoria de los datos generados
    }

    free(bin_maxes);
    free(bin_counts);
    free(local_data);
    free(local_bin_counts);

    MPI_Finalize();
    return 0;
}

void Usage(char prog_name[]) {
    fprintf(stderr, "usage: %s <bin_count> <min_meas> <max_meas> <data_count>\n", prog_name);
    exit(0);
}

void Get_args(char* argv[], int* bin_count_p, float* min_meas_p, float* max_meas_p, int* data_count_p) {
    *bin_count_p = strtol(argv[1], NULL, 10);
    *min_meas_p = strtof(argv[2], NULL);
    *max_meas_p = strtof(argv[3], NULL);
    *data_count_p = strtol(argv[4], NULL, 10);
}

void Gen_data(float min_meas, float max_meas, float data[], int data_count) {
    int i;
    srandom(0); 
    for (i = 0; i < data_count; i++)
        data[i] = min_meas + (max_meas - min_meas) * random() / ((double)RAND_MAX);
}

void Gen_bins(float min_meas, float max_meas, float bin_maxes[], int bin_counts[], int bin_count) { // definir los límites de los bins y inicializar los contadores de cada bin
    float bin_width;
    int i;

    bin_width = (max_meas - min_meas) / bin_count;  // Calcular el ancho de cada bin
    for (i = 0; i < bin_count; i++) {
        bin_maxes[i] = min_meas + (i + 1) * bin_width;  // Definir el límite superior de cada bin
        bin_counts[i] = 0;  // Inicializar el contador de cada bin a 0
    }
}

// determinar a qué bin pertenece un dato específico
int Which_bin(float data, float bin_maxes[], int bin_count, float min_meas) {
    int bottom = 0, top = bin_count - 1;
    int mid;
    float bin_max, bin_min;

    // búsqueda binaria para encontrar el bin correspondiente
    while (bottom <= top) {
        mid = (bottom + top) / 2;
        bin_max = bin_maxes[mid];
        bin_min = (mid == 0) ? min_meas : bin_maxes[mid - 1];
        if (data >= bin_max)
            bottom = mid + 1;
        else if (data < bin_min)
            top = mid - 1;
        else
            return mid;
    }

    // si el dato no pertenece a ningún bin, imprimir error y terminar
    fprintf(stderr, "Data = %f doesn't belong to a bin!\n", data);
    exit(-1);
}

// imprimir el histograma
void Print_histo(float bin_maxes[], int bin_counts[], int bin_count, float min_meas) {
    int i, j;
    float bin_max, bin_min;

    for (i = 0; i < bin_count; i++) {
        bin_max = bin_maxes[i];
        bin_min = (i == 0) ? min_meas : bin_maxes[i - 1];
        printf("%.3f-%.3f:\t", bin_min, bin_max);
        for (j = 0; j < bin_counts[i]; j++)
            printf("X");
        printf("\n");
    }
}