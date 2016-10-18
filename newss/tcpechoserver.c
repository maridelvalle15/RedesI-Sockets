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

//Para crear hilos
void *connection_handler(void *);


/*
    Funcion principal
*/
void main(int numArgs , char *args[]){

    // Verificamos los parametros de entrada.

    if (numArgs != 7){
        printf("Error de argumentos: numero equivocado de argumentos.\n");
        printf("Sintaxis: ");
        printf("bsb_svr -l <puerto_bsb_svr> -i <bitácora_deposito> -o <bitácora_retiro>\n" );
        exit(1);
    }
    if ((strcmp(args[1],"-l") != 0) || (strcmp(args[3],"-i") != 0) || (strcmp(args[5],"-o") != 0) ) {
        printf("\nError de argumentos: Argumentos en orden equivocado.\n");
        printf("Sintaxis: ");
        printf("bsb_svr -l <puerto_bsb_svr> -i <bitácora_deposito> -o <bitácora_retiro>\n" );
        exit(1);
    }
    // Verificamos que los archivos de entrada y salida no se llamen igual
    if ((strcmp(args[4],args[6]) == 0)){
        printf("Error de argumentos: Los archivos de entrada y salida no deben llamarse igual.\n" );
        exit(1);
    }

    int socket_desc , client_sock , c , *new_sock, port;
    struct sockaddr_in server , client;

    //Creacion del socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    port = atoi(args[2]);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {

        perror("bind failed. Error");
        exit(1);
    }
    puts("bind done");

    //Listen
    listen(socket_desc , 3);

    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);


    //Acepta la conexion
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");

        // Creamos los hilos para multiples conexiones
        pthread_t sniffer_thread;

        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void *)&client_sock) < 0)
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


/*
    Manejo de conexion con varios clientes a traves de hilos
*/
void *connection_handler(void *socket_desc){

    //Informacion del socket
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];

    //Enviamos y recibimos mensajes del cliente

    message = "Entrada recibida \n";

    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {

        write(sock , client_message , strlen(client_message));
    }

    if(read_size == 0)
    {
        printf("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    close(sock);
}
