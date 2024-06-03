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

typedef struct {
    double input;
    double output;
    double input_buffer[B_SIZE];
    double output_buffer[A_SIZE];
} real_world_t;

static real_world_t real_world = {
    .input = 0,
    .output = 0,
    .input_buffer = {[0 ... B_SIZE - 1] = 0},
    .output_buffer = {[0 ... A_SIZE - 2] = 0},
};

static double RecurrenceFunction(double input) {
    // Desplazar valores en el buffer de entrada
    for (int i = B_SIZE - 1; i > 0; --i) {
        real_world.input_buffer[i] = real_world.input_buffer[i - 1];
    }
    real_world.input_buffer[0] = input;

    // Calcular la parte del numerador
    double output = 0;
    output += (NUM0 * real_world.input_buffer[0]) ;
    output += (NUM1 * real_world.input_buffer[1]) ;

    // Calcular la parte del denominador
    output -= (DEN1 * real_world.output_buffer[0]);
    output -= (DEN2 * real_world.output_buffer[1]);

    // Desplazar valores en el buffer de salida
    for (int i = A_SIZE - 2; i > 0; --i) {
        real_world.output_buffer[i] = real_world.output_buffer[i - 1];
    }
    real_world.output_buffer[0] = output;

    return output;
}


// Datos simulados de entrada y salida
static double u_sys[100] = {0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0};
// double u[DATA_SIZE];
static double y_sys[DATA_SIZE]; // Salida

// Función para multiplicar dos números Q15
double q15_mult(double a, double b) {
    return (a * b);
}

// Función para dividir dos números Q15
double q15_div(double a, double b) {
    // Asegurarse de que no hay división por cero

    return (double)(a / b);
}

// Función para generar la señal PRBS
void generate_prbs_signal(double *u, int size) {
    uint32_t lfsr = (1.0); // Estado inicial no nulo
    uint32_t bit;

    for (int i = 0; i < size; i++) {
        // Generar el bit pseudo-aleatorio
        bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
        lfsr = (lfsr >> 1) | (bit);

        // Mapear el valor del PRBS a +1 o -1
        u[i] = (lfsr & 1) ? 1.0 : 0;
    }
}

// Función para adquirir la salida del sistema (simulación)
void AcquireOutputSignal(double *u, double *y, int size) {
    for (int i = 0; i < size; i++) {
        y[i] =  RecurrenceFunction(u[i]);
    }
}

// Función para invertir una matriz 5x5 (Gauss-Jordan)
void InvertMatrix(double A[5][5], double A_inv[5][5]) {
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
            A[i][j] = q15_div(A[i][j], a);
            A_inv[i][j] = q15_div(A_inv[i][j], a);
        }
        for (k = 0; k < 5; k++) {
            if (k != i) {
                ratio = A[k][i];
                for (j = 0; j < 5; j++) {
                    A[k][j] -= q15_mult(ratio, A[i][j]);
                    A_inv[k][j] -= q15_mult(ratio, A_inv[i][j]);
                }
            }
        }
    }
}

// Función para resolver el sistema de ecuaciones utilizando cuadrados mínimos
void LeastSquares(double *u, double *y, int size, double *a, double *b) {
    double Phi[DATA_SIZE][5] = {
        [0 ... DATA_SIZE -1 ] = {
            [0 ... 4] = 0
        }
    }; // Matriz de diseño
    double Y[DATA_SIZE] = {[0 ... DATA_SIZE -1 ] = 0};    // Vector de salida
    double PhiT[5][DATA_SIZE]; // Transpuesta de Phi
    double PhiTPhi[5][5];       // PhiT * Phi
    double XtY[5];          // PhiT * Y
    double invPhiTPhi[5][5];    // Inversa de PhiTPhi
    double invPhiTPhiPhiT[5][DATA_SIZE] = {
        [0 ... 4] = {
            [0 ... DATA_SIZE -1 ] = 0
        }
    };

    // Llenar la matriz de diseño y el vector de salida
    for (int i = 2; i < (size); i++) {
        Phi[i-2][0] = y[i - 1];
        Phi[i-2][1] = y[i - 2];
        Phi[i-2][2] = u[i];
        Phi[i-2][3] = u[i-1];
        Phi[i-2][4] = u[i-2];
        Y[i - 2] = y[i];
    }

    // Calcular PhiT
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < size; j++) {
            PhiT[i][j] = Phi[j][i];
        }
    }

    // Calcular PhiTPhi
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            PhiTPhi[i][j] = 0;
            for (int k = 0; k < size; k++) {
                PhiTPhi[i][j] += q15_mult(PhiT[i][k], Phi[k][j]);
            }
        }
    }

    InvertMatrix(PhiTPhi, invPhiTPhi);

    // Calcular invPhiTPhiPhiT
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < size; j++) {
            invPhiTPhiPhiT[i][j] = 0;
            for (int k = 0; k < 5; k++) {
                invPhiTPhiPhiT[i][j] += q15_mult(invPhiTPhi[i][k], PhiT[k][j]);
            }
        }
    }

    // Calcular XtY
    for (int i = 0; i < 5; i++) {
        XtY[i] = 0;
        for (int j = 0; j < size; j++) {
            XtY[i] += q15_mult(invPhiTPhiPhiT[i][j], Y[j]);
        }
    }


    a[0] = 1;
    a[1] = XtY[0];
    a[2] = XtY[1];
    b[0] = XtY[2];
    b[1] = XtY[3];
}

int main() {
    double a[3], b[2];

    // generate_prbs_signal(u, DATA_SIZE);
    AcquireOutputSignal(u_sys, y_sys, DATA_SIZE);
    LeastSquares(u_sys, y_sys, DATA_SIZE, a, b);

    printf("System parameters:\n");
    printf("a0: %f, a1: %f, a2: %f\n", (double)DEN0, (double)DEN1, (double)DEN2);
    printf("b0: %f, b1: %f\n\n", (double)NUM0, (double)NUM1);
    printf("Identified system parameters:\n");
    printf("DEN0 = %f\nDEN1 = %f\nDEN2 = %f\n", (double)a[0], (double)a[1], (double)a[2]);
    printf("NUM0 = %f\nNUM1 = %f\n\n", (double)b[0], (double)b[1]);
    return 0;
}
