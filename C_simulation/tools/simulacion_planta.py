import os
import numpy as np
import matplotlib.pyplot as plt
import control as cnt

CB_color_cycle = (
    "#377eb8",
    "#ff7f00",
    "#4daf4a",
    "#f781bf",
    "#a65628",
    "#984ea3",
    "#999999",
    "#e41a1c",
    "#dede00",
)

def SquareFunc(fs: float, f0: float, amp: float = 1.0, samples: int = 100) -> [np.ndarray, np.ndarray]:
    t_step = np.arange(0, samples) / fs
    square = (amp * np.sign(np.sin(2 * np.pi * f0 * t_step)))
    square[square < 0] = 0
    return t_step, square

# Obtiene la ruta del archivo actual
current_dir = os.path.dirname(os.path.abspath(__file__))

# Ruta del archivo de salida
expected_output_dir = os.path.join(current_dir, "../test/support")
os.makedirs(expected_output_dir, exist_ok=True)  # Crea el directorio si no existe
expected_output_path = os.path.join(expected_output_dir, "expected_output.h")

r_1 = 10e3
c_1 = 1e-6
r_2 = 1e3 #valor particular de la planta
c_2 = 1e-6

s = cnt.tf('s')
hs_1 = 1 / ((r_1*c_1*r_2*c_2)*s*s + (r_1*c_1+r_1*c_2+r_2*c_2)*s + 1)
num_1, den_1 = cnt.tfdata(hs_1)
h = 0.005
hz_1 = cnt.c2d(hs_1, h, 'zoh')
numz_planta, denz_planta = cnt.tfdata(hz_1)
numz_planta = numz_planta[0][0]
denz_planta= denz_planta[0][0]

DEN0 = denz_planta[0]
DEN1 = denz_planta[1]
DEN2 = denz_planta[2]
NUM0 = numz_planta[0]
NUM1 = numz_planta[1]

# DEN0 = 1.876489
# DEN1 = -1.265066
# DEN2 = 0.386865
# NUM0 = 0.003344
# NUM1 = 0.023523
# Definición de coeficientes en Q15
NUM = [int(NUM0 * (1 << 15)), int(NUM1 * (1 << 15))]
DEN = [int(DEN0 * (1 << 15)), int(DEN1 * (1 << 15)), int(DEN2 * (1 << 15))]

# NUM = [int(0.0041309458458261838 * (1 << 15)), int(0.048339736956908297  * (1 << 15))]
# DEN = [int(1.8107526733935444 * (1 << 15)), int(-1.1851826187871666 * (1 << 15)), int(0.32212874048961293 * (1 << 15))]

# NUM = [int(0.04976845 * (1 << 15)), int(0.03505064 * (1 << 15))]
# DEN = [int(1.0 * (1 << 15)), int(-1.2631799459800208 * (1 << 15)), int(0.34799904079225535 * (1 << 15))]

# Buffers para mantener el estado
input_buffer = [0] * len(NUM)
output_buffer = [0] * (len(DEN) - 1)
input_buffer2 = [0] * len(NUM)
output_buffer2 = [0] * (len(DEN) - 1)

def reset_recurrence(num, den):
    global input_buffer, output_buffer
    input_buffer = [0] * len(num)
    output_buffer = [0] * (len(den) - 1)
    
# Función de recurrencia adaptada a Python
def recurrence_function(input_value, num, den):
    global input_buffer, output_buffer

    # Desplazar valores en el buffer de entrada
    for i in range(len(num) - 1, 0, -1):
        input_buffer[i] = input_buffer[i - 1]
    input_buffer[0] = input_value

    # Calcular la parte del numerador
    output = 0
    for i in range(len(num)):
        output += (num[i] * input_buffer[i]) >> 15

    # Calcular la parte del denominador
    for i in range(1, len(den)):
        output -= (den[i] * output_buffer[i - 1]) >> 15

    # Desplazar valores en el buffer de salida
    for i in range(len(den) - 2, 0, -1):
        output_buffer[i] = output_buffer[i - 1]
    output_buffer[0] = output

    return output


def reset_recurrence2(num, den):
    global input_buffer2, output_buffer2
    input_buffer2 = [0] * len(num)
    output_buffer2 = [0] * (len(den) - 1)
    
def recurrence_function2(input_value, num, den):
    global input_buffer2, output_buffer2

    # Desplazar valores en el buffer de entrada
    for i in range(len(num) - 1, 0, -1):
        input_buffer2[i] = input_buffer2[i - 1]
    input_buffer2[0] = input_value

    # Calcular la parte del numerador
    output = 0
    for i in range(len(num)):
        output += (num[i] * input_buffer2[i]) >> 15

    # Calcular la parte del denominador
    for i in range(1, len(den)):
        output -= (den[i] * output_buffer2[i - 1]) >> 15

    # Desplazar valores en el buffer de salida
    for i in range(len(den) - 2, 0, -1):
        output_buffer2[i] = output_buffer2[i - 1]
    output_buffer2[0] = output

    return output


# Simulación en Python
# t_step, u_step = SquareFunc(fs=1/h, f0=(10), amp=2**15, samples=(1/h))
t_step = np.arange(0,6,h)
u_step = np.concatenate((np.zeros(int(len(t_step) / 2)), 2**15 * np.ones(int(len(t_step) / 2))), axis=None)

# Vector para almacenar la salida
y_step = []

for value in u_step:
    y_step.append(recurrence_function(int(value),NUM,DEN))

# Convertir a numpy array para facilitar el ploteo
y_step = np.array(y_step)
step_input_str = f"static int32_t step_input[] = {{{', '.join(map(str, u_step.astype(int)))}}};\n"
step_output_str = f"static int32_t step_expected_output[] = {{{', '.join(map(str, y_step))}}};\n\n"

reset_recurrence2(NUM,DEN)

OFFSET = 9929
AMP = 9930

t_square, u_square = SquareFunc(fs=1/h, f0=(1), amp=AMP, samples=5*(1/h))
u_square+= OFFSET
u_square=u_square[1:]
t_square=t_square[1:]

u_90 = np.ones(int(len(t_square) )) * (0.9 * (AMP) + OFFSET)
t_90 = t_square
u_10 = np.ones(int(len(t_square) )) * (0.1 * (AMP) + OFFSET)
t_10 = t_square

y_square = []

for value in u_square:
    y_square.append(recurrence_function2(int(value),NUM,DEN))

# Convertir a numpy array para facilitar el ploteo
y_square = np.array(y_square)

t1 = t_square[y_square > (0.1 * (AMP) + OFFSET)][0]
t2 = t_square[y_square > (0.9 * (AMP) + OFFSET)][0]

print(t2-t1)

square_input_str = f"static int32_t square_input[] = {{{', '.join(map(str, u_square.astype(int)))}}};\n"
square_output_str = f"static int32_t square_expected_output[] = {{{', '.join(map(str, y_square))}}};\n\n"



K  = 4.0 
Ti = 0.04
Td = 0.005
pid = cnt.tf([K*Td, K,K/Ti], [1, 0])

pid_sys  = cnt.feedback(pid, 1)
hz_1 = cnt.c2d(pid_sys, h, 'zoh')
numz_1, denz_1 = cnt.tfdata(hz_1)

numz_1 = numz_1[0][0]
denz_1 = denz_1[0][0]

y_sys = [0]
# numz_1 = [int(0.41666059 * (1<<15)), int(-0.54398644 * (1<<15)), int(0.15342283 * (1<<15))]
# denz_1 = [int(1.0 * (1<<15)), int(-1.8569365254781038 * (1<<15)), int(1.0826991410679994 * (1<<15)), int(-0.19966564006790186 * (1<<15))]
print(numz_1)
print(denz_1)

numz_1 = (numz_1 * (1 << 15)).astype(int)
denz_1 = (denz_1 * (1 << 15)).astype(int)
reset_recurrence2(numz_1, denz_1)
reset_recurrence(NUM,DEN)
y_pid = []
for i, value in enumerate(u_square):
    error = 2*int(value)-int(y_sys[i])
    pid_output = recurrence_function2(error, numz_1, denz_1)
    y_pid.append(pid_output)
    system_output = recurrence_function(pid_output, NUM, DEN)
    y_sys.append(system_output)
# for value in u_square:
#     y_sys.append(recurrence_function2(int(value),numz_1,denz_1))

# Convertir a numpy array para facilitar el ploteo
y_sys = np.array(y_sys[1:])

pid_output_str = f"static int32_t pid_expected_output[] = {{{', '.join(map(str, y_pid))}}};\n"
cont_sys_output_str = f"static int32_t controlled_expected_output[] = {{{', '.join(map(str, y_sys))}}};\n\n"
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


# Escribir los datos en un archivo
with open(expected_output_path, "w") as f:
    f.write(header_str)
    f.write(step_input_str)
    f.write(step_output_str)
    f.write(square_input_str)
    f.write(square_output_str)
    f.write(pid_output_str)
    f.write(cont_sys_output_str)

    
    f.write(footer_str)

# Graficar la salida
plt.plot(t_step, y_step, label="output")
plt.plot(t_step, u_step, label="input")
plt.title("Respuesta al escalón")
plt.xlabel("Tiempo (seg)")
plt.ylabel("Salida (Q15)")
plt.grid(True)
plt.legend()
plt.show()

plt.plot(t_square, y_square, label="output",color=CB_color_cycle[0])
plt.plot(t_square, u_square, label="input",color=CB_color_cycle[1])
plt.plot(t_90, u_90, label="i_90", linestyle="--",color=CB_color_cycle[2])
plt.plot(t_10, u_10, label="i_10", linestyle="--",color=CB_color_cycle[2])
plt.title("Respuesta a la Cuadrada")
plt.xlabel("Tiempo (seg)")
plt.ylabel("Salida (Q15)")
plt.grid(True)
plt.legend()
plt.show()

plt.plot(t_square, y_square, label="output",color=CB_color_cycle[0])
plt.plot(t_square, u_square, label="input",color=CB_color_cycle[1])
plt.plot(t_square, y_sys, label="pid_sys",color=CB_color_cycle[3])
# plt.plot(t_square, y_pid, label="pid",color=CB_color_cycle[4])
plt.plot(t_90, u_90, label="i_90", linestyle="--",color=CB_color_cycle[2])
plt.plot(t_10, u_10, label="i_10", linestyle="--",color=CB_color_cycle[2])
plt.title("Output of Recurrence Function with pid")
plt.xlabel("Time")
plt.ylabel("Output (Q15)")
plt.grid(True)
plt.legend()
plt.show()

print("Simulación completada, gráfico generado y salida guardada en 'expected_output.h'")
