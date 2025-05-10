//PROYECTO SISTEMAS OPERATIVOS
//Por: Katheryn Guasca, Juan Esteban Diaz, Nicolas Morales, Daniel Bohorquez
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

//Definir una estructura para guardar los libros
//Prestado = 0 -> disponible, 1 -> prestado
typedef struct {
    char titulo[100];
    int isbn;
    int prestado;
} Libro;

// Nombre del pipe para la comunicación
#define PIPE_NAME "pipeReceptor"  

//Estructura que guarda los libros que se reciben 
#define MAX_LIBROS 100
Libro biblioteca[MAX_LIBROS];
int num_libros = 0;

// 🔎 Función para mostrar los libros en la biblioteca
void mostrar_biblioteca() {
    printf("\n📚 Estado actual de la biblioteca:\n");
    for (int i = 0; i < num_libros; i++) {
        printf("- %s (ISBN: %d) [%s]\n",
               biblioteca[i].titulo,
               biblioteca[i].isbn,
               biblioteca[i].prestado ? "Prestado" : "Disponible");
    }
    printf("\n");
}

void procesar_solicitud(char* mensaje) {
    char operacion[2], libro[100];
    int isbn;
    
    sscanf(mensaje, "%1s,%99[^,],%d", operacion, libro, &isbn); 

    // Verificar si el libro ya existe
    int encontrado = 0;
    for (int i = 0; i < num_libros; i++) {
        if (strcmp(biblioteca[i].titulo, libro) == 0 && biblioteca[i].isbn == isbn) {
            encontrado = 1;
            // Actualizar según la operación
            if (strcmp(operacion, "P") == 0) {
                biblioteca[i].prestado = 1;
            } else if (strcmp(operacion, "D") == 0) {
                biblioteca[i].prestado = 0;
            } else if (strcmp(operacion, "R") == 0) {
                biblioteca[i].prestado = 1; // renovación = mantener prestado
            }
            break;
        }
    }

    // Si no se encontró, agregarlo
    if (!encontrado && num_libros < MAX_LIBROS) {
        strcpy(biblioteca[num_libros].titulo, libro);
        biblioteca[num_libros].isbn = isbn;
        if (strcmp(operacion, "P") == 0 || strcmp(operacion, "R") == 0) {
            biblioteca[num_libros].prestado = 1;
        } else {
            biblioteca[num_libros].prestado = 0;
        }
        num_libros++;
    }

    // Mostrar mensaje de operación
    if (strcmp(operacion, "P") == 0) {
        printf("Procesando préstamo: %s (ISBN: %d)\n", libro, isbn);
    } else if (strcmp(operacion, "R") == 0) {
        printf("Procesando renovación: %s (ISBN: %d)\n", libro, isbn);
    } else if (strcmp(operacion, "D") == 0) {
        printf("Procesando devolución: %s (ISBN: %d)\n", libro, isbn);
    } else {
        printf("Operación desconocida: %s\n", operacion);
    }

    // Mostrar el estado completo
    printf("\n📚 Estado actual de la biblioteca:\n");
    for (int i = 0; i < num_libros; i++) {
        printf("- %s (ISBN: %d) [%s]\n", biblioteca[i].titulo, biblioteca[i].isbn,
               biblioteca[i].prestado ? "Prestado" : "Disponible");
    }
}


// Función para manejar las solicitudes
void manejar_solicitudes() {
    // Abrir el pipe FIFO en modo lectura
    int pipe_fd = open(PIPE_NAME, O_RDONLY);
    if (pipe_fd == -1) {
        perror("Error al abrir el pipe");
        exit(1);
    }

    char mensaje[256];
    
    // Leer las solicitudes de forma continua
    while (1) {
        ssize_t bytes_read = read(pipe_fd, mensaje, sizeof(mensaje));
        
        if (bytes_read > 0) {
            // Imprimir el mensaje recibido
            printf("Solicitud recibida: %s\n", mensaje);
            
            // Si el mensaje es "Q", se termina el proceso
            if (strncmp(mensaje, "Q", 1) == 0) {
                printf("Proceso receptor finalizado.\n");
                break;
            }
            
            // Procesar la solicitud recibida
            procesar_solicitud(mensaje);
        }
    }
    
    // Cerrar el pipe
    close(pipe_fd);  
}

int main() {
    // Llamamos a la función para manejar las solicitudes
    manejar_solicitudes();
    return 0;
}
