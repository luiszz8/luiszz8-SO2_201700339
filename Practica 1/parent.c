#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>


int logc=0;

void sigint_handler(int signum) {
    int lineas = 0;
    int writeC = 0;
    int ReadC = 0;
    int SeekC = 0;


    char buffer[1024];
    int logcAux = open("syscalls.log", O_RDWR, 0777);

    const char *word_to_search1 = "write";
    const char *word_to_search2 = "read";
    const char *word_to_search3 = "lseek";
    lseek(logcAux, 0, SEEK_SET);

    while (read(logcAux, buffer, sizeof(buffer)) > 0) {
        char *token = strtok(buffer, "\n"); 

        while (token != NULL) {
            lineas++; 

            if (strstr(token, word_to_search1) != NULL) {
                writeC++; 
            }

            if (strstr(token, word_to_search2) != NULL) {
                ReadC++; 
            }

            if (strstr(token, word_to_search3) != NULL) {
                SeekC++; 
            }

            token = strtok(NULL, "\n"); 
        }
    }


    close(logcAux);


    printf("Cantidad de procesos: %d \n", lineas-1);
    printf("Write: %d \n", writeC);
    printf("Read: %d \n", ReadC);
    printf("Lseek: %d \n", SeekC);
    printf("Terminando proceso padre\n");
    exit(0);
}

int main() {
    signal(SIGINT, sigint_handler);
    int fd = open("practica1.txt",  O_RDWR | O_CREAT | O_TRUNC, 0777);
    char fdd[20];
    int fdp[2]; //Descriptor del pipe
    pipe(fdp);
    logc=fdp[0];
    char fdp1[9];
    char fdp2[9];
    sprintf(fdd, "%d", fd);
    sprintf(fdp1, "%d", fdp[0]);
    sprintf(fdp2, "%d", fdp[1]);

    pid_t pid1 = fork();

    if (pid1 == -1) {
        perror("fork");
        exit(1);
    }

    if (pid1 == 0) {
        //printf("Hijjo 1");
        // Código para el primer hijo
        //signal(SIGINT, SIG_DFL);
        char *arg_Ptr[4];
        arg_Ptr[0] = "child.bin";
        arg_Ptr[1] = fdd; 
        arg_Ptr[2] = fdp1;
        arg_Ptr[3] = fdp2;
        arg_Ptr[4] = NULL;

        execv("/home/luis/Escritorio/SO2/Practica 1/child.bin", arg_Ptr);
    }

    // Crear el segundo hijo
    pid_t pid2 = fork();

    if (pid2 == -1) {
        perror("fork");
        exit(1);
    }

    if (pid2 == 0) {
        //printf("Hijjo 2");
        //signal(SIGINT, SIG_DFL);
        char *arg_Ptr[4];
        arg_Ptr[0] = "child.bin"; 
        arg_Ptr[1] = fdd; 
        arg_Ptr[2] = fdp1;
        arg_Ptr[3] = fdp2;
        arg_Ptr[4] = NULL;

        execv("/home/luis/Escritorio/SO2/Practica 1/child.bin", arg_Ptr);
    }

    // Código para el padre
    //printf("Soy el proceso padre\n");
    int inciial=0;
    write(fdp[1], &inciial, sizeof(int));
    // Esperar a que ambos hijos terminen
    int status;
    char command[100];
    sprintf(command, "%s %d %d %s", "sudo stap trace.stp ", pid1, pid2, " > syscalls.log");
    system(command);
    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);
    
    return 0;
}