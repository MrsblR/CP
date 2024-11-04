#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void Usage(char prog_name[]);
void Get_args(char* argv[], int* bin_count_p, float* min_meas_p, float* max_meas_p, int* data_count_p);
void Gen_data(float min_meas, float max_meas, float data[], int data_count);
void Gen_bins(float min_meas, float max_meas, float bin_maxes[], int bin_counts[], int bin_count);
int Which_bin(float data, float bin_maxes[], int bin_count, float min_meas);
void Print_histo(float bin_maxes[], int bin_counts[], int bin_count, float min_meas);
void* Thread_work(void* rank);

// Variables globales
int thread_count;
int bin_count, data_count;
float min_meas, max_meas;
float* data;
float* bin_maxes;
int* bin_counts;
pthread_mutex_t mutex;

int main(int argc, char* argv[]) {
    long thread; 
    pthread_t* thread_handles;

    if (argc != 6) Usage(argv[0]);
    Get_args(argv, &bin_count, &min_meas, &max_meas, &data_count);
    thread_count = strtol(argv[5], NULL, 10);

    data = malloc(data_count * sizeof(float));
    bin_maxes = malloc(bin_count * sizeof(float));
    bin_counts = malloc(bin_count * sizeof(int));
    thread_handles = malloc(thread_count * sizeof(pthread_t));
    pthread_mutex_init(&mutex, NULL);

    Gen_data(min_meas, max_meas, data, data_count);
    Gen_bins(min_meas, max_meas, bin_maxes, bin_counts, bin_count);

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, Thread_work, (void*)thread);

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    Print_histo(bin_maxes, bin_counts, bin_count, min_meas);

    free(data);
    free(bin_maxes);
    free(bin_counts);
    free(thread_handles);
    pthread_mutex_destroy(&mutex);

    return 0;
}  
/*---------------------------------------------------------------------*/
void Usage(char prog_name[]) {
    fprintf(stderr, "usage: %s <bin_count> <min_meas> <max_meas> <data_count> <thread_count>\n", prog_name);
    exit(0);
}  

/*---------------------------------------------------------------------*/
void Get_args(char* argv[], int* bin_count_p, float* min_meas_p, float* max_meas_p, int* data_count_p) {
    *bin_count_p = strtol(argv[1], NULL, 10);
    *min_meas_p = strtof(argv[2], NULL);
    *max_meas_p = strtof(argv[3], NULL);
    *data_count_p = strtol(argv[4], NULL, 10);
}  

/*---------------------------------------------------------------------*/
void Gen_data(float min_meas, float max_meas, float data[], int data_count) {
    int i;
    srandom(0);
    for (i = 0; i < data_count; i++)
        data[i] = min_meas + (max_meas - min_meas) * random() / ((double)RAND_MAX);
}  

/*---------------------------------------------------------------------*/
void Gen_bins(float min_meas, float max_meas, float bin_maxes[], int bin_counts[], int bin_count) {
    float bin_width = (max_meas - min_meas) / bin_count;
    int i;

    for (i = 0; i < bin_count; i++) {
        bin_maxes[i] = min_meas + (i + 1) * bin_width;
        bin_counts[i] = 0;
    }
}  

/*---------------------------------------------------------------------*/
int Which_bin(float data, float bin_maxes[], int bin_count, float min_meas) {
    int bottom = 0, top = bin_count - 1, mid;
    float bin_max, bin_min;

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

    fprintf(stderr, "Data = %f doesn't belong to a bin!\n", data);
    exit(-1);
}  

/*---------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------*/
void* Thread_work(void* rank) {
    long my_rank = (long)rank;
    int i, my_first_i, my_last_i, bin;
    int local_data_count = data_count / thread_count;
    my_first_i = my_rank * local_data_count;
    my_last_i = (my_rank == thread_count - 1) ? data_count : my_first_i + local_data_count;

    for (i = my_first_i; i < my_last_i; i++) {
        bin = Which_bin(data[i], bin_maxes, bin_count, min_meas);
        pthread_mutex_lock(&mutex);
        bin_counts[bin]++;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}  