/**
* @title Client
*
* @author Mathieu De Valery 10-10193
* @author Marisela Del Valle 11-10267
*
* @description
*       Conexión TCP cliente-servidor. Crea el socket e intenta conectarse con el servidor.
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

#define MAX_BUFF 100


//Funcion principal
void main(int numArgs , char *args[])
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

    int sock, port, contador_retiros;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000], buffer[3];

    //Creacion del socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }

    port = atoi(args[4]);

    server.sin_addr.s_addr = inet_addr(args[2]);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    //Conexion al servidor
        int i;
        for (i=1;i<=3;i=i+1){
            if (i==3){
                if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0){
                    perror("connect failed. Error");
                    exit(1);
                }
                else{
                    break;
                }
            }
            else{
                if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0){
                    printf("No se pudo conectar al servidor. ");
                    printf("Pulse para volver a intentarlo...\n");
                    fflush(stdin);
                    getchar();
                }
                else{
                    break;
                }
            }
        }

    printf("¡BIENVENIDO!\n");
    contador_retiros = 0;

    //Mantiene la conexion con el servidor
    while(1)
    {

        char server_reply[2000],buffer[MAX_BUFF];
        memset(server_reply,0,sizeof(server_reply)/sizeof(int));
        memset(buffer,0,sizeof(buffer)/sizeof(int));

        // Verificamos si la operacion a realizar es deposito
        if (strcmp(args[6],"d") == 0){

        printf("Ingrese el monto a depositar : ");
        scanf("%s" , message);

        }

        // Verificamos si la operacion a realizar es retiro
        if (strcmp(args[6],"r") == 0){

            printf("Ingrese el monto a retirar : ");
            scanf("%s" , message);

            // Recibimos el monto, y verificamos que no pase de 3000
            int monto;

            monto = atoi(message);

            if (monto>3000){
                printf("Ingrese un monto menor o igual a 3000.");
                continue;
            }

            else{
                contador_retiros ++;

                if (contador_retiros > 3){
                    printf("Ha excedido el numero maximo de retiros.\n");
                    exit(1);
                }
            }
        }

        // Copiamos la accion en el buffer
        strcpy(buffer,args[6]);
        // Copiamos el monto en el buffer
        strcat(buffer," ");
        strcat(buffer,message);

        //Envio de datos
        if( send(sock , buffer , strlen(buffer)+1 , 0) < 0)
        {
            printf("Error al enviar monto");
            exit(1);
        }

        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            printf("recv failed");
            break;
        }

        printf("Recibido del buffer: %s \n",server_reply);

    }

    close(sock);

}
