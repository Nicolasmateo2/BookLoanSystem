// PROYECTO SISTEMAS OPERATIVOS
// Por: Katheryn Guasca, Juan Esteban Díaz, Nicolás Morales, Daniel Bohórquez

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

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

//Funcion que permite buscar un Libro a partir de un nombre, retorna un Apuntador a un Libro
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

void enviar_solicitud(int pipe_fd, char* operacion, char* libro, int isbn) {
    char mensaje[256];
    snprintf(mensaje, sizeof(mensaje), "%s,%s,%d", operacion, libro, isbn);
    write(pipe_fd, mensaje, strlen(mensaje) + 1);
}

// Elimina espacios en blanco al inicio y fin de una cadena
void trim(char* str) {
    char* end;

    // Eliminar espacios iniciales
    while (*str == ' ' || *str == '\n') str++;

    // Si la cadena quedó vacía
    if (*str == 0) return;

    // Eliminar espacios finales y saltos de línea
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\n')) end--;

    // Nuevo fin de cadena
    *(end + 1) = '\0';
}


// ================= MODO MANUAL ================= //

// Función para leer un archivo de entrada
void modo_manual(const char* archivo) {
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


// ================= MODO INTERACTIVO ================= //

void modo_interactivo() {
    int pipe_fd = open(PIPE_NAME, O_WRONLY);
    if (pipe_fd == -1) {
        perror("❌ Error al abrir el pipe");
        exit(1);
    }

    int opcion;
    char libro[100];
    int isbn;
    char operacion[2];

    while (1) {
        printf("\n📚 Menú de Solicitudes:\n");
        printf("1. Solicitar préstamo\n");
        printf("2. Solicitar renovación\n");
        printf("3. Solicitar devolución\n");
        printf("4. Salir\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);
        getchar();  // limpiar '\n'

        if (opcion == 4) break;

        printf("Ingrese el título del libro: ");
        fgets(libro, sizeof(libro), stdin);
        libro[strcspn(libro, "\n")] = 0;

        //Llamado a la funcion buscarLibroPorNombre para ver que el libro exista en la biblioteca
        Libro* encontrado = buscarLibroPorNombre(libro);
        if (encontrado == NULL) {
            printf("⚠️  El libro \"%s\" no está en la biblioteca. Intente nuevamente.\n", libro);
            continue;
        }

        //Asignarle el isbn del Apuntador del Libro encontrado
        isbn = encontrado->isbn;

        switch (opcion) {
            case 1: strcpy(operacion, "P"); break;
            case 2: strcpy(operacion, "R"); break;
            case 3: strcpy(operacion, "D"); break;
            default: 
                printf("⚠️  Opción no válida.\n");
                continue;
        }

        //Se envia la solicitud escrita manualmente 
        enviar_solicitud(pipe_fd, operacion, libro, isbn);
        printf("📨 Solicitud enviada: %s %s %d\n", operacion, libro, isbn);
    }

    close(pipe_fd);
    printf("✅ Modo interactivo finalizado.\n");
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

    // Elegir modo
    int modo;
    printf("\n📌 Seleccione modo de operación:\n");
    printf("1. Modo Manual (leer archivo de solicitudes)\n");
    printf("2. Modo Interactivo (menú)\n");
    printf("Ingrese opción: ");
    scanf("%d", &modo);
    getchar(); // Limpiar buffer

    if (modo == 1) {
        char archivo_solicitudes[100];
        printf("📂 Ingrese el archivo de solicitudes: ");
        fgets(archivo_solicitudes, sizeof(archivo_solicitudes), stdin);
        archivo_solicitudes[strcspn(archivo_solicitudes, "\n")] = 0;
        modo_manual(archivo_solicitudes);
    } else if (modo == 2) {
        modo_interactivo();
    } else {
        printf("⚠️  Opción no válida. Terminando...\n");
    }

    return 0;
}
