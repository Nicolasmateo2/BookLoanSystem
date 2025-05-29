//PROYECTO SISTEMAS OPERATIVOS
//Por: Katheryn Guasca, Juan Esteban Diaz, Nicolas Morales, Daniel Bohorquez
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

//Definir una estructura para guardar los libros
//Prestado = 0 -> disponible, 1 -> prestado
typedef struct {
    char titulo[100];
    int isbn;
    int prestado;
} Libro;

// Nombre del pipe para la comunicación
#define PIPE_NAME "pipeReceptor"  

//Define el tamaño del buffer para el hilo auxiliar
#define BUFFER_SIZE 10

//Estructura que guarda los libros que se reciben 
#define MAX_LIBROS 100
Libro biblioteca[MAX_LIBROS];
int num_libros = 0;

// Estructura para las solicitudes del hilo auxiliar (D y R)
typedef struct {
    char operacion; // 'D' o 'R'
    char libro[100];
    int isbn;
} Solicitud;

// Buffer compartido y variables de sincronización
Solicitud buffer[BUFFER_SIZE];
int count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t lleno = PTHREAD_COND_INITIALIZER;
pthread_cond_t vacio = PTHREAD_COND_INITIALIZER;

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

// Función del hilo auxiliar (procesa D y R)
void* hilo_auxiliar(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (count == 0) {
            pthread_cond_wait(&lleno, &mutex); // Espera si el buffer está vacío
        }
        Solicitud solicitud = buffer[--count]; // Extrae una solicitud
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&vacio);

        // Procesa la solicitud (D o R)
        for (int i = 0; i < num_libros; i++) {
            if (strcmp(biblioteca[i].titulo, solicitud.libro) == 0 && 
                biblioteca[i].isbn == solicitud.isbn) {
                biblioteca[i].prestado = (solicitud.operacion == 'D') ? 0 : 1;
                printf("[Hilo Auxiliar] Procesado: %s de %s (ISBN: %d)\n",
                      (solicitud.operacion == 'D') ? "Devolución" : "Renovación",
                      solicitud.libro, solicitud.isbn);
                break;
            }
        }
    }
    return NULL;
}

void procesar_solicitud(char* mensaje) {
    char operacion[2], libro[100];
    int isbn;
    sscanf(mensaje, "%1s,%99[^,],%d", operacion, libro, &isbn);

    if (strcmp(operacion, "D") == 0 || strcmp(operacion, "R") == 0) {
        // Enviar al buffer del hilo auxiliar
        pthread_mutex_lock(&mutex);
        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&vacio, &mutex); // Espera si el buffer está lleno
        }
        strcpy(buffer[count].libro, libro);
        buffer[count].isbn = isbn;
        buffer[count].operacion = operacion[0];
        count++;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&lleno);
    } else if (strcmp(operacion, "P") == 0) {
        // Procesar préstamos directamente
        int encontrado = 0;
        for (int i = 0; i < num_libros; i++) {
            if (strcmp(biblioteca[i].titulo, libro) == 0 && biblioteca[i].isbn == isbn) {
                encontrado = 1;
                biblioteca[i].prestado = 1;
                printf("Préstamo exitoso: %s (ISBN: %d)\n", libro, isbn);
                break;
            }
        }
        if (!encontrado) {
            printf("⚠️ Libro no encontrado: %s\n", libro);
        }
    } else {
        printf("Operación desconocida: %s\n", operacion);
    }
    mostrar_biblioteca();
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
    while (1) {
        ssize_t bytes_read = read(pipe_fd, mensaje, sizeof(mensaje));
        if (bytes_read > 0) {
            printf("Solicitud recibida: %s\n", mensaje);
            if (strncmp(mensaje, "Q", 1) == 0) {
                printf("Proceso receptor finalizado.\n");
                break;
            }
            procesar_solicitud(mensaje);
        }
    }
    close(pipe_fd);
}

int main() {
    // Inicializar mutex y condiciones
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&lleno, NULL);
    pthread_cond_init(&vacio, NULL);

    // Crear hilo auxiliar
    pthread_t hilo_aux;
    pthread_create(&hilo_aux, NULL, hilo_auxiliar, NULL);

    // Manejar solicitudes
    manejar_solicitudes();

    // Limpieza (nunca se ejecutará por el bucle infinito)
    pthread_cancel(hilo_aux);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&lleno);
    pthread_cond_destroy(&vacio);

    return 0;
}