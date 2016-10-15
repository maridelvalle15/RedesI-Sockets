/**
* @title Client
*
* @author Mathieu De Valery 10-10193
* @author Marisela Del Valle 11-10267
*
* @description
* 		Contains all the necesary thing to connect with a server. It create the socket,
*		and try to connect with the server(Before three times trying, print a message).
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>

#define MAXDATASIZE 100
/* El número máximo de datos en bytes */

void main(int numArgs, char *args[]) {

	// Verificamos los parametros de entrada.
    if (numArgs != 9){
        printf("Error de argumentos: numero equivocado de argumentos.\n");
        exit(1);
    }
    if ((strcmp(args[1],"-d") != 0) || (strcmp(args[3],"-p") != 0) || (strcmp(args[5],"-c") != 0) ||
        (strcmp(args[7],"-i") != 0)) {
        printf("\nError de argumentos: Argumentos en orden equivocado.\n");
        printf("Sintaxis: ");
        printf("bsb_cli -d <nombre_modulo_atencion> -p <puerto_bsb_svr> -c <op> -i <codigo_usuario>\n" );
        exit(1);
    }
    // Verificamos que la operacion sea de retiro (r) o deposito (d)
    if ((strcmp(args[6],"d") != 0) && (strcmp(args[6],"r") != 0)){
    	printf("\nError de argumentos: Solo son operaciones validas 'd' o 'r'.\n");
    	exit(1);
    }

    int fd, numbytes;
   /* ficheros descriptores */

   char buf[MAXDATASIZE];
   /* en donde es almacenará el texto recibido */

   struct hostent *he;
   /* estructura que recibirá información sobre el nodo remoto */

   struct sockaddr_in server;
   /* información sobre la dirección del servidor */

	int port; // Port

	port = atoi(args[4]);


	while (1) {

   if ((fd=socket(AF_INET, SOCK_STREAM, 0))==-1){
      /* llamada a socket() */
      perror("error en socket()\n");
      exit(-1);
   }

   server.sin_family = AF_INET;
   server.sin_port = htons(port);
   server.sin_addr.s_addr = inet_addr(args[2]);
   bzero(&(server.sin_zero),8);

   // Try connect with the server.
    int i;
    for (i = 0; i < 3; i++) {
      if (connect(fd, (struct sockaddr *)&server,sizeof(server)) == 0)
          break;

        if (i == 2) {
          perror("Couldn't connect with the server");
            exit(1);
        }
        else {
          printf("No se pudo conectar al servidor. ");
          printf("Pulse para volver a intentarlo...\n");
          fflush(stdin);
          getchar();
        }
    }

   if ((numbytes=recv(fd,buf,MAXDATASIZE,0)) == -1){
      /* llamada a recv() */
      perror(" error Error \n");
      exit(-1);
   }

   buf[numbytes]='\0';


   printf("Mensaje del Servidor: %s\n",buf);
   /* muestra el mensaje de bienvenida del servidor =) */


		close(fd);

	}
	 close(fd);
}
