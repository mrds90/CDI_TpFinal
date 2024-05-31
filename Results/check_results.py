import argparse
import pandas as pd
import matplotlib.pyplot as plt

# Configuración del parser de argumentos
parser = argparse.ArgumentParser(description='Seleccione el archivo CSV a abrir.')
parser.add_argument('-mode', choices=['OL', 'CL'], default='OL', help='Modo de operación: OL o CL')

# Parsear los argumentos
args = parser.parse_args()


# Seleccionar el archivo CSV según el argumento
if args.mode == 'OL':
    df = pd.read_csv('results_ol.csv')
else:
    df = pd.read_csv('results_cl.csv')

output_max = df['output'].max()
output_min = df['output'].min()
t1 = df['time'][df['output'] > (0.1 * (output_max - output_min) + output_min)].iloc[0]
t2 = df['time'][df['output'] > (0.9 * (output_max - output_min) + output_min)].iloc[0]
rise_time = t2 - t1
        
print(t2-t1)

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

# Crear los gráficos
plt.figure(figsize=(12, 6))

# Gráfico del output respecto al tiempo
plt.plot(df['time'], df['output'], linestyle='-', color=CB_color_cycle[0], label='Output')
plt.plot(df['time'], df['pid'], linestyle='-', color=CB_color_cycle[1], label='Pid')
plt.plot(df['time'], df['input'], linestyle='-', color=CB_color_cycle[2], label='Input')
plt.xlabel('Tiempo [ms]')
plt.ylabel('Salida [mV]')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
