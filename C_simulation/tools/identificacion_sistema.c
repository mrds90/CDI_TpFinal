#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define DATA_SIZE 100

#define A_SIZE 3
#define B_SIZE 2

#define NUM0 (0.04976845)
#define NUM1 (0.03505064)

#define DEN0 (1.0)
#define DEN1 (-1.2631799459800208)
#define DEN2 (0.34799904079225535)  

// Datos simulados de entrada y salida
double u[DATA_SIZE]; // Entrada
double y[DATA_SIZE]; // Salida

// Función para generar la señal PRBS
void generate_prbs_signal(double *u, int size) {
    uint16_t lfsr = 0xACE1u; // Estado inicial no nulo
    uint16_t bit;

    for (int i = 0; i < size; i++) {
        // Generar el bit pseudo-aleatorio
        bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
        lfsr = (lfsr >> 1) | (bit << 15);

        // Mapear el valor del PRBS a +1 o -1
        u[i] = (lfsr & 1) ? 1.0 : 2.0;
    }
}

// Función para adquirir la salida del sistema (simulación)
void acquire_output_signal(double *u, double *y, int size) {
    // Aquí se simula la respuesta del sistema
    // Modificar esta función para reflejar el nuevo modelo del sistema
    double a[A_SIZE] = {DEN0, DEN1, DEN2}; // Coeficientes "a"
    double b[B_SIZE] = {NUM0, NUM1};        // Coeficientes "b"

    for (int i = 0; i < size; i++) {
        y[i] = b[0] * u[i] + b[1] * (i > 0 ? u[i - 1] : 0)
               + a[0] * (i > 0 ? y[i - 1] : 0) + a[1] * (i > 1 ? y[i - 2] : 0)
               + a[2] * (i > 2 ? y[i - 3] : 0);
    }
}

// Función para invertir una matriz 5x5 (Gauss-Jordan)
void invert_matrix(double A[5][5], double A_inv[5][5]) {
    int i, j, k;
    double ratio, a;

    // Inicializar A_inv como matriz identidad
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 5; j++) {
            A_inv[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }

    // Aplicar Gauss-Jordan
    for (i = 0; i < 5; i++) {
        a = A[i][i];
        for (j = 0; j < 5; j++) {
            A[i][j] /= a;
            A_inv[i][j] /= a;
        }
        for (k = 0; k < 5; k++) {
            if (k != i) {
                ratio = A[k][i];
                for (j = 0; j < 5; j++) {
                    A[k][j] -= ratio * A[i][j];
                    A_inv[k][j] -= ratio * A_inv[i][j];
                }
            }
        }
    }
}

// Función para resolver el sistema de ecuaciones utilizando cuadrados mínimos
void least_squares(double *u, double *y, int size, double *a, double *b) {
    double X[DATA_SIZE][5]; // Matriz de diseño
    double Y[DATA_SIZE];    // Vector de salida
    double Xt[5][DATA_SIZE]; // Transpuesta de X
    double XtX[5][5];       // Xt * X
    double XtY[5];          // Xt * Y
    double invXtX[5][5];    // Inversa de XtX

    // Llenar la matriz de diseño y el vector de salida
    for (int i = 3; i < size; i++) {
        X[i][0] = y[i - 1];
        X[i][1] = y[i - 2];
        X[i][2] = y[i - 3];
        X[i][3] = u[i];
        X[i][4] = u[i - 1];
        Y[i] = y[i];
    }

    // Calcular Xt
    for (int i = 0; i < 5; i++) {
        for (int j = 3; j < size; j++) {
            Xt[i][j] = X[j][i];
        }
    }

    // Calcular XtX
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            XtX[i][j] = 0;
            for (int k = 3; k < size; k++) {
                XtX[i][j] += Xt[i][k] * X[k][j];
            }
        }
    }

    // Calcular XtY
    for (int i = 0; i < 5; i++) {
        XtY[i] = 0;
        for (int k = 3; k < size; k++) {
            XtY[i] += Xt[i][k] * Y[k];
        }
    }

    // Invertir XtX
    invert_matrix(XtX, invXtX);

    // Resolver para los coeficientes a y b
    double result[5];
    for (int i = 0; i < 5; i++) {
        result[i] = 0;
        for (int j = 0; j < 5; j++) {
            result[i] += invXtX[i][j] * XtY[j];
        }
    }

    a[0] = result[0];
    a[1] = result[1];
    a[2] = result[2];
    b[0] = result[3];
    b[1] = result[4];
}


int main() {
    double a[3], b[2];

    generate_prbs_signal(u, DATA_SIZE);
    acquire_output_signal(u, y, DATA_SIZE);
    least_squares(u, y, DATA_SIZE, a, b);

    printf("Identified system parameters:\n");
    printf("a1: %f, a2: %f, b2: %f\n", a[0], a[1], a[2]);
    printf("b0: %f, b1: %f", b[0], b[1]);

    return 0;
}
