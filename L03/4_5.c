#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_TAREAS 10  

// Estructura para una tarea (nodo de la lista enlazada)
struct Tarea {
    int dato;
    struct Tarea* siguiente;
};

// Estructura para la cola de tareas
struct ColaDeTareas {
    struct Tarea* cabeza;  // Cabeza de la cola
    struct Tarea* cola;    // Cola de la cola
    int contador_tareas;   // Contador de tareas
    pthread_mutex_t mutex;   // Mutex para proteger la cola
    pthread_cond_t variable_condicion; // Variable de condición para esperar notificaciones
};

// Variables globales
struct ColaDeTareas cola_tareas;   // Cola de tareas global
int no_hay_mas_tareas = 0;         // Indicador de que ya no hay más tareas

// Función para inicializar la cola de tareas
void inicializar_cola_tareas() {
    cola_tareas.cabeza = NULL;
    cola_tareas.cola = NULL;
    cola_tareas.contador_tareas = 0;
    pthread_mutex_init(&cola_tareas.mutex, NULL);
    pthread_cond_init(&cola_tareas.variable_condicion, NULL);
}

// Función para agregar una tarea a la cola
void agregar_tarea(int dato) {
    struct Tarea* nueva_tarea = (struct Tarea*) malloc(sizeof(struct Tarea));
    nueva_tarea->dato = dato;
    nueva_tarea->siguiente = NULL;

    pthread_mutex_lock(&cola_tareas.mutex);  // Bloquear el mutex para acceder a la cola

    if (cola_tareas.cola == NULL) {
        cola_tareas.cabeza = nueva_tarea;
        cola_tareas.cola = nueva_tarea;
    } else {
        cola_tareas.cola->siguiente = nueva_tarea;
        cola_tareas.cola = nueva_tarea;
    }
    cola_tareas.contador_tareas++;  // Incrementar el contador de tareas

    pthread_cond_signal(&cola_tareas.variable_condicion);  // Señalar que hay una tarea disponible
    pthread_mutex_unlock(&cola_tareas.mutex);  // Desbloquear el mutex
}

// Función para eliminar una tarea de la cola
struct Tarea* quitar_tarea() {
    struct Tarea* tarea;

    pthread_mutex_lock(&cola_tareas.mutex);  // Bloquear el mutex para acceder a la cola

    while (cola_tareas.contador_tareas == 0 && !no_hay_mas_tareas) {
        pthread_cond_wait(&cola_tareas.variable_condicion, &cola_tareas.mutex);  // Esperar hasta que haya una tarea o se indique que no hay más tareas
    }

    if (cola_tareas.contador_tareas > 0) {
        tarea = cola_tareas.cabeza;
        cola_tareas.cabeza = cola_tareas.cabeza->siguiente;
        if (cola_tareas.cabeza == NULL) {
            cola_tareas.cola = NULL;
        }
        cola_tareas.contador_tareas--;  // Decrementar el contador de tareas
    } else {
        tarea = NULL;
    }

    pthread_mutex_unlock(&cola_tareas.mutex);  // Desbloquear el mutex
    return tarea;
}

// Función del hilo para procesar tareas
void* hilo_trabajador(void* arg) {
    while (1) {
        struct Tarea* tarea = quitar_tarea();  // Obtener una tarea de la cola

        if (tarea != NULL) {
            // Procesar la tarea
            printf("Hilo %ld procesando tarea: %d\n", (long) arg, tarea->dato);
            free(tarea);  // Liberar la memoria de la tarea
        } else if (no_hay_mas_tareas) {
            break;  // Salir del ciclo si no hay más tareas
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    int cantidad_hilos = strtol(argv[1], NULL, 10);  // Leer el número de hilos desde los argumentos
    pthread_t* manejadores_hilos = (pthread_t*) malloc(cantidad_hilos * sizeof(pthread_t));  // Crear los manejadores de los hilos
    long hilo;

    inicializar_cola_tareas();  // Inicializar la cola de tareas

    // Crear los hilos trabajadores
    for (hilo = 0; hilo < cantidad_hilos; hilo++) {
        pthread_create(&manejadores_hilos[hilo], NULL, hilo_trabajador, (void*) hilo);  // Crear un hilo
    }

    // Generar las tareas
    for (int i = 0; i < NUM_TAREAS; i++) {
        agregar_tarea(i);  // Agregar la tarea a la cola
    }

    // Señalar que no hay más tareas y despertar a todos los hilos
    no_hay_mas_tareas = 1;
    pthread_cond_broadcast(&cola_tareas.variable_condicion);  // Despertar todos los hilos

    // Esperar a que los hilos trabajadores terminen
    for (hilo = 0; hilo < cantidad_hilos; hilo++) {
        pthread_join(manejadores_hilos[hilo], NULL);  // Esperar a que cada hilo termine
    }

    // Destruir el mutex y la variable de condición
    pthread_mutex_destroy(&cola_tareas.mutex);
    pthread_cond_destroy(&cola_tareas.variable_condicion);
    free(manejadores_hilos);  // Liberar la memoria de los manejadores de hilos

    return 0;
}