# Makefile para el sistema de préstamo de libros
# Compila los programas receptor y solicitante

# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -lpthread

# Definición de objetivos principales
all: receptor solicitante

# Regla para compilar el receptor
receptor: main_receptor.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Regla para compilar el solicitante
solicitante: main_solicitante.c
	$(CC) $(CFLAGS) -o $@ $<

# Regla para crear el pipe nominal
pipe:
	mkfifo -m 0666 pipeReceptor || true

# Regla para limpiar los archivos generados
clean:
	rm -f receptor solicitante pipeReceptor

# Regla para ejecutar el sistema (receptor + solicitante)
run: all pipe
	./receptor -f libros.txt -v & ./solicitante

.PHONY: all clean pipe run