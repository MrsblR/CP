#include <iostream>
#include <vector>
#include <chrono>

using namespace std;
using namespace std::chrono;

void classicMatrixMultiplication(vector<vector<int>>& A, vector<vector<int>>& B, vector<vector<int>>& C, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main() {
    vector<int> sizes = {128, 256, 512, 1024};

    for (int N : sizes) {
        vector<vector<int>> A(N, vector<int>(N, 1));
        vector<vector<int>> B(N, vector<int>(N, 2));
        vector<vector<int>> C(N, vector<int>(N, 0));

        cout << "Multiplicacion clasica para tamanio de matriz " << N << "x" << N << ":" << endl;

        auto start = high_resolution_clock::now();
        classicMatrixMultiplication(A, B, C, N);
        auto stop = high_resolution_clock::now();

        auto duration = duration_cast<milliseconds>(stop - start);
        cout << "Duracion: " << duration.count() << " ms" << endl << endl;
    }

    return 0;
}
