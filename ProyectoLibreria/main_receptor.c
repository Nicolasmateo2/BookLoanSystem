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
#include <time.h>

#define PIPE_NAME "pipeReceptor"
#define MAX_LIBROS 100

typedef struct {
    char titulo[100];
    int isbn;
    int prestado;  // 0 -> disponible, 1 -> prestado
} Libro;

Libro biblioteca[MAX_LIBROS];
int num_libros = 0;

// ================= FUNCIONES AUXILIARES ================= //

Libro* buscarLibroPorNombre(const char* nombre) {
    for (int i = 0; i < num_libros; i++) {
        if (strcmp(biblioteca[i].titulo, nombre) == 0) {
            return &biblioteca[i];
        }
    }
    return NULL;
}

void cargarLibrosDesdeArchivo() {
    char nombre_archivo[100];
    printf("📥 Ingrese el archivo de libros disponibles (formato: titulo,isbn,estado): ");
    fgets(nombre_archivo, sizeof(nombre_archivo), stdin);
    nombre_archivo[strcspn(nombre_archivo, "\n")] = 0;

    FILE* archivo = fopen(nombre_archivo, "r");
    if (!archivo) {
        perror("❌ Error al abrir archivo de libros");
        exit(1);
    }

    char linea[200];
    while (fgets(linea, sizeof(linea), archivo)) {
        char titulo[100];
        int isbn, prestado;
        if (sscanf(linea, "%99[^,],%d,%d", titulo, &isbn, &prestado) == 3) {
            if (num_libros < MAX_LIBROS) {
                strcpy(biblioteca[num_libros].titulo, titulo);
                biblioteca[num_libros].isbn = isbn;
                biblioteca[num_libros].prestado = prestado;
                num_libros++;
            }
        }
    }

    fclose(archivo);
    printf("✅ Libros cargados correctamente (%d libros).\n", num_libros);
}

void mostrar_biblioteca() {
    printf("\n📚 Estado actual de la biblioteca:\n");
    for (int i = 0; i < num_libros; i++) {
        printf("- %s (ISBN: %d) [%s]\n",
               biblioteca[i].titulo,
               biblioteca[i].isbn,
               biblioteca[i].prestado ? "Prestado" : "Disponible");
    }
}

// ================= IMPLEMENTACIÓN DE COMANDOS ================= //

void* manejar_comandos(void* arg) {
    char comando[50];
    while (1) {
        printf("\nIngrese comando (reporte / salir): ");
        fgets(comando, sizeof(comando), stdin);
        comando[strcspn(comando, "\n")] = '\0';

        if (strcmp(comando, "salir") == 0) {
            printf("Terminando proceso receptor...\n");
            exit(0);
        } else if (strcmp(comando, "reporte") == 0) {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            printf("\n📊 Reporte de Libros - %02d-%02d-%d\n", 
                   tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
            printf("==================================\n");
            
            for (int i = 0; i < num_libros; i++) {
                printf("📖 %s (ISBN: %d) - %s\n",
                       biblioteca[i].titulo,
                       biblioteca[i].isbn,
                       biblioteca[i].prestado ? "PRESTADO" : "DISPONIBLE");
            }
            printf("==================================\n");
            printf("Total libros: %d\n", num_libros);
        } else {
            printf("Comando no reconocido. Use reporte o salir\n");
        }
    }
    return NULL;
}

// ================= PROCESAMIENTO DE SOLICITUDES ================= //

void procesar_solicitud(char* mensaje) {
    char operacion[2], libro[100];
    int isbn;
    sscanf(mensaje, "%1s,%99[^,],%d", operacion, libro, &isbn);

    for (int i = 0; i < num_libros; i++) {
        if (strcmp(biblioteca[i].titulo, libro) == 0 && biblioteca[i].isbn == isbn) {
            switch(operacion[0]) {
                case 'P': // Préstamo
                    if (biblioteca[i].prestado) {
                        printf("Error: %s ya está prestado\n", libro);
                    } else {
                        biblioteca[i].prestado = 1;
                        printf("Préstamo exitoso: %s\n", libro);
                    }
                    break;
                    
                case 'R': // Renovación
                    if (!biblioteca[i].prestado) {
                        printf("Error: %s no está prestado\n", libro);
                    } else {
                        printf("Renovación exitosa: %s\n", libro);
                    }
                    break;
                    
                case 'D': // Devolución
                    if (!biblioteca[i].prestado) {
                        printf("Error: %s no estaba prestado\n", libro);
                    } else {
                        biblioteca[i].prestado = 0;
                        printf("Devolución exitosa: %s\n", libro);
                    }
                    break;
                    
                default:
                    printf("Operación desconocida: %s\n", operacion);
            }
            mostrar_biblioteca();
            return;
        }
    }
    printf("Error: Libro %s (ISBN: %d) no encontrado\n", libro, isbn);
}

// ================= FUNCIÓN PRINCIPAL ================= //

int main() {
    // Crear pipe si no existe
    if (access(PIPE_NAME, F_OK) == -1) {
        if (mkfifo(PIPE_NAME, 0666) == -1) {
            perror("❌ Error al crear el pipe FIFO");
            exit(1);
        }
    }

    // Cargar biblioteca
    cargarLibrosDesdeArchivo();

    // Iniciar hilo para manejar comandos
    pthread_t hilo_comandos;
    pthread_create(&hilo_comandos, NULL, manejar_comandos, NULL);

    // Abrir pipe para lectura
    int pipe_fd = open(PIPE_NAME, O_RDONLY);
    if (pipe_fd == -1) {
        perror("❌ Error al abrir el pipe");
        exit(1);
    }

    printf("✅ Receptor iniciado. Esperando solicitudes...\n");

    // Procesar solicitudes
    char mensaje[256];
    while (1) {
        ssize_t bytes_read = read(pipe_fd, mensaje, sizeof(mensaje));
        if (bytes_read > 0) {
            printf("Solicitud recibida: %s\n", mensaje);
            if (strncmp(mensaje, "Q", 1) == 0) {
                printf("Recibido comando de terminación.\n");
                break;
            }
            procesar_solicitud(mensaje);
        }
    }

    // Limpieza
    close(pipe_fd);
    pthread_cancel(hilo_comandos);
    
    return 0;
}