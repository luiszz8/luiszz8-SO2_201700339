#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

// Se define una variable global para contar el número total de llamadas al sistema
int total_syscalls = 0;

// Función para manejar la señal SIGINT
void sigint_handler(int signum) {
    printf("\nNúmero total de llamadas al sistema: %d\n", total_syscalls);
    printf("Terminando proceso padre\n");
    exit(0);
}

int main() {
    // Configurar el manejador de señales para SIGINT
    signal(SIGINT, sigint_handler);
    int fd = open("practica1.txt", O_RDWR | O_CREAT, 0777);
    char fdd[20];
    sprintf(fdd, "%d", fd);
    // Crear el primer hijo
    pid_t pid1 = fork();

    if (pid1 == -1) {
        perror("fork");
        exit(1);
    }

    if (pid1 == 0) {
        printf("Hijjo 1");
        // Código para el primer hijo
        signal(SIGINT, SIG_DFL);
        char *arg_Ptr[3];
        arg_Ptr[0] = "child.bin"; // Se corrigió el nombre del ejecutable del hijo
        arg_Ptr[1] = fdd; // Se cambió el argumento para identificar al primer hijo
        arg_Ptr[2] = NULL;

        execv("/home/luis/Escritorio/Practica 1/child.bin", arg_Ptr);
    }

    // Crear el segundo hijo
    pid_t pid2 = fork();

    if (pid2 == -1) {
        perror("fork");
        exit(1);
    }

    if (pid2 == 0) {
        printf("Hijjo 2");
        // Código para el segundo hijo
        signal(SIGINT, SIG_DFL);
        char *arg_Ptr[3];
        arg_Ptr[0] = "child.bin"; // Se corrigió el nombre del ejecutable del hijo
        arg_Ptr[1] = fdd; // Se cambió el argumento para identificar al segundo hijo
        arg_Ptr[2] = NULL;

        execv("/home/luis/Escritorio/Practica 1/child.bin", arg_Ptr);
    }

    // Código para el padre
    printf("Soy el proceso padre\n");
    
    // Esperar a que ambos hijos terminen
    int status;
    char command[100];
    sprintf(command, "%s %d %d %s", "sudo stap trace.stp ", pid1, pid2, " > calls.log");
    system(command);
    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);
    
    return 0;
}
