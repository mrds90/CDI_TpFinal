/**
 * @file identificacion.c
 * @author 
 *
 * @brief 
 *
 * @version 0.1
 * @date 2024-05-27
 */

/*========= [DEPENDENCIES] =====================================================*/

#include "identificacion.h"
#include <string.h>

/*========= [PRIVATE MACROS AND CONSTANTS] =====================================*/

#define ORDER 2
#define MAX_SAMPLES 200

/*========= [PRIVATE DATA TYPES] ===============================================*/

/*========= [TASK DECLARATIONS] ================================================*/

/*========= [PRIVATE FUNCTION DECLARATIONS] ====================================*/

STATIC void matrix_multiply(int16_t *A, int16_t *B, int16_t *C, int m, int n, int p);
STATIC void matrix_transpose(int16_t *A, int16_t *B, int m, int n);
STATIC bool_t matrix_inverse(int16_t *A, int16_t *A_inv, int n);

/*========= [INTERRUPT FUNCTION DECLARATIONS] ==================================*/

/*========= [LOCAL VARIABLES] ==================================================*/

/*========= [STATE FUNCTION POINTERS] ==========================================*/

/*========= [PUBLIC FUNCTION IMPLEMENTATION] ===================================*/

void Identify_SystemParameters(int n, int16_t *u, int16_t *y, int len, int16_t *Theta) {
    int i, j;
    int num_filas = len - n;
    int num_columnas = 2 * n + 1;

    // Segmento de y desde el índice n hasta el final
    int16_t *Y = y + n;

    // Inicializa la matriz Phi con ceros
    static int16_t Phi[(MAX_SAMPLES - ORDER) * (2 * ORDER + 1)] = {0};

    // Construcción de Phi
    for (i = 0; i < num_filas; i++) {
        for (j = 0; j < n; j++) {
            Phi[i * num_columnas + j] = y[n + i - 1 - j];
        }
        for (j = 0; j <= n; j++) {
            Phi[i * num_columnas + (n + j)] = u[n + i - j];
        }
    }

    // Transponer Phi
    static int16_t Phi_T[(2 * ORDER + 1) * (MAX_SAMPLES - ORDER)] = {0};
    matrix_transpose(Phi, Phi_T, num_filas, num_columnas);

    // Calcula Phi' * Phi
    static int16_t PhiT_Phi[(2 * ORDER + 1) * (2 * ORDER + 1)] = {0};
    matrix_multiply(Phi_T, Phi, PhiT_Phi, num_columnas, num_filas, num_columnas);

    // Calcula (Phi' * Phi)^-1
    static int16_t PhiT_Phi_inv[(2 * ORDER + 1) * (2 * ORDER + 1)] = {0};
    if (!matrix_inverse(PhiT_Phi, PhiT_Phi_inv, num_columnas)) {
        // Manejar el caso donde la matriz no es invertible
        return;
    }

    // Calcula (Phi' * Phi)^-1 * Phi'
    static int16_t Temp[(2 * ORDER + 1) * MAX_SAMPLES] = {0};
    matrix_multiply(PhiT_Phi_inv, Phi_T, Temp, num_columnas, num_columnas, num_filas);

    // Calcula Theta = (Phi' * Phi)^-1 * Phi' * Y
    for (i = 0; i < num_columnas; i++) {
        Theta[i] = 0;
        for (j = 0; j < num_filas; j++) {
            Theta[i] += (Temp[i * num_filas + j] * Y[j]) >> 15; // Ajuste para Q15
        }
    }
}

/*========= [PRIVATE FUNCTION IMPLEMENTATION] ==================================*/

// Función para transponer una matriz
STATIC void matrix_transpose(int16_t *A, int16_t *B, int m, int n) {
    int i, j;
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            B[j * m + i] = A[i * n + j];
        }
    }
}

// Función para multiplicar dos matrices
STATIC void matrix_multiply(int16_t *A, int16_t *B, int16_t *C, int m, int n, int p) {
    int i, j, k;
    for (i = 0; i < m; i++) {
        for (j = 0; j < p; j++) {
            C[i * p + j] = 0;
            for (k = 0; k < n; k++) {
                C[i * p + j] += (A[i * n + k] * B[k * p + j]) >> 15; // Ajuste para Q15
            }
        }
    }
}

// Función para invertir una matriz cuadrada (utilizando un método simple, no recomendado para producción)
STATIC bool_t matrix_inverse(int16_t *A, int16_t *A_inv, int n) {
    int i, j, k;
    int32_t temp;

    // Crear una matriz identidad de tamaño nxn
    int16_t identity[n * n];
    memset(identity, 0, n * n * sizeof(int16_t));
    for (i = 0; i < n; i++) {
        identity[i * n + i] = 1 << 15; // 1 en Q15
    }

    // Copiar A a A_inv para trabajar en la matriz A_inv
    memcpy(A_inv, A, n * n * sizeof(int16_t));

    // Aplicar el método de Gauss-Jordan
    for (i = 0; i < n; i++) {
        // Asegurar que el elemento diagonal sea diferente de cero
        if (A_inv[i * n + i] == 0) {
            // Encontrar una fila para intercambiar
            bool_t found_nonzero = FALSE;
            for (k = i + 1; k < n; k++) {
                if (A_inv[k * n + i] != 0) {
                    for (j = 0; j < n; j++) {
                        temp = A_inv[i * n + j];
                        A_inv[i * n + j] = A_inv[k * n + j];
                        A_inv[k * n + j] = temp;

                        temp = identity[i * n + j];
                        identity[i * n + j] = identity[k * n + j];
                        identity[k * n + j] = temp;
                    }
                    found_nonzero = TRUE;
                    break;
                }
            }
            // Si no se encuentra una fila adecuada, la matriz no es invertible
            if (!found_nonzero) {
                return FALSE; // Indicar que la matriz no es invertible
            }
        }

        // Escalar la fila para hacer que el elemento diagonal sea 1
        temp = A_inv[i * n + i];
        for (j = 0; j < n; j++) {
            A_inv[i * n + j] = (A_inv[i * n + j] << 15) / temp;
            identity[i * n + j] = (identity[i * n + j] << 15) / temp;
        }

        // Hacer ceros los elementos sobre y bajo el pivote
        for (k = 0; k < n; k++) {
            if (k != i) {
                temp = A_inv[k * n + i];
                for (j = 0; j < n; j++) {
                    A_inv[k * n + j] -= (A_inv[i * n + j] * temp) >> 15;
                    identity[k * n + j] -= (identity[i * n + j] * temp) >> 15;
                }
            }
        }
    }

    // Copiar el resultado en A_inv
    memcpy(A_inv, identity, n * n * sizeof(int16_t));
    return TRUE; // Indicar que la matriz fue invertida con éxito
}

/*========= [INTERRUPT FUNCTION IMPLEMENTATION] ================================*/

