# Makefile para compilar identificacion_sistema.c

# Nombre del compilador
CC = gcc

# Opciones del compilador
CFLAGS = -Wall -Wextra -std=c99 -O2

# Nombre del ejecutable
TARGET = identificacion_sistema.o

# Fuentes
SRCS = identificacion_sistema.c

# Regla para compilar el ejecutable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Regla para limpiar los archivos generados
clean:
	rm -f $(TARGET)