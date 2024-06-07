#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define DATA_SIZE 100

#define A_SIZE 3
#define B_SIZE 2

#define Q15_SCALE(x)  (int32_t)((x) * (1 << 15))

#define NUM0 Q15_SCALE(0.04976845)
#define NUM1 Q15_SCALE(0.03505064)

// Coeficientes del denominador en Q15
#define DEN0 Q15_SCALE(1.0)
#define DEN1 Q15_SCALE(-1.2631799459800208)
#define DEN2 Q15_SCALE(0.34799904079225535)

#define MUL_ELEMENTS(a, b) ((a)*(b))
typedef struct {
    int32_t input;
    int32_t output;
    int32_t input_buffer[B_SIZE];
    int32_t output_buffer[A_SIZE];
} real_world_t;

static real_world_t real_world = {
    .input = 0,
    .output = 0,
    .input_buffer = {[0 ... B_SIZE - 1] = 0},
    .output_buffer = {[0 ... A_SIZE - 2] = 0},
};

static int32_t RecurrenceFunction(int32_t input) {
    // Desplazar valores en el buffer de entrada
    for (int i = B_SIZE - 1; i > 0; --i) {
        real_world.input_buffer[i] = real_world.input_buffer[i - 1];
    }
    real_world.input_buffer[0] = input;

    // Calcular la parte del numerador
    int32_t output = 0;
    output += (NUM0 * real_world.input_buffer[0]) >> 15;
    output += (NUM1 * real_world.input_buffer[1]) >> 15;

    // Calcular la parte del denominador
    output -= (DEN1 * real_world.output_buffer[0]) >> 15;
    output -= (DEN2 * real_world.output_buffer[1]) >> 15;

    // Desplazar valores en el buffer de salida
    for (int i = A_SIZE - 2; i > 0; --i) {
        real_world.output_buffer[i] = real_world.output_buffer[i - 1];
    }
    real_world.output_buffer[0] = output;

    return output;
}


// static float RecurrenceFunction(float input) {
//     // Desplazar valores en el buffer de entrada
//     for (int i = B_SIZE - 1; i > 0; --i) {
//         real_world.input_buffer[i] = real_world.input_buffer[i - 1];
//     }
//     real_world.input_buffer[0] = input;

//     // Calcular la parte del numerador
//     float output = 0;
//     output += (NUM0 * real_world.input_buffer[0]) ;
//     output += (NUM1 * real_world.input_buffer[1]) ;

//     // Calcular la parte del denominador
//     output -= (DEN1 * real_world.output_buffer[0]);
//     output -= (DEN2 * real_world.output_buffer[1]);

//     // Desplazar valores en el buffer de salida
//     for (int i = A_SIZE - 2; i > 0; --i) {
//         real_world.output_buffer[i] = real_world.output_buffer[i - 1];
//     }
//     real_world.output_buffer[0] = output;

//     return output;
// }


// Datos simulados de entrada y salida
// static float u_sys[100] = {[0 ... 99] = 0};
static float u_sys[100] = {0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0};
// float u[DATA_SIZE];
static float y_sys[DATA_SIZE]; // Salida

// Función para multiplicar dos números Q15


// Función para dividir dos números Q15
float q15_div(float a, float b) {
    // Asegurarse de que no hay división por cero

    return (float)(a / b);
}

// Función para generar la señal PRBS
void generate_prbs_signal(float *u, int size) {
    uint16_t lfsr = 0xACE1u; // Estado inicial no nulo
    uint32_t bit;

    for (int i = 0; i < size; i++) {
        // Generar el bit pseudo-aleatorio
        bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
        lfsr = (lfsr >> 1) | (bit);

        // Mapear el valor del PRBS a +1 o -1
        u[i] = (lfsr & 1) ? 1 : 0;
    }
}

// Función para adquirir la salida del sistema (simulación)
void AcquireOutputSignal(float *u, float *y, int size) {
    for (int i = 0; i < size; i++) {
        y[i] =  (float)RecurrenceFunction(Q15_SCALE(u[i]*1000)/3300) * 3300 / (1<<15) /1000;
    }
}

// Función para invertir una matriz 5x5 (Gauss-Jordan)
void InvertMatrix(float A[5][5], float A_inv[5][5]) {
    int i, j, k;
    float ratio, a;

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
                    A[k][j] -= MUL_ELEMENTS(ratio, A[i][j]);
                    A_inv[k][j] -= MUL_ELEMENTS(ratio, A_inv[i][j]);
                }
            }
        }
    }
}

// Función para resolver el sistema de ecuaciones utilizando cuadrados mínimos
void LeastSquares(float *u, float *y, int size, float *a, float *b) {
    float Phi[DATA_SIZE][5] = {
        [0 ... DATA_SIZE -1 ] = {
            [0 ... 4] = 0
        }
    }; // Matriz de diseño
    float Y[DATA_SIZE] = {[0 ... DATA_SIZE -1 ] = 0};    // Vector de salida
    float PhiTPhi[5][5];       // PhiT * Phi
    float XtY[5];          // PhiT * Y
    float invPhiTPhi[5][5];    // Inversa de PhiTPhi
    float invPhiTPhiPhiT[5][DATA_SIZE] = {
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

    // Calcular PhiTPhi
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            PhiTPhi[i][j] = 0;
            for (int k = 0; k < size; k++) {
                PhiTPhi[i][j] += MUL_ELEMENTS(Phi[k][i], Phi[k][j]);
            }
        }
    }

    InvertMatrix(PhiTPhi, invPhiTPhi);

    // Calcular invPhiTPhiPhiT
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < size; j++) {
            invPhiTPhiPhiT[i][j] = 0;
            for (int k = 0; k < 5; k++) {
                invPhiTPhiPhiT[i][j] += MUL_ELEMENTS(invPhiTPhi[i][k], Phi[j][k]);
            }
        }
    }

    // Calcular XtY
    for (int i = 0; i < 5; i++) {
        XtY[i] = 0;
        for (int j = 0; j < size; j++) {
            XtY[i] += MUL_ELEMENTS(invPhiTPhiPhiT[i][j], Y[j]);
        }
    }


    a[0] = 1;
    a[1] = -XtY[0];
    a[2] = -XtY[1];
    b[0] = XtY[2];
    b[1] = XtY[3];
}

int main() {
    float a[3], b[2];

    // generate_prbs_signal(u_sys, DATA_SIZE);
    AcquireOutputSignal(u_sys, y_sys, DATA_SIZE);
    LeastSquares(u_sys, y_sys, DATA_SIZE, a, b);

    printf("System parameters:\n");
    printf("a0: %f, a1: %f, a2: %f\n", (float)DEN0 / (1<<15), (float)DEN1 / (1<<15), (float)DEN2 / (1<<15));
    printf("b0: %f, b1: %f\n\n", (float)NUM0 / (1<<15), (float)NUM1 / (1<<15));
    printf("Identified system parameters:\n");
    printf("DEN0 = %f\nDEN1 = %f\nDEN2 = %f\n", (float)a[0], (float)a[1], (float)a[2]);
    printf("NUM0 = %f\nNUM1 = %f\n\n", (float)b[0], (float)b[1]);
    return 0;
}

// System parameters:
// a0: 1.000000, a1: -1.263180, a2: 0.347999
// b0: 0.049768, b1: 0.035051

// Identified system parameters:
// DEN0 = 1.000000
// DEN1 = 1.263180
// DEN2 = -0.347999
// NUM0 = 0.049768
// NUM1 = 0.035051
