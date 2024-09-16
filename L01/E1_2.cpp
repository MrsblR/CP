#include <chrono>
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <math.h>

using namespace std;

const int MAX = 500;
const int NUM_RUNS = 10; // Número de ejecuciones para cada bucle
double A[MAX][MAX], x[MAX], y[MAX];

void init() {
    for(int i = 0; i < MAX ; i++) {
        y[i] = 0;
        x[i] = 1 + rand() % 9;
        for(int j = 0 ; j < MAX ; j++) 
            A[i][j] = 1 + rand() % 100;
    }
}

double calcTime(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end) {
    return chrono::duration_cast<chrono::microseconds>(end - start).count() / 1e6;
}

double nestedLoop1() {
    init();
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < MAX; i++)
        for (int j = 0; j < MAX; j++)
            y[i] += A[i][j] * x[j];
    auto end = chrono::high_resolution_clock::now();
    return calcTime(start, end);
}

double nestedLoop2() {
    init();
    auto start = chrono::high_resolution_clock::now();
    for (int j = 0; j < MAX; j++)
        for (int i = 0; i < MAX; i++)
            y[i] += A[i][j] * x[j];
    auto end = chrono::high_resolution_clock::now();
    return calcTime(start, end);
}

void runComparison() {
    vector<double> times1, times2;
    
    for (int i = 0; i < NUM_RUNS; i++) {
        times1.push_back(nestedLoop1());
        times2.push_back(nestedLoop2());
    }

    auto calcStats = [](vector<double>& times) {
        double sum = accumulate(times.begin(), times.end(), 0.0);
        double mean = sum / times.size();
        double sq_sum = inner_product(times.begin(), times.end(), times.begin(), 0.0);
        double stdev = sqrt(sq_sum / times.size() - mean * mean);
        return make_pair(mean, stdev);
    };

    auto [mean1, stdev1] = calcStats(times1);
    auto [mean2, stdev2] = calcStats(times2);

    cout << fixed << setprecision(6);
    cout << "Bucle Anidado 1 (por filas):" << endl;
    cout << "  Tiempo promedio: " << mean1 << " segundos" << endl;
    cout << "  Desviación estándar: " << stdev1 << " segundos" << endl;
    cout << "Bucle Anidado 2 (por columnas):" << endl;
    cout << "  Tiempo promedio: " << mean2 << " segundos" << endl;
    cout << "  Desviación estándar: " << stdev2 << " segundos" << endl;

    double speedup = mean2 / mean1;
    cout << "Aceleración (Bucle 1 vs Bucle 2): " << speedup << "x" << endl;
}

int main() {    
    srand(time(0));
    runComparison();
    return 0;
}