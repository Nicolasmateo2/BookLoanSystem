//PROYECTO SISTEMAS OPERATIVOS
//Por: Katheryn Guasca, Juan Esteban Diaz, Nicolas Morales, Daniel Bohorquez
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// Nombre del pipe para la comunicación
#define PIPE_NAME "pipeReceptor"  

// Función para enviar solicitudes
void enviar_solicitud(int pipe_fd, char* operacion, char* libro, int isbn) {
    char mensaje[256];
    // Formato: Operación, Libro, ISBN
    snprintf(mensaje, sizeof(mensaje), "%s,%s,%d", operacion, libro, isbn);  
    // Escribimos el mensaje en el pipe
    write(pipe_fd, mensaje, strlen(mensaje) + 1);  
}

// Función para leer un archivo de entrada
void leer_archivo(const char* archivo) {
    FILE *fp = fopen(archivo, "r");
    if (fp == NULL) {
        perror("Error al abrir el archivo");
        exit(1);
    }

    char operacion[2], libro[100];
    int isbn;

    // Abrir el pipe FIFO
    int pipe_fd = open(PIPE_NAME, O_WRONLY);
    if (pipe_fd == -1) {
        perror("Error al abrir el pipe");
        exit(1);
    }

    // Leer el archivo línea por línea
    while (fscanf(fp, "%1s,%99[^,],%d", operacion, libro, &isbn) != EOF) {
        // Si encontramos el comando de salida "Q", terminamos
        if (strcmp(operacion, "Q") == 0) {
            printf("Fin de las solicitudes\n");
            break;
        }
        // Enviar solicitud al RP
        enviar_solicitud(pipe_fd, operacion, libro, isbn);
        printf("Enviando solicitud: %s %s %d\n", operacion, libro, isbn);
        sleep(1);  // Esperamos 1 segundo entre cada solicitud (simulación de tiempo)
    }

    // Cerramos el pipe
    close(pipe_fd);  
    // Cerramos el archivo
    fclose(fp);      
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Uso: %s <archivo de entrada>\n", argv[0]);
        exit(1);
    }

    // Comprobamos si el pipe existe, si no, lo creamos
    if (access(PIPE_NAME, F_OK) == -1) {
        if (mkfifo(PIPE_NAME, 0666) == -1) {
            perror("Error al crear el pipe FIFO");
            exit(1);
        }
    }

    // Llamamos a la función para leer el archivo y enviar las solicitudes
    leer_archivo(argv[1]);

    return 0;
}
