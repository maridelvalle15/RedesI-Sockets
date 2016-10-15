/**
* @title Server
*
* @author Mathieu De Valery 10-10193
* @author Marisela Del Valle 11-10267
*
* @description
*			Creates the server and creates a new socket to connect clients.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>


pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

void *connection_Threads(void *arg){

		pthread_mutex_lock(&mymutex);
		// Cast the args.
		int newsock = (*(int *)(arg));

		char buffer[128], *token;
		token = (char *)malloc(128*sizeof(char));

		// Initialize the buffer.
	    bzero(buffer,128);
        // Receive information from the socket.
        // Seat information.

        if ((recv(newsock,buffer,127,0)) == -1) {
	        perror("Couldn't read from socket");
	        exit(1);
	    }

	    // Initialize the buffer.
	    bzero(buffer,128);

	    // Send into the socket.
	    if (send(newsock,buffer,127,0) == -1) {
	    	perror("Couldn't send into the socket");
	    }

    	close(newsock);
    	pthread_mutex_unlock(&mymutex);
    	pthread_exit(NULL);

}

void main(int numArgs, char *args[]){

	// Verificamos los parametros de entrada.

    if (numArgs != 7){
        printf("Error de argumentos: numero equivocado de argumentos.\n");
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


	int port, sock, newsock, thread_socket; // Port and Sockets.
	struct sockaddr_in clientaddr, serveraddr; // Client and server address.
	char buffer[128]; // Socket's buffer
	int i,j,bytes;
	pthread_t pthread_connection;

	port = atoi(args[2]);


	// Se crea the socket.
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("No se pudo crear el socket");
		exit(1);
	}

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(port); // Assing a port to socket.

	// Asociate a port and a location to socket.
	if (bind(sock, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr)) == -1) {
        perror("Couldn't run bind");
        exit(1);
    }
    // Set socket to listen.
    if (listen(sock, 5) == -1) {
        perror("Couldn't set socket to listen");
        exit(1);
    }

    while(1) {

    	// Look for the connection.
    	socklen_t lenaddr =  sizeof(clientaddr);
    	if ((thread_socket = accept(sock, (struct sockaddr *)&clientaddr,&lenaddr)) == -1) {
            perror("Couldn't run accept");
            continue;
        }

        printf("server: connection from %s\n",inet_ntoa(clientaddr.sin_addr));



        // Create a new thread.
      	int rc = pthread_create(&pthread_connection,NULL,connection_Threads,(void *)&thread_socket);
		if (rc) {
			printf("Error creating threads.\n");
			exit(-1);
		}
	}


}

