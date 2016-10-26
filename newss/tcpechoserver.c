/**
* @title Server
*
* @author Mathieu De Valery 10-10193
* @author Marisela Del Valle 11-10267
*
* @description
*           Crea el servidor y crea un nuevo socket para que uno o mas clientes se conecten.
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

// Tamano maximo del buffer
#define MAX_BUFF 100

// Para crear hilos
void *connection_handler(void *);

// Variables del cajero
int TotalDisponible;

struct Datos {
    char *nombre_entrada;
    char *nombre_salida;
    int socket;
    FILE *archivo_deposito;
    FILE *archivo_retiro;
};

// Funcion principal
void main(int numArgs , char *args[]){

    // Verificamos los parametros de entrada.

    if (numArgs != 7){
        printf("Error de argumentos: numero equivocado de argumentos.\n");
        printf("Sintaxis: ");
        printf("bsb_svr -l <puerto_bsb_svr> -i <bitácora_deposito> -o <bitácora_retiro>\n" );
        exit(1);
    }

    char *puerto, *b_deposito, *b_retiro;
    int i;

    for (i = 1; i <= 5; i = i + 2){
        if (strcmp(args[i],"-l") == 0){
            puerto = strdup(args[i+1]);
        }
        else if (strcmp(args[i],"-i") == 0){
            b_deposito = strdup(args[i+1]);
        }
        else if (strcmp(args[i],"-o") == 0){
            b_retiro = strdup(args[i+1]);
        }
        else{
            printf("Error de argumentos: Argumentos equivocados.\n");
            printf("Sintaxis: ");
            printf("bsb_svr -l <puerto_bsb_svr> -i <bitácora_deposito> -o <bitácora_retiro>\n" );
            exit(1);
        }
    }

    // Verificamos que los archivos de entrada y salida no se llamen igual
    if ((strcmp(b_deposito,b_retiro) == 0)){
        printf("Error de argumentos: Los archivos de entrada y salida no deben llamarse igual.\n" );
        exit(1);
    }

    // Variables para el socket
    int socket_desc , client_sock , c , *new_sock, port;
    struct sockaddr_in server , client;

    // Archivos de logs de deposito y retiro
    FILE *archivo_deposito, *archivo_retiro;

    //Creacion del socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    port = atoi(puerto);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {

        perror("bind failed. Error");
        exit(1);
    }

    //Listen
    listen(socket_desc , 3);


    //Acepta la conexion
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);


    //Creamos los archivos para guardar los logs de deposito y retiro
    archivo_deposito = fopen(b_deposito,"a");

    if (!(archivo_deposito)){
        fprintf(stderr, "No se pudo crear el archivo de deposito.\n");
    }

    archivo_retiro = fopen(b_retiro,"a");

    if (!(archivo_retiro)){
        fprintf(stderr, "No se pudo crear el archivo de retiro.\n");
    }

    TotalDisponible = 80000;


    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");



        // Creamos los hilos para multiples conexiones
        pthread_t sniffer_thread;

        struct Datos datos;
        datos.socket = client_sock;
        datos.nombre_entrada = b_deposito;
        datos.nombre_salida = b_retiro;
        datos.archivo_deposito = archivo_deposito;
        datos.archivo_retiro = archivo_retiro;

        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void *)&datos) < 0)
        {
            perror("could not create thread");
            exit(1);
        }

        puts("Handler assigned");


    }

    if (client_sock < 0)
    {
        perror("accept failed");
        exit(1);
    }
}


//Manejo de conexion con varios clientes a traves de hilos
void *connection_handler(void *datos){

    // Archivos de logs de deposito y retiro

    struct Datos* mis_datos = (struct Datos*)datos;
    //Informacion del socket
    int sock = mis_datos->socket;
    int read_size, *monto;
    char *message , client_message[2000];
    char *b_deposito = mis_datos->nombre_entrada;
    char *b_retiro = mis_datos->nombre_salida;
    FILE *archivo_deposito = mis_datos->archivo_deposito;
    FILE *archivo_retiro = mis_datos->archivo_retiro;

    ////////////////////////////////////////////////////////////////

    char buff_rcvd[MAX_BUFF], HOUR[30];

    archivo_deposito = fopen(b_deposito,"a");
    archivo_retiro = fopen(b_retiro,"a");

    //Enviamos y recibimos mensajes del cliente

    while( (read_size = recv(sock , buff_rcvd , MAX_BUFF , 0)) > 0 )
    {

        if (TotalDisponible < 5000){
            puts("Total disponible menor a 5000");
        }
        // Chequeamos si la accion es de retiro
        if ( buff_rcvd[0] == 'r'){
            write(sock , buff_rcvd , MAX_BUFF+1);

            //Obtenemos la fecha y hora
            time_t now;
            struct tm *ts;
            struct tm *tsalida;
            now = time(0);
            ts = localtime(&now);
            strftime(HOUR, sizeof(HOUR), "%a %Y-%m-%d %H:%M:%S %Z", ts);

            //Creamos el monto
            int i, monto_decrementar;
            char monto[100];
            for (i = 2; i < MAX_BUFF; i = i + 1){
                monto[i-2] = buff_rcvd[i];
            }

            sscanf(monto, "%d", &monto_decrementar);
            TotalDisponible = TotalDisponible - monto_decrementar;

            fprintf(archivo_retiro, "Fecha y hora del retiro: %s, Monto: %s\n",HOUR,monto);
        }

        // Chequeamos si la accion es de deposito
        else if ( buff_rcvd[0] == 'd'){
            write(sock , buff_rcvd , MAX_BUFF+1);

            //Obtenemos la fecha y hora
            time_t now;
            struct tm *ts;
            struct tm *tsalida;
            now = time(0);
            ts = localtime(&now);
            strftime(HOUR, sizeof(HOUR), "%a %Y-%m-%d %H:%M:%S %Z", ts);

            //Creamos el monto
            int i, monto_incrementar;
            char monto[100];
            for (i = 2; i < MAX_BUFF; i = i + 1){
                monto[i-2] = buff_rcvd[i];
            }

            sscanf(monto, "%d", &monto_incrementar);
            TotalDisponible = TotalDisponible + monto_incrementar;

            fprintf(archivo_deposito, "Fecha y hora del deposito: %s, Monto: %s\n",HOUR,monto);
        }
    }

    fprintf(archivo_retiro, "Total Disponible: %d\n",TotalDisponible);
    fprintf(archivo_deposito, "Total Disponible: %d\n",TotalDisponible);

    // Verificaciones en caso que haya error al leer del socket
    if(read_size == 0)
    {
        printf("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    fclose(archivo_retiro);
    fclose(archivo_deposito);
    close(sock);

    ////////////////////////////////////////////////////////////////
}
