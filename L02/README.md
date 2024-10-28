# EJERCICIO 3.1


## 1. Inicialización de MPI

```c
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
```

Aquí se inicializa el entorno MPI y se configuran las comunicaciones entre procesos. `MPI_Comm_rank` obtiene el identificador del proceso actual, mientras que `MPI_Comm_size` devuelve el número total de procesos que participan en la ejecución.

## 2. Obtención de Argumentos en el Proceso 0

```c
if (my_rank == 0) {
    if (argc != 5) Usage(argv[0]);
    Get_args(argv, &bin_count, &min_meas, &max_meas, &data_count);
}
```

El proceso 0, conocido como proceso raíz, es el encargado de leer los argumentos de la línea de comandos. Esto evita que múltiples procesos intenten acceder a la entrada al mismo tiempo.

## 3. Broadcast de los Argumentos

```c
MPI_Bcast(&bin_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
MPI_Bcast(&min_meas, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
MPI_Bcast(&max_meas, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
MPI_Bcast(&data_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
```

Con `MPI_Bcast`, se distribuyen los valores de los argumentos desde el proceso 0 a todos los demás. Esto asegura que cada proceso tenga la misma configuración inicial.

## 4. Asignación de Memoria para Bins y Contadores

```c
bin_maxes = malloc(bin_count * sizeof(float));
bin_counts = malloc(bin_count * sizeof(int));
for (i = 0; i < bin_count; i++) bin_counts[i] = 0;
```

Se reserva memoria para `bin_maxes` (los límites superiores de cada bin) y `bin_counts` (los contadores). Todos los procesos realizan esta asignación.

## 5. Generación de Datos y Definición de Bins en el Proceso 0

```c
if (my_rank == 0) {
    data = malloc(data_count * sizeof(float));
    Gen_data(min_meas, max_meas, data, data_count);
    Gen_bins(min_meas, max_meas, bin_maxes, bin_counts, bin_count);
}
```

Solo el proceso 0 genera datos aleatorios y define los límites de los bins. `Gen_data` crea los datos en el rango definido, mientras que `Gen_bins` inicializa los límites.

## 6. Distribución de los Límites de los Bins a Todos los Procesos

```c
MPI_Bcast(bin_maxes, bin_count, MPI_FLOAT, 0, MPI_COMM_WORLD);
```

Los límites de los bins se envían a todos los procesos, permitiendo que cada uno trabaje con la misma información.

## 7. División de Datos entre Procesos

```c
int local_data_count = data_count / comm_sz;
float* local_data = malloc(local_data_count * sizeof(float));
MPI_Scatter(data, local_data_count, MPI_FLOAT, local_data, local_data_count, MPI_FLOAT, 0, MPI_COMM_WORLD);
```

`MPI_Scatter` distribuye los datos desde el proceso 0, asegurando que cada proceso reciba una parte igual.

## 8. Conteo de Datos Locales en Cada Bin

```c
int* local_bin_counts = malloc(bin_count * sizeof(int));
for (i = 0; i < bin_count; i++) local_bin_counts[i] = 0;
for (i = 0; i < local_data_count; i++) {
    bin = Which_bin(local_data[i], bin_maxes, bin_count, min_meas);
    local_bin_counts[bin]++;
}
```

Cada proceso cuenta cuántos de sus datos caen en cada bin, guardando los resultados en `local_bin_counts`.

## 9. Reducción de Resultados Locales al Proceso 0

```c
MPI_Reduce(local_bin_counts, bin_counts, bin_count, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
```

Con `MPI_Reduce`, se suman los contadores locales y el resultado se envía al proceso 0, que ahora tiene el total acumulado por bin.

## 10. Impresión del Histograma

```c
if (my_rank == 0) {
    Print_histo(bin_maxes, bin_counts, bin_count, min_meas);
    free(data);
}
```

Solo el proceso 0 imprime el histograma. También se libera la memoria de los arreglos utilizados por este proceso.

## 11. Liberación de Memoria y Finalización de MPI

```c
free(bin_maxes);
free(bin_counts);
free(local_data);
free(local_bin_counts);
MPI_Finalize();
```

Cada proceso libera la memoria que ha reservado. Finalmente, `MPI_Finalize` finaliza el entorno MPI.


# EJERCICIO 3.2


## 1. Inicialización de MPI y Argumentos

```c
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
```

- Se almacenan `argc` y `argv` para que todos los procesos puedan utilizarlos.
- `MPI_Comm_rank` asigna un identificador único (`my_rank`) a cada proceso, y `MPI_Comm_size` guarda el número total de procesos (`comm_sz`). Cada proceso opera de manera independiente.

## 2. Lectura de Argumentos en el Proceso 0 y Broadcast

```c
if (my_rank == 0) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_tosses>\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }
    number_of_tosses = atoll(argv[1]);
}

MPI_Bcast(&number_of_tosses, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);
```

- `number_of_tosses` se almacena solo en el proceso 0 y luego se distribuye a todos los procesos.
- `MPI_Bcast` envía el valor de `number_of_tosses` desde el proceso 0 a todos los demás, asegurando que cada proceso tenga el mismo número de lanzamientos.

## 3. Distribución del Trabajo

```c
long long int local_tosses = number_of_tosses / comm_sz;
```

- Cada proceso almacena su propia parte de los lanzamientos (`local_tosses`).
- Se divide el total de lanzamientos entre todos los procesos, distribuyendo la carga de trabajo de manera uniforme.

## 4. Generación de Números Aleatorios y Cálculo de Puntos

```c
seed = (unsigned int) time(NULL) + my_rank;

for (toss = 0; toss < local_tosses; toss++) {
    x = (double) rand_r(&seed) / RAND_MAX * 2.0 - 1.0;
    y = (double) rand_r(&seed) / RAND_MAX * 2.0 - 1.0;
    distance_squared = x * x + y * y;
    if (distance_squared <= 1.0) {
        local_number_in_circle++;
    }
}
```

- Cada proceso almacena sus propias coordenadas (`x`, `y`) y un contador (`local_number_in_circle`) para los lanzamientos que caen dentro del círculo.
- Cada proceso genera números aleatorios y calcula de forma independiente si caen dentro del círculo, optimizando así el tiempo total de ejecución.

## 5. Reducción de Resultados Parciales

```c
MPI_Reduce(&local_number_in_circle, &number_in_circle, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);
```

- `local_number_in_circle` es la variable local de cada proceso. `number_in_circle` se almacena solo en el proceso 0, donde se guardará la suma total.
- `MPI_Reduce` combina los resultados parciales de cada proceso y almacena el total en el proceso 0, lo que le permite calcular el valor de π.

## 6. Cálculo de la Estimación de π y Liberación de Recursos

```c
if (my_rank == 0) {
    double pi_estimate = 4 * ((double) number_in_circle) / ((double) number_of_tosses);
    printf("Estimated value of pi: %f\n", pi_estimate);
}

MPI_Finalize();
```

- La estimación de π (`pi_estimate`) se calcula y almacena solo en el proceso 0.
- El proceso 0 realiza el cálculo final utilizando los resultados de todos los procesos. `MPI_Finalize` cierra el entorno MPI, asegurando que todos los procesos terminen de manera coordinada.



# EJERCICIO 3.3


## 1. Declaración de Variables y Configuración Inicial

```c
int comm_sz; 
int my_rank; 
int local_val, global_sum;

MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
```

  - `comm_sz` y `my_rank` almacenan el número total de procesos y el rango del proceso actual, respectivamente.
  - `MPI_Init` inicia el entorno MPI, permitiendo que todos los procesos comiencen a ejecutarse en paralelo.
  - Cada proceso tiene su propia memoria local para estas variables.

## 2. Asignación del Valor Local

```c
local_val = my_rank;
```

- Cada proceso asigna a `local_val` un valor basado en su rango (`my_rank`).
- Cada proceso tiene su propia copia de `local_val`.
- Todos los procesos realizan esta asignación simultáneamente.

## 3. Reducción de Datos en Forma de Árbol

```c
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
```

- Implementa la reducción de datos en forma de árbol, dividiendo el trabajo entre procesos jerárquicamente.

  - Los procesos reciben y envían datos utilizando `MPI_Recv` y `MPI_Send`.
  - A medida que `step` aumenta, disminuye el número de procesos activos, formando un árbol binario.
  - La sincronización se da implícitamente, ya que cada proceso espera recibir datos antes de continuar.

## 4. Resultado 

```c
if (my_rank == 0) {
    global_sum = local_val;
    printf("The global sum is %d\n", global_sum);
}
```

- Solo el proceso 0 guarda y muestra el valor total de `global_sum`.
- El valor se almacena únicamente en el proceso 0, mientras que los demás procesos completan su tarea y esperan la finalización.
- Solo el proceso 0 imprime el resultado final.

## 5. Cierre de MPI

```c
MPI_Finalize();
return 0;
```

- `MPI_Finalize` libera los recursos de MPI y termina el entorno paralelo.
- Todos los procesos deben llegar a esta línea antes de que se complete el entorno de MPI, actuando como una barrera de sincronización.

## 6. Para Versión General para `comm_sz` No Potencia de Dos

```c
int new_comm_sz = pow(2, (int)log2(comm_sz));
if (my_rank >= new_comm_sz) {
    MPI_Send(&local_val, 1, MPI_INT, my_rank - new_comm_sz, 0, MPI_COMM_WORLD);
} else if (my_rank < comm_sz - new_comm_sz) {
    int received_val;
    MPI_Recv(&received_val, 1, MPI_INT, my_rank + new_comm_sz, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    local_val += received_val;
}
```

- Esta lógica maneja casos en los que el número de procesos (`comm_sz`) no es una potencia de dos.
- `new_comm_sz` calcula la potencia de dos más cercana que es menor o igual a `comm_sz`.
- Los procesos que no están involucrados en la potencia de dos se comunican con los procesos activos, asegurando que todos los datos se integren en la suma global.



# EJERCICIO 3.5

## 1. Definición de la Matriz y el Vector

```c
n = 4;
int local_n = n / comm_sz;
```

- `n` define el tamaño de la matriz (n x n) y `local_n` especifica cuántas columnas de la matriz manejará cada proceso.
- `local_n` asegura que cada proceso reciba un bloque igual de la matriz, facilitando la distribución de trabajo.

## 2. Asignación de Memoria para los Bloques Locales

```c
local_matrix = (double *)malloc(local_n * n * sizeof(double));
vector = (double *)malloc(n * sizeof(double));
local_result = (double *)malloc(local_n * sizeof(double));
if (rank == 0) result = (double *)malloc(n * sizeof(double));
```

- Cada proceso reserva espacio para su bloque de la matriz (`local_matrix`), el vector (`vector`), y su resultado parcial (`local_result`). Solo el proceso 0 reserva espacio para `result`.
- Cada proceso tiene su propia porción de memoria, permitiendo que trabajen de forma independiente.

## 3. Lectura de la Matriz y el Vector en el Proceso 0

```c
if (rank == 0) {
    matrix = (double *)malloc(n * n * sizeof(double));
    for (int i = 0; i < n; i++) {
        vector[i] = 1.0;
        for (int j = 0; j < n; j++) {
            matrix[i * n + j] = i * n + j;
        }
    }
}
```

- El proceso 0 reserva memoria para la matriz completa (`matrix`) y el vector, inicializándolos.
- Solo el proceso 0 realiza la lectura e inicialización, evitando duplicación de datos.

## 4. Distribución de la Matriz a los Procesos

```c
for (int p = 0; p < comm_sz; p++) {
    if (p == 0) {
        for (int i = 0; i < local_n * n; i++) {
            local_matrix[i] = matrix[i];
        }
    } else {
        MPI_Send(matrix + p * local_n * n, local_n * n, MPI_DOUBLE, p, 0, MPI_COMM_WORLD);
    }
}
free(matrix);
```

- El proceso 0 envía bloques de `local_n` columnas a cada proceso. Después de la distribución, libera la memoria de la matriz completa.
- Los envíos son concurrentes, permitiendo que cada proceso trabaje solo con su porción de la matriz.

## 5. Recepción de la Matriz en Otros Procesos

```c
} else {
    MPI_Recv(local_matrix, local_n * n, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}
```

- Cada proceso (excepto el 0) recibe su porción de la matriz y la almacena en `local_matrix`.
- Todos los procesos comienzan a trabajar en su cálculo local tan pronto como reciben sus datos.

## 6. Broadcast del Vector a Todos los Procesos

```c
MPI_Bcast(vector, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
```

- El vector se copia en cada proceso.
- Todos los procesos reciben el vector al mismo tiempo, asegurando que tengan la misma versión antes de iniciar los cálculos.

## 7. Multiplicación Local Matriz-Vector

```c
for (int i = 0; i < local_n; i++) {
    local_result[i] = 0.0;
    for (int j = 0; j < n; j++) {
        local_result[i] += local_matrix[i * n + j] * vector[j];
    }
}
```

- Cada proceso realiza la multiplicación usando su `local_matrix` y el `vector`, almacenando el resultado parcial en `local_result`.
- Cada proceso ejecuta su bucle de forma independiente y simultánea, aprovechando el paralelismo de datos.

## 8. Reducción de los Resultados Parciales

```c
int recvcounts[comm_sz];
for (int i = 0; i < comm_sz; i++) {
    recvcounts[i] = local_n;
}

MPI_Reduce_scatter(local_result, result, recvcounts, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
```

- `recvcounts` define cuántos elementos recibe cada proceso después de la reducción. `MPI_Reduce_scatter` combina los resultados parciales de todos los procesos y distribuye los resultados a `result`.
- La operación permite la suma de los elementos correspondientes y su distribución a todos los procesos.

## 9. Impresión del Resultado Final

```c
if (rank == 0) {
    printf("Result vector:\n");
    for (int i = 0; i < n; i++) {
        printf("%f ", result[i]);
    }
    printf("\n");
}
```

- Solo el proceso 0 imprime el resultado final.
- El resultado final se almacena en el proceso 0, aunque todos contribuyeron a generarlo de manera paralela.

## 10. Liberación de Memoria y Finalización de MPI

```c
free(local_matrix);
free(vector);
free(local_result);
if (rank == 0) free(result);

MPI_Finalize();
```

- Se libera la memoria dinámica utilizada por cada proceso.
- `MPI_Finalize` sincroniza a los procesos antes de terminar el programa, asegurando que todos finalicen correctamente.



# EJERCICIO 3.8

## 1. Inicialización de MPI

```c
MPI_Init(&argc, &argv);
```

- Se inicializa el entorno MPI, estableciendo el contexto para la ejecución paralela.
- No hay asignación de memoria, pero todos los procesos se sincronizan y se preparan para comenzar la ejecución.

## 2. Obtención del Rango y Tamaño de Procesos

```c
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
```

- Se obtiene el identificador único del proceso en `rank` y el número total de procesos en `comm_sz`.
- Esta información se utiliza para dividir el trabajo entre todos los procesos disponibles.

## 3. Entrada de Usuario y Distribución

```c
if (rank == 0) { ... }
MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
```

- Solo el proceso 0 gestiona la entrada del usuario para `n`, que se distribuye a todos los procesos.
- Esto asegura que solo un proceso maneje la entrada, evitando múltiples solicitudes y sincronizando el tamaño del trabajo.

## 4. Cálculo del Tamaño Local

```c
local_n = n / comm_sz;
```

- Se calcula el número de elementos que cada proceso manejará.
- Establece la carga de trabajo para garantizar una distribución equitativa de las tareas.

## 5. Asignación de Memoria para Datos Locales

```c
local_data = (int *)malloc(local_n * sizeof(int));
```

- Se asigna espacio en memoria para el array local de cada proceso.
- Cada proceso tiene su propia copia de `local_data`, lo que permite el trabajo independiente y paralelo.

## 6. Generación de Números Aleatorios

```c
srand(rank + 1); 
local_data[i] = rand() % 100;
```

- Inicializa el generador de números aleatorios y llena `local_data` con números aleatorios.
- Cada proceso genera su propia lista de números de forma independiente, evitando conflictos.

## 7. Ordenación Local

```c
qsort(local_data, local_n, sizeof(int), compare);
```

- Se ordena el array local en cada proceso.
- La ordenación se realiza de manera local, permitiendo que cada proceso trabaje en su parte de los datos.

## 8. Recolección de Datos

```c
MPI_Gather(local_data, local_n, MPI_INT, recv_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);
```

- Se recolectan los arrays ordenados de cada proceso y se almacenan en `recv_data` en el proceso 0.
- Todos los procesos envían sus datos a proceso 0, lo que se realiza de manera eficiente al mismo tiempo.

## 9. Mezcla de Resultados

```c
int step = 1; 
while (step < comm_sz) { ... }
```

- Se determina el tamaño de las sublistas a mezclar en cada iteración.
- La mezcla se realiza en pasos, permitiendo que múltiples procesos trabajen en la combinación de datos de manera paralela.

## 10. Impresión del Resultado Final

```c
if (rank == 0) {
    printf("Sorted array:\n");
    for (int i = 0; i < n; i++) {
        printf("%f ", recv_data[i]);
    }
    printf("\n");
}
```

- Solo el proceso 0 imprime el resultado final.
- Limitar la impresión al proceso 0 evita interferencias y confusiones en la salida.

## 11. Liberación de Memoria

```c
free(local_data); 
free(recv_data);
```

- Se libera la memoria asignada a los arrays locales y al resultado.
- Cada proceso libera su propia memoria de forma independiente, asegurando que no haya fugas de memoria al finalizar.

