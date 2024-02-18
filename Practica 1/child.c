#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

char caracterAleatorio(){
    int choice = rand() % 2;
    if (choice == 0) {
        return 'a' + rand() % 26;
    } else {
        return '0' + rand() % 10;
    }
}

int main(int argc, char *argv[]){
     
    pid_t pid = getpid();
    printf("%d\n", pid);

    while(1){
        int funcion = rand() % 3;
        switch (funcion) {
        case 0: {
            char textoAleatorio[10];
            for (int i = 0; i < 8; ++i) {
                textoAleatorio[i] = caracterAleatorio(); 
            }
            textoAleatorio[8] = '\n';
            textoAleatorio[9] = '\0';

            write(atoi(argv[1]), textoAleatorio, strlen(textoAleatorio));
            break;
        }
        case 1: {
            char buffer[9];
            read(atoi(argv[1]), buffer, 8);
            buffer[8] = '\0';
            break;
        }
        case 2:
            lseek(atoi(argv[1]), 0, SEEK_SET);
            break;
        }
        int espera = rand() % 3+1;
        sleep(espera);
    }
}

