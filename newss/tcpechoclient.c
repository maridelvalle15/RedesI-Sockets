/**
* @title Client
*
* @author Mathieu De Valery 10-10193
* @author Marisela Del Valle 11-10267
*
* @description
*       Conexión TCP clientr-servidor. Crea el socket e intenta conectarse con el servidor.
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

int main(int numArgs , char *args[])
{

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

    int sock, port;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];

    //Creacion del socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    port = atoi(args[3]);

    server.sin_addr.s_addr = inet_addr(args[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons( port );

    //Conexion al servidor
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    //Mantiene la conexion con el servidor
    while(1)
    {
        printf("Enter message : ");
        scanf("%s" , message);

        //Envio de datos
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }

        //Recepcion de respuesta del servidor
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            puts("recv failed");
            break;
        }

        puts("Server reply :");
        puts(server_reply);
    }

    close(sock);
    return 0;
}
