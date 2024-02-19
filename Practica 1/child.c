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
     
    //pid_t pid = getpid();
    //printf("%d\n", pid);
    
    while(1){
        int funcion = rand() % 3;
        switch (funcion) {
        case 0: {
            char textoAleatorio[10];
            int bufferlog;
            for (int i = 0; i < 8; ++i) {
                textoAleatorio[i] = caracterAleatorio(); 
            }
            textoAleatorio[8] = '\n';
            textoAleatorio[9] = '\0';
    
            write(atoi(argv[1]), textoAleatorio, strlen(textoAleatorio));
            //printf("%s",textoAleatorio);
            //lseek(atoi(argv[3]), 0, SEEK_SET);
            read(atoi(argv[2]),&bufferlog,sizeof(int));
            int cantwrite=bufferlog;
            cantwrite=cantwrite+1;
            
            //sprintf(bufferlog, "%d", cantwrite);
            write(atoi(argv[3]), &cantwrite, sizeof(int));
            break;
        }
        case 1: {
            char buffer[9];
            int bufferlog;
            read(atoi(argv[1]), buffer, 8);
            //lseek(atoi(argv[3]), sizeof(int), SEEK_SET);
            read(atoi(argv[2]),&bufferlog,sizeof(int));
            int cantwrite=bufferlog;
            cantwrite=cantwrite+1;
            //lseek(atoi(argv[3]), sizeof(int), SEEK_SET);
            write(atoi(argv[3]), &cantwrite, sizeof(int));
            break;
        }
        case 2:
            int bufferlog;
            lseek(atoi(argv[1]), 0, SEEK_SET);
            read(atoi(argv[2]),&bufferlog,sizeof(int));
            int cantwrite=bufferlog;
            cantwrite=cantwrite+1;
            //lseek(atoi(argv[3]), sizeof(int), SEEK_SET);
            write(atoi(argv[3]), &cantwrite, sizeof(int));
            break;
        }
        int espera = rand() % 3+1;
        sleep(espera);
    }
}

