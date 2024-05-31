import pandas as pd
import matplotlib.pyplot as plt
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
# Cargar el archivo CSV
df = pd.read_csv('resultsol.csv')
# df = pd.read_csv('results.csv')


output_max = df['output'].max()
t1 = df['time'][df['output'] > (0.1 * output_max)].iloc[0]
t2 = df['time'][df['output'] > (0.9 * output_max)].iloc[0]
rise_time = t2 - t1
        
print(t2-t1)

# Crear los gráficos
plt.figure(figsize=(12, 6))

# Gráfico del output respecto al tiempo
plt.plot(df['time'], df['output'], linestyle='-', color=CB_color_cycle[0], label='Output')
# plt.plot(df['time'], df['pid'], linestyle='-', color=CB_color_cycle[1], label='Pid')
plt.plot(df['time'], df['input'], linestyle='-', color=CB_color_cycle[2], label='Input')
plt.xlabel('Tiempo [ms]')
plt.ylabel('Salida [mV]')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
