#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

#define MAX_USERS 111
#define MAX_TRANSACIONES 204
#define MAX_LINE_LENGTH 100

typedef struct {
    int account_number;
    char name[50];
    float balance;
} User;

typedef struct {
    int thread_id;
    int start_index;
    int end_index;
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
int total_errorsT = 0;
pthread_mutex_t mutex;
FILE *report;
FILE *reportT; 
FILE *transaction;
int lineaTemp=0;

bool is_valid_balance(const char *balance_str) {
    int length = strlen(balance_str);
    for (int i = 0; i < length; i++) {
        if (balance_str[i] == '\r' || balance_str[i] == '\n'|| balance_str[i] == '-') {
            continue; // Ignorar los caracteres de salto de línea
        }
        if (!isdigit(balance_str[i]) && balance_str[i] != '.') {
            return false;
        }
    }
    return true;
}

void *load_users(void *data) {
    ThreadData *thread_data = (ThreadData *)data;
    FILE *file = fopen("usuarios.csv", "r");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    int index = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (index >= thread_data->start_index && index <= thread_data->end_index) {
            char *token = strtok(line, ",");
            int account_number = atoi(token);
            token = strtok(NULL, ",");
            char name[50];
            strcpy(name, token);
            token = strtok(NULL, ",");
            float balance = atof(token);

            if (!is_valid_balance(token))
            {
                pthread_mutex_lock(&mutex);
                total_errors++;
                fprintf(report, "Hilo %d: Error en línea %d - Número de cuenta: %d, Saldo no es número \n", 
                    thread_data->thread_id, index + 1, account_number);
                pthread_mutex_unlock(&mutex);
            }else if (account_number <= 0 || !is_valid_balance(token) || balance < 0 ) {
                pthread_mutex_lock(&mutex);
                total_errors++;
                fprintf(report, "Hilo %d: Error en línea %d - Número de cuenta: %d, Saldo: %.2f\n", 
                    thread_data->thread_id, index + 1, account_number, balance);
                pthread_mutex_unlock(&mutex);
            } else {
                pthread_mutex_lock(&mutex);
                int duplicate = 0;
                for (int i = 0; i < total_users; i++) {
                    if (users[i].account_number == account_number) {
                        duplicate = 1;
                        break;
                    }
                }
                if (duplicate) {
                    total_errors++;
                    fprintf(report, "Hilo %d: Error en línea %d - Número de cuenta duplicado: %d\n",
                        thread_data->thread_id, index + 1, account_number);
                } else {
                    User user;
                    user.account_number = account_number;
                    strcpy(user.name, name);
                    user.balance = balance;
                    users[total_users++] = user;
                    thread_data->loaded_users++;
                }
                pthread_mutex_unlock(&mutex);
            }
        }
        index++;
    }

    fclose(file);
    pthread_exit(NULL);
}

// Función para buscar una cuenta por su número de cuenta
User *find_user(int account_number) {
    for (int i = 0; i < total_users; i++) {
        if (users[i].account_number == account_number) {
            return &users[i];
        }
    }
    return NULL;
}

void *load_transaccion(void *data) {
    ThreadData *thread_data = (ThreadData *)data;
    FILE *file = fopen("transacciones.csv", "r");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }
    
    char line[MAX_LINE_LENGTH];
    int index = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (index >= thread_data->start_index && index <= thread_data->end_index) {
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
                    total_errorsT++;
                    fprintf(reportT, "Hilo %d: Error en línea %d - Número de cuenta no existe \n", 
                    thread_data->thread_id, index + 1);
                    pthread_mutex_unlock(&mutex);
                } else if (monto <= 0) {
                    pthread_mutex_lock(&mutex);
                    total_errorsT++;
                    fprintf(reportT, "Hilo %d: Error en línea %d - Saldo menor a 0 \n", 
                    thread_data->thread_id, index + 1);
                    pthread_mutex_unlock(&mutex);
                }else{
                    pthread_mutex_lock(&mutex);
                    user->balance += monto;
                    thread_data->loaded_users++;
                    pthread_mutex_unlock(&mutex);
                }
                
            }else if(operacion==2){
                User *user = find_user(cuenta1);
                if (user == NULL) {
                    pthread_mutex_lock(&mutex);
                    total_errorsT++;
                    fprintf(reportT, "Hilo %d: Error en línea %d - Número de cuenta no existe \n", 
                    thread_data->thread_id, index + 1);
                    pthread_mutex_unlock(&mutex);
                }else if (monto <= 0) {
                    pthread_mutex_lock(&mutex);
                    total_errorsT++;
                    fprintf(reportT, "Hilo %d: Error en línea %d - Monto menor a 0 \n", 
                    thread_data->thread_id, index + 1);
                    pthread_mutex_unlock(&mutex);
                }  else if (user->balance < monto) {
                    pthread_mutex_lock(&mutex);
                    total_errorsT++;
                    fprintf(reportT, "Hilo %d: Error en línea %d - Saldo menor a monto \n", 
                    thread_data->thread_id, index + 1);
                    pthread_mutex_unlock(&mutex);
                }else{
                    pthread_mutex_lock(&mutex);
                    user->balance -= monto;
                    thread_data->loaded_users++;
                    pthread_mutex_unlock(&mutex);
                }   
            }else if(operacion==3){
                User *from_user = find_user(cuenta1);
                User *to_user = find_user(cuenta2);
                if (from_user == NULL) {
                    pthread_mutex_lock(&mutex);
                    total_errorsT++;
                    fprintf(reportT, "Hilo %d: Error en línea %d - Número de cuenta origen no existe \n", 
                    thread_data->thread_id, index + 1);
                    pthread_mutex_unlock(&mutex);
                }else if (to_user == NULL) {
                    pthread_mutex_lock(&mutex);
                    total_errorsT++;
                    fprintf(reportT, "Hilo %d: Error en línea %d - Número de cuenta destino no existe \n", 
                    thread_data->thread_id, index + 1);
                    pthread_mutex_unlock(&mutex);
                }else if (monto <= 0 ) {
                    pthread_mutex_lock(&mutex);
                    total_errorsT++;
                    fprintf(reportT, "Hilo %d: Error en línea %d - Monto menor a 0 \n", 
                    thread_data->thread_id, index + 1);
                    pthread_mutex_unlock(&mutex);
                }else if (from_user->balance < monto) {
                    pthread_mutex_lock(&mutex);
                    total_errorsT++;
                    fprintf(reportT, "Hilo %d: Error en línea %d - Saldo menor a monto \n", 
                    thread_data->thread_id, index + 1);
                    pthread_mutex_unlock(&mutex);
                }else{
                    pthread_mutex_lock(&mutex);
                    from_user->balance -= monto;
                    to_user->balance += monto;
                    thread_data->loaded_users++;
                    pthread_mutex_unlock(&mutex);
                }
            }else{
                pthread_mutex_lock(&mutex);
                total_errorsT++;
                fprintf(reportT, "Hilo %d: Error en línea %d - Operacion no existe %d \n", 
                thread_data->thread_id, index + 1,operacion);
                pthread_mutex_unlock(&mutex);
            }
        }
        index++;
    }
    fclose(file);
    pthread_exit(NULL);
}

// Función para realizar un depósito
void deposito() {
    int account_number;
    float amount;
    printf("Ingrese el número de cuenta: ");
    scanf("%d", &account_number);
    User *user = find_user(account_number);
    if (user == NULL) {
        printf("Error: No existe el número de cuenta.\n");
        return;
    }
    printf("Ingrese el monto a depositar: ");
    scanf("%f", &amount);
    if (amount <= 0) {
        printf("Error: El monto indicado no es válido.\n");
        return;
    }
    user->balance += amount;
    printf("Depósito realizado correctamente.\n");
}

// Función para realizar un retiro
void retiro() {
    int account_number;
    float amount;
    printf("Ingrese el número de cuenta: ");
    scanf("%d", &account_number);
    User *user = find_user(account_number);
    if (user == NULL) {
        printf("Error: No existe el número de cuenta.\n");
        return;
    }
    printf("Ingrese el monto a retirar: ");
    scanf("%f", &amount);
    if (amount <= 0) {
        printf("Error: El monto indicado no es válido.\n");
        return;
    }
    if (user->balance < amount) {
        printf("Error: La cuenta no tiene saldo suficiente para realizar el retiro.\n");
        return;
    }
    user->balance -= amount;
    printf("Retiro realizado correctamente.\n");
}

// Función para realizar una transferencia
void transfer() {
    int from_account, to_account;
    float amount;
    printf("Ingrese el número de cuenta de origen: ");
    scanf("%d", &from_account);
    User *from_user = find_user(from_account);
    if (from_user == NULL) {
        printf("Error: No existe el número de cuenta de origen.\n");
        return;
    }
    printf("Ingrese el número de cuenta de destino: ");
    scanf("%d", &to_account);
    User *to_user = find_user(to_account);
    if (to_user == NULL) {
        printf("Error: No existe el número de cuenta de destino.\n");
        return;
    }
    printf("Ingrese el monto a transferir: ");
    scanf("%f", &amount);
    if (amount <= 0 ) {
        printf("Error: El monto indicado no es válido.\n");
        return;
    }
    if (from_user->balance < amount) {
        printf("Error: La cuenta de origen no tiene saldo suficiente para realizar la transferencia.\n");
        return;
    }
    from_user->balance -= amount;
    to_user->balance += amount;
    printf("Transferencia realizada correctamente.\n");
}

// Función para consultar una cuenta
void view_account() {
    int account_number;
    printf("Ingrese el número de cuenta: ");
    scanf("%d", &account_number);
    User *user = find_user(account_number);
    if (user == NULL) {
        printf("Error: No existe el número de cuenta.\n");
        return;
    }
    printf("Número de cuenta: %d\n", user->account_number);
    printf("Nombre: %s\n", user->name);
    printf("Saldo: %.2f\n", user->balance);
}

// Función para buscar una cuenta por su número de cuenta
void estado_cuenta() {
    FILE *file = fopen("estado_cuentas.csv", "w");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "no_cuenta,nombre,saldo \n");
    for (int i = 0; i < total_users; i++) {
        fprintf(file, "%d, %s, %.2f \n", 
                    users[i].account_number,users[i].name,users[i].balance);
    }
    fclose(file);
}

int main() {
    pthread_mutex_init(&mutex, NULL);
    pthread_t threads[3];
    ThreadData thread_data[3];
    pthread_t threadsT[4];
    ThreadData thread_dataT[4];
    int lines_per_thread = MAX_USERS / 3;
    int lines_per_threadT = MAX_TRANSACIONES / 4;

    // Obtener la fecha y hora actual
    time_t current_time;
    struct tm *time_info;
    char time_str[20];
    time(&current_time);
    time_info = localtime(&current_time);
    strftime(time_str, sizeof(time_str), "%Y_%m_%d-%H_%M_%S", time_info);

    // Construir el nombre del archivo de reporte
    char report_filename[50];
    snprintf(report_filename, sizeof(report_filename), "carga_%s.log", time_str);

    // Abrir archivo de reporte
    report = fopen(report_filename, "w");
    if (report == NULL) {
        perror("Error al crear el archivo de reporte");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 3; i++) {
        thread_data[i].thread_id = i + 1;
        thread_data[i].start_index = i * lines_per_thread;
        if (i==0)
        {
            thread_data[i].start_index = 1;
        }
        
        thread_data[i].end_index = (i + 1) * lines_per_thread - 1;
        thread_data[i].loaded_users = 0;
        thread_data[i].errors = 0;
        pthread_create(&threads[i], NULL, load_users, (void *)&thread_data[i]);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    // Generar reporte de carga
    fprintf(report, "Reporte de carga - %d-%02d-%02d %02d:%02d:%02d\n",
        time_info->tm_year + 1900, time_info->tm_mon + 1, time_info->tm_mday,
        time_info->tm_hour, time_info->tm_min, time_info->tm_sec);

    fprintf(report, "\nDetalles de la carga por hilo:\n");
    int total_loaded_users = 0;
    for (int i = 0; i < 3; i++) {
        fprintf(report, "Hilo %d: %d usuarios cargados\n", 
            thread_data[i].thread_id, thread_data[i].loaded_users);
        total_loaded_users += thread_data[i].loaded_users;
    }
    fprintf(report, "Total de usuarios cargados: %d\n", total_loaded_users);
    fprintf(report, "Total de errores: %d\n", total_errors);
    
    fclose(report);
    // Menú de operaciones
    int choice;
    do {
        printf("\nMenú de Operaciones:\n");
        printf("1. Depósito\n");
        printf("2. Retiro\n");
        printf("3. Transacción\n");
        printf("4. Consultar cuenta\n");
        printf("5. Carga Masiva\n");
        printf("6. Estado de Cuentas\n");
        printf("7. Salir\n");
        printf("Seleccione una opción: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                deposito();
                break;
            case 2:
                retiro();
                break;
            case 3:
                transfer();
                break;
            case 4:
                view_account();
                break;
            case 5:
                // Obtener la fecha y hora actual
                //time_t current_time;
                //struct tm *time_info;
                //char time_str[20];
                time(&current_time);
                time_info = localtime(&current_time);
                strftime(time_str, sizeof(time_str), "%Y_%m_%d-%H_%M_%S", time_info);

                // Construir el nombre del archivo de reporte
                char report_filenameT[50];
                snprintf(report_filenameT, sizeof(report_filenameT), "operaciones_%s.log", time_str);

                // Abrir archivo de reporte
                reportT = fopen(report_filenameT, "w");
                if (report == NULL) {
                    perror("Error al crear el archivo de reporte");
                    exit(EXIT_FAILURE);
                }
                for (int i = 0; i < 4; i++) {
                    thread_dataT[i].thread_id = i + 1;
                    thread_dataT[i].start_index = i * lines_per_threadT;
                    if (i==0)
                    {
                        thread_dataT[i].start_index = 1;
                    }
        
                    thread_dataT[i].end_index = (i + 1) * lines_per_threadT - 1;
                    thread_dataT[i].loaded_users = 0;
                    thread_dataT[i].errors = 0;
                    pthread_create(&threadsT[i], NULL, load_transaccion, (void *)&thread_dataT[i]);
                }

                for (int i = 0; i < 4; i++) {
                    pthread_join(threadsT[i], NULL);
                }
                // Generar reporte de carga
                fprintf(reportT, "Reporte de carga - %d-%02d-%02d %02d:%02d:%02d\n",
                time_info->tm_year + 1900, time_info->tm_mon + 1, time_info->tm_mday,
                time_info->tm_hour, time_info->tm_min, time_info->tm_sec);

                fprintf(reportT, "\nDetalles de la carga por hilo:\n");
                int total_loaded_trans = 0;
                for (int i = 0; i < 4; i++) {
                    fprintf(reportT, "Hilo %d: %d transacciones cargadas\n", 
                        thread_dataT[i].thread_id, thread_dataT[i].loaded_users);
                    total_loaded_trans += thread_dataT[i].loaded_users;
                }
                fprintf(reportT, "Total de transacciones cargadas: %d\n", total_loaded_trans);
                fprintf(reportT, "Total de errores: %d\n", total_errorsT);
                fclose(reportT);
                printf("Carga Lista\n");
                break;
            case 6:
                estado_cuenta();
                break;
            case 7:
                printf("Saliendo.\n");
                break;
            default:
                printf("Opción no válida. Por favor, seleccione una opción válida.\n");
        }
    } while (choice != 7);
    pthread_mutex_destroy(&mutex);
    return 0;
}
