#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

#define MAX_USERS 111
#define MAX_LINE_LENGTH 100

typedef struct {
    int account_number;
    char name[50];
    float balance;
} User;

typedef struct {
    int thread_id;
    int loaded_users;
    int errors;
} ThreadData;

typedef struct {
    int operation;
    int account1;
    int account2;
    float amount;
} Operation;

User users[MAX_USERS];
int total_users = 0;
int total_errors = 0;
pthread_mutex_t mutex;
FILE *report; // Declarar como variable global para su acceso desde load_users()
FILE *transaction;
int lineaTemp=0;



void *load_transaccion(void *data) {
    ThreadData *thread_data = (ThreadData *)data;
    
    char line[MAX_LINE_LENGTH];
    int index = 0;
    bool banderaLectura =false;
    pthread_mutex_lock(&mutex);
    if (fgets(line, sizeof(line), transaction)!=NULL)
    {
        banderaLectura=true;
    }
    pthread_mutex_unlock(&mutex);
    while (banderaLectura) {
        int lineaActual=lineaTemp;
        lineaTemp++;
        if (true) {
            char *token = strtok(line, ",");
            int operacion = atoi(token);
            token = strtok(NULL, ",");
            int cuenta1 = atoi(token);
            token = strtok(NULL, ",");
            int cuenta2 = atoi(token);
            token = strtok(NULL, ",");
            float monto = atof(token);

            if (operacion==1)
            {
                User *user = find_user(cuenta1);
                if (user == NULL) {
                    pthread_mutex_lock(&mutex);
                    fprintf(report, "Hilo %d: Error en línea %d - Número de cuenta no existe \n", 
                    thread_data->thread_id, lineaActual + 1);
                    pthread_mutex_unlock(&mutex);
                } else if (monto <= 0) {
                    pthread_mutex_lock(&mutex);
                    fprintf(report, "Hilo %d: Error en línea %d - Saldo menor a 0 \n", 
                    thread_data->thread_id, lineaActual + 1);
                    pthread_mutex_unlock(&mutex);
                }else{
                    pthread_mutex_lock(&mutex);
                    user->balance += monto;
                    pthread_mutex_unlock(&mutex);
                }
                
            }else if(operacion==2){
                User *user = find_user(cuenta1);
                if (user == NULL) {
                    pthread_mutex_lock(&mutex);
                    fprintf(report, "Hilo %d: Error en línea %d - Número de cuenta no existe \n", 
                    thread_data->thread_id, lineaActual + 1);
                    pthread_mutex_unlock(&mutex);
                }else if (monto <= 0) {
                    pthread_mutex_lock(&mutex);
                    fprintf(report, "Hilo %d: Error en línea %d - Monto menor a 0 \n", 
                    thread_data->thread_id, lineaActual + 1);
                    pthread_mutex_unlock(&mutex);
                }  else if (user->balance < monto) {
                    pthread_mutex_lock(&mutex);
                    fprintf(report, "Hilo %d: Error en línea %d - Saldo menor a monto \n", 
                    thread_data->thread_id, lineaActual + 1);
                    pthread_mutex_unlock(&mutex);
                }else{
                    pthread_mutex_lock(&mutex);
                    user->balance -= monto;
                    pthread_mutex_unlock(&mutex);
                }   
            }else if(operacion==3){
                User *from_user = find_user(cuenta1);
                User *to_user = find_user(cuenta2);
                if (from_user == NULL) {
                    pthread_mutex_lock(&mutex);
                    fprintf(report, "Hilo %d: Error en línea %d - Número de cuenta origen no existe \n", 
                    thread_data->thread_id, lineaActual + 1);
                    pthread_mutex_unlock(&mutex);
                }else if (to_user == NULL) {
                    pthread_mutex_lock(&mutex);
                    fprintf(report, "Hilo %d: Error en línea %d - Número de cuenta destino no existe \n", 
                    thread_data->thread_id, lineaActual + 1);
                    pthread_mutex_unlock(&mutex);
                }else if (monto <= 0 ) {
                    pthread_mutex_lock(&mutex);
                    fprintf(report, "Hilo %d: Error en línea %d - Monto menor a 0 \n", 
                    thread_data->thread_id, lineaActual + 1);
                    pthread_mutex_unlock(&mutex);
                }else if (from_user->balance < monto) {
                    pthread_mutex_lock(&mutex);
                    fprintf(report, "Hilo %d: Error en línea %d - Saldo menor a monto \n", 
                    thread_data->thread_id, lineaActual + 1);
                    pthread_mutex_unlock(&mutex);
                }else{
                    pthread_mutex_lock(&mutex);
                    from_user->balance -= monto;
                    to_user->balance += monto;
                    pthread_mutex_unlock(&mutex);
                }
            }else{
                pthread_mutex_lock(&mutex);
                fprintf(report, "Hilo %d: Error en línea %d - Operacion no existe %d \n", 
                thread_data->thread_id, lineaActual + 1,operacion);
                pthread_mutex_unlock(&mutex);
            }
        }
        pthread_mutex_lock(&mutex);
        if (fgets(line, sizeof(line), transaction)!=NULL)
        {
            banderaLectura=true;
        }else{
            banderaLectura=false;
        }
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_mutex_init(&mutex, NULL);

    pthread_t threadsT[4];
    ThreadData thread_dataT[4];


    // Construir el nombre del archivo de reporte
    char report_filename[50];
    snprintf(report_filename, sizeof(report_filename), "carga_%s.log");

    // Abrir archivo de reporte
    report = fopen(report_filename, "w");
    if (report == NULL) {
        perror("Error al crear el archivo de reporte");
        exit(EXIT_FAILURE);
    }
    
    transaction = fopen("prueba_transacciones.csv", "r");
    
    
	for (int i = 0; i < 4; i++) {
                    thread_dataT[i].thread_id = i + 1;
                    pthread_create(&threadsT[i], NULL, load_transaccion, (void *)&thread_dataT[i]);
                }

                for (int i = 0; i < 4; i++) {
                    pthread_join(threadsT[i], NULL);
                }
                fclose(report);
    pthread_mutex_destroy(&mutex);
    return 0;
}