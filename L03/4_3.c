#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// Variables globales
int thread_count;
int flag = 0;                // Busy-waiting flag
pthread_mutex_t mutex;       // Mutex
sem_t sem;                   // Semaphore
double a, b;                 // Límites de integración
int n;                       // Número de trapezoides
double h;                    // Ancho de cada trapezoide
double total_sum = 0.0;      // Resultado final compartido

// Función a integrar
double f(double x) {
    return x * x; // Por ejemplo, f(x) = x^2
}

// Cálculo parcial por cada hilo
void* Thread_sum(void* rank) {
    long my_rank = (long) rank;
    double local_a = a + my_rank * n / thread_count * h;
    double local_b = local_a + n / thread_count * h;
    double local_sum = (f(local_a) + f(local_b)) / 2.0;
    int local_n = n / thread_count;

    for (int i = 1; i < local_n; i++) {
        double x = local_a + i * h;
        local_sum += f(x);
    }
    local_sum *= h;

    // **Busy-Waiting**
    while (flag != my_rank);
    total_sum += local_sum;
    flag = (flag + 1) % thread_count;

    // **Mutex**
    pthread_mutex_lock(&mutex);
    total_sum += local_sum;
    pthread_mutex_unlock(&mutex);

    // **Semaphores**
    sem_wait(&sem);
    total_sum += local_sum;
    sem_post(&sem);

    return NULL;
}

int main(int argc, char* argv[]) {
    long thread;
    pthread_t* thread_handles;

    // Inicialización automática
    thread_count = 4;  // Número de hilos
    a = 0.0;           // Límite inferior de integración
    b = 1.0;           // Límite superior de integración
    n = 1000;          // Número de trapezoides

    printf("Usando los siguientes parámetros:\n");
    printf("Número de hilos: %d\n", thread_count);
    printf("Límite inferior (a): %f\n", a);
    printf("Límite superior (b): %f\n", b);
    printf("Número de trapezoides (n): %d\n", n);

    thread_handles = malloc(thread_count * sizeof(pthread_t));
    pthread_mutex_init(&mutex, NULL);
    sem_init(&sem, 0, 1);
    h = (b - a) / n; // Calcular el ancho de cada trapezoide

    // Crear hilos
    for (thread = 0; thread < thread_count; thread++) {
        pthread_create(&thread_handles[thread], NULL, Thread_sum, (void*) thread);
    }

    // Esperar a los hilos
    for (thread = 0; thread < thread_count; thread++) {
        pthread_join(thread_handles[thread], NULL);
    }

    // Mostrar el resultado
    printf("Con n = %d trapezoides, la estimación de la integral de %f a %f es %.15e\n", n, a, b, total_sum);

    // Liberar recursos
    free(thread_handles);
    pthread_mutex_destroy(&mutex);
    sem_destroy(&sem);

    return 0;
}
