#include <chrono>
#include <iostream>
#include <sys/time.h>
#include <ctime>
#include <cstdlib>

using namespace std;

const int MAX = 500;
double A[MAX][MAX], x[MAX], y[MAX];

void init() {
    for(int i = 0; i < MAX ; i++) {
        y[i] = 0;
        x[i] = 1 + rand() % 9; // Genera números entre 1 y 9
        for(int j = 0 ; j < MAX ; j++) 
            A[i][j] = 1 + rand() % 100; // Genera números entre 1 y 100
    }
}

double calcTime(clock_t t0, clock_t t1) {
    return static_cast<double>(t1 - t0) / CLOCKS_PER_SEC;
}

void nestedLoop1() {
    clock_t t0, t1;
    init();
    t0 = clock();
    for (int i = 0; i < MAX; i++)
        for (int j = 0; j < MAX; j++)
            y[i] += A[i][j] * x[j];
    t1 = clock();
    cout << "Tiempo de Ejecución del Bucle Anidado 1: " << calcTime(t0, t1) << " segundos" << endl;
}

void nestedLoop2() {
    clock_t t0, t1;
    init();
    t0 = clock();
    for (int j = 0; j < MAX; j++)
        for (int i = 0; i < MAX; i++)
            y[i] += A[i][j] * x[j];
    t1 = clock();
    cout << "Tiempo de Ejecución del Bucle Anidado 2: " << calcTime(t0, t1) << " segundos" << endl;
}

int main() {    
    srand(time(0)); // Semilla para números aleatorios
    nestedLoop1();
    nestedLoop2();
    return 0;
}