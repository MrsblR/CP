#include <chrono>
#include <iostream>
#include <sys/time.h>
#include <ctime>

using namespace std;

const int MAX = 500;
double A[MAX][MAX], x[MAX], y[MAX];

void init() {
	for(int i = 0; i < MAX ; i++) {
		y[i] = 0;
		x[i] = 1+rand()%(10-1);
		for(int j = 0 ; j < MAX ; j++) 
            A[i][j] = 1+rand()%(101-1);
	}
}

double calcTime(unsigned t0 , unsigned t1) {
	return (double(t1-t0)/CLOCKS_PER_SEC);
}

void nestedLoop1(unsigned t0 , unsigned t1) {
    init();
    t0 = clock();
    for (int i = 0; i < MAX; i++)
		for (int j = 0; j < MAX; j++)
			y[i] += A[i][j] * x[j];
    t1 = clock();
    cout << "Tiempo de Ejecucion del Bucle Anidado 1: " << calcTime(t0 , t1) << endl;
}

void nestedLoop2(unsigned t0 , unsigned t1) {
    init();
    t0 = clock();
    for (int j = 0; j < MAX; j++)
		for (int i = 0; i < MAX; i++)
			y[i] += A[i][j] * x[j];
    t1 = clock();
    cout << "Tiempo de Ejecucion del Bucle Anidado 2: " << calcTime(t0 , t1) << endl;
}


int main() {	
	unsigned t0, t1;
    nestedLoop1(t0 , t1);
    nestedLoop2(t0 , t1);
	return 0;
}