import os
import numpy as np
import matplotlib.pyplot as plt
import control as cnt
from filter_constructor import FilterGenerator

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

def calculate_rise_time(signal, time,ref_value=1):
    # Find the index where the signal first crosses the start threshold
    start_index = np.argmax(signal >= 0.1*ref_value)
    
    # Find the index where the signal crosses the end threshold
    end_index = np.argmax(signal >= 0.9*ref_value)
    
    # Calculate the rise time as the difference between the two indices, multiplied by the time step
    rise_time = time[end_index] - time[start_index]
    
    return rise_time

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
r_2 = 18e3
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




# Nombre del archivo
file_name = "real_world_filter"
c_file_path = "../CIAAFirmware/src"
h_file_path = "../CIAAFirmware/inc"
print(numz_planta)
print(denz_planta)
# Llamar al script de generación de archivos .c y .h
module = FilterGenerator(file_name, numz_planta, denz_planta)
module.write_files(c_file_path, h_file_path)

# print("DEN0 = ",DEN0)
# print("DEN1 = ",DEN1)
# print("DEN2 = ",DEN2)
# print("NUM0 = ",NUM0)
# print("NUM1 = ",NUM1)

print("// Coeficientes del numerador en Q15")
print(f"#define NUM0 Q15_SCALE({NUM0})")
print(f"#define NUM1 Q15_SCALE({NUM1})")
print("\n// Coeficientes del denominador en Q15")
print(f"#define DEN0 Q15_SCALE({DEN0})")
print(f"#define DEN1 Q15_SCALE({DEN1})")
print(f"#define DEN2 Q15_SCALE({DEN2})")
# # DEN0 = 1.000000
# # DEN1 = -0.914136
# # DEN2 = -0.043714
# # NUM0 = 0.566069
# # NUM1 = -0.524298


# Definición de coeficientes en Q15
NUM = [int(NUM0 * (1 << 15)), int(NUM1 * (1 << 15))]
DEN = [int(DEN0 * (1 << 15)), int(DEN1 * (1 << 15)), int(DEN2 * (1 << 15))]


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

# t1 = t_square[y_square > (0.1 * (AMP) + OFFSET)][0]
# t2 = t_square[y_square > (0.9 * (AMP) + OFFSET)][0]

# print(t2-t1)

square_input_str = f"static int32_t square_input[] = {{{', '.join(map(str, u_square.astype(int)))}}};\n"
square_output_str = f"static int32_t square_expected_output[] = {{{', '.join(map(str, y_square))}}};\n\n"


## PID
t = np.arange(0, 2, h)
u = np.ones(len(t))
t_ol, y_ol = cnt.forced_response(hs_1, t, u)
x1 = t_ol[y_ol> (0.1 * y_ol.max())][0]
x2 = t_ol[y_ol> (0.9 * y_ol.max())][0]
y1 = y_ol[y_ol> (0.1 * y_ol.max())][0]
y2 = y_ol[y_ol> (0.9 * y_ol.max())][0]

print(x2 - x1) # Tiempo de subida
# Ecuación de la recta
t1 = np.linspace(0, x2, 5000)
y_r = ((y2-y1)/(x2-x1))*(t1-x1)+y1

L = x1
T = x2-x1
# A = B = 1

# PID con Z-N
K  = 1.2*T/L
Ti = 2*L
Td = 0.5*L

print(K,Ti,Td)

pid = cnt.tf([K*Td, K, K/Ti], [1, 0])

print("coeficientes del PID:")
print(pid)
cl_sys  = cnt.feedback(hs_1*pid, 1)
t_cl, y_cl = cnt.forced_response(cl_sys, t, u)

rt = calculate_rise_time(y_ol,t_ol)
print('Tiempo de subida en LA:',rt)
print('Tiempo de subida deseado:',0.7*rt)

rt = calculate_rise_time(t_cl,y_cl)
print('Tiempo de subida en con PID:',rt)
print('Sobrepico: ',max(y_cl))

plt.grid(color='k', linestyle='-', linewidth=0.2)
plt.plot(t_ol, y_ol, 'r--', label='Out_ol')
plt.plot(t_cl, y_cl, 'b-', label='Out_cl')
plt.legend(loc='best')
plt.xlabel('Time (s)')
plt.ylabel('Amplitude')
plt.show()

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

numz_1 = (numz_1 * (1 << 15)).astype(int)
denz_1 = (denz_1 * (1 << 15)).astype(int)

print(numz_1)
print(denz_1)

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
plt.title("Respuesta a la Cuadrada con pid")
plt.xlabel("Tiempo [Seg]")
plt.ylabel("Output (Q15)")
plt.grid(True)
plt.legend()
plt.show()

print("Simulación completada, gráfico generado y salida guardada en 'expected_output.h'")
