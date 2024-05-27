import os
import numpy as np
import matplotlib.pyplot as plt

def SquareFunc(fs: float, f0: float, amp: float = 1.0, samples: int = 100) -> [np.ndarray, np.ndarray]:
    t = np.arange(0, samples) / fs
    square = (amp * np.sign(np.sin(2 * np.pi * f0 * t)))
    return t, square

# Obtiene la ruta del archivo actual
current_dir = os.path.dirname(os.path.abspath(__file__))

# Ruta del archivo de salida
expected_output_dir = os.path.join(current_dir, "../test/support")
os.makedirs(expected_output_dir, exist_ok=True)  # Crea el directorio si no existe
expected_output_path = os.path.join(expected_output_dir, "expected_output.h")

# Definición de coeficientes en Q15
NUM = [int(0.00067056 * (1 << 15)), int(0.00064738 * (1 << 15))]
DEN = [int(1.0 * (1 << 15)), int(-1.898506534800089 * (1 << 15)), int(0.8998244812091836 * (1 << 15))]

# Buffers para mantener el estado
input_buffer = [0] * len(NUM)
output_buffer = [0] * (len(DEN) - 1)

# Función de recurrencia adaptada a Python
def recurrence_function(input_value):
    global input_buffer, output_buffer

    # Desplazar valores en el buffer de entrada
    for i in range(len(NUM) - 1, 0, -1):
        input_buffer[i] = input_buffer[i - 1]
    input_buffer[0] = input_value

    # Calcular la parte del numerador
    output = 0
    for i in range(len(NUM)):
        output += (NUM[i] * input_buffer[i]) >> 15

    # Calcular la parte del denominador
    for i in range(1, len(DEN)):
        output -= (DEN[i] * output_buffer[i - 1]) >> 15

    # Desplazar valores en el buffer de salida
    for i in range(len(DEN) - 2, 0, -1):
        output_buffer[i] = output_buffer[i - 1]
    output_buffer[0] = output

    return output

# Simulación en Python
t, u = SquareFunc(fs=1/0.005, f0=(1), amp=2**15, samples=200)

# Vector para almacenar la salida
y_1 = []

for value in u:
    y_1.append(recurrence_function(int(value)))

# Convertir a numpy array para facilitar el ploteo
y_1 = np.array(y_1)

# Encabezado del archivo
header_str = '''/**
 * @file expected_output.h
 * @author Marcos Dominguez
 *
 * @brief description
 *
 * @version 0.1
 * @date 2024-05-27
 */

#ifndef EXPECTED_OUTPUT_H
#define EXPECTED_OUTPUT_H

#ifdef  __cplusplus
extern "C" {
#endif

/*========= [DEPENDENCIES] =====================================================*/

#include "data_types.h"

/*========= [PUBLIC MACRO AND CONSTANTS] =======================================*/

/*========= [PUBLIC DATA TYPE] =================================================*/

'''

footer_str = '''

/*========= [PUBLIC FUNCTION DECLARATIONS] =====================================*/

#ifdef  __cplusplus
}

#endif

#endif  /* EXPECTED_OUTPUT_H */
'''

# Guardar la salida y la entrada en el formato deseado
output_str = f"static int32_t expected_output[] = {{{', '.join(map(str, y_1))}}};\n"
input_str = f"static int32_t input[] = {{{', '.join(map(str, u.astype(int)))}}};\n"

# Escribir los datos en un archivo
with open(expected_output_path, "w") as f:
    f.write(header_str)
    f.write(output_str)
    f.write(input_str)
    f.write(footer_str)

# Graficar la salida
plt.plot(t, y_1, label="output")
plt.plot(t, u, label="input")
plt.title("Output of Recurrence Function")
plt.xlabel("Time")
plt.ylabel("Output (Q15)")
plt.grid(True)
plt.legend()
plt.show()

print("Simulación completada, gráfico generado y salida guardada en 'expected_output.h'")
