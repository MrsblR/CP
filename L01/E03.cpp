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

void blockMatrixMultiplication(vector<vector<int>>& A, vector<vector<int>>& B, vector<vector<int>>& C, int N, int blockSize) {
    for (int ii = 0; ii < N; ii += blockSize) {
        for (int jj = 0; jj < N; jj += blockSize) {
            for (int kk = 0; kk < N; kk += blockSize) {
                // Multiplicar los bloques
                for (int i = ii; i < min(ii + blockSize, N); i++) {
                    for (int j = jj; j < min(jj + blockSize, N); j++) {
                        for (int k = kk; k < min(kk + blockSize, N); k++) {
                            C[i][j] += A[i][k] * B[k][j];
                        }
                    }
                }
            }
        }
    }
}

int main() {
    vector<int> sizes = {128, 256, 512, 1024};

    int blockSize = 64;

    for (int N : sizes) {
        cout << "Probando multiplicación para tamaño de matriz " << N << "x" << N << ":" << endl;

        vector<vector<int>> A(N, vector<int>(N, 1));
        vector<vector<int>> B(N, vector<int>(N, 2));
        vector<vector<int>> C(N, vector<int>(N, 0));

        auto start = high_resolution_clock::now();
        classicMatrixMultiplication(A, B, C, N);
        auto stop = high_resolution_clock::now();
        auto durationClassic = duration_cast<milliseconds>(stop - start);
        cout << "Duración (clásica): " << durationClassic.count() << " ms" << endl;

        fill(C.begin(), C.end(), vector<int>(N, 0));

        start = high_resolution_clock::now();
        blockMatrixMultiplication(A, B, C, N, blockSize);
        stop = high_resolution_clock::now();
        auto durationBlock = duration_cast<milliseconds>(stop - start);
        cout << "Duración (por bloques): " << durationBlock.count() << " ms" << endl << endl;
    }

    return 0;
}
