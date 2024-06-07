import argparse
import pandas as pd
import matplotlib.pyplot as plt
import os

# Configuración del parser de argumentos
parser = argparse.ArgumentParser(description='Seleccione el archivo CSV a abrir.')
parser.add_argument('-mode', choices=['OL', 'CL', 'PP', 'PPO'], default='OL', help='Modo de operación: OL o CL')

# Parsear los argumentos
args = parser.parse_args()

# Obtener la ruta del archivo actual
current_dir = os.path.dirname(os.path.abspath(__file__))

# Seleccionar el archivo CSV según el argumento
if args.mode == 'OL':
    csv_file = os.path.join(current_dir, 'results_ol.csv')
elif args.mode == 'CL':
    csv_file = os.path.join(current_dir, 'results_cl.csv')
elif args.mode == 'PP':
    csv_file = os.path.join(current_dir, 'results_pp.csv')
elif args.mode == 'PPO':
    csv_file = os.path.join(current_dir, 'results_ppo.csv')

# Leer el archivo CSV
df = pd.read_csv(csv_file)

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
plt.plot(df['time'] - df['time'].min(), df['output'], linestyle='-', color=CB_color_cycle[0], label='output - y')
plt.plot(df['time'] - df['time'].min(), df['pid'], linestyle='-', color=CB_color_cycle[1], label='input - u')
plt.plot(df['time'] - df['time'].min(), df['input'], linestyle='-', color=CB_color_cycle[2], label='reference - r')
plt.xlabel('Tiempo [ms]')
plt.ylabel('Salida [mV]')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
