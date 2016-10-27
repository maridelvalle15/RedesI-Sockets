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

// Tamano maximo del buffer
#define MAX_BUFF 100


//Funcion principal
void main(int numArgs , char *args[])
{

    // Verificamos los parametros de entrada.
    if (numArgs != 9){
        printf("Error de argumentos: numero equivocado de argumentos.\n");
        printf("Sintaxis: ");
        printf("bsb_cli -d <nombre_modulo_atencion> -p <puerto_bsb_svr> -c <op> -i <codigo_usuario>\n" );
        exit(1);
    }

    // Variables para guardar los flags ingresados por consola
    char *ip, *puerto, *operacion, *id_usuario;
    int i;

    // Los argumentos pueden ser ingresados en desorden
    // Dependiendo del flag, los guardamos en una variable
    for (i = 1; i <= 7; i = i + 2){
        if (strcmp(args[i],"-d") == 0){
            ip = strdup(args[i+1]);
        }
        else if (strcmp(args[i],"-p") == 0){
            puerto = strdup(args[i+1]);
        }
        else if (strcmp(args[i],"-c") == 0){
            operacion = strdup(args[i+1]);
        }
        else if (strcmp(args[i],"-i") == 0){
            id_usuario = strdup(args[i+1]);
        }
        else{
            printf("Error de argumentos: Argumentos equivocados.\n");
            printf("Sintaxis: ");
            printf("bsb_cli -d <nombre_modulo_atencion> -p <puerto_bsb_svr> -c <op> -i <codigo_usuario>\n" );
            exit(1);
        }
    }

    // Verificamos que la operacion sea de retiro (r) o deposito (d)
    if ((strcmp(operacion,"d") != 0) && (strcmp(operacion,"r") != 0)){
        printf("\nError de argumentos: Solo son operaciones validas 'd' o 'r'.\n");
        exit(1);
    }

    int sock, port, contador_retiros;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];

    //Creacion del socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Fallo en la creacion del socket");
    }

    port = atoi(puerto);

    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    //El cliente tiene 3 intentos para conectarse al servidor
        for (i=1;i<=3;i=i+1){
            if (i==3){
                if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0){
                    perror("Error al conectar. Error");
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

    char monto_restante[100];
    int monto_recalculado;
    if( recv(sock , monto_restante , 100 , 0) < 0)
        {
            printf("Fallo al recibir monto restante");
            exit(1);
        }
    monto_recalculado = atoi(monto_restante);
    // El cliente solo puede realizar 3 retiros por conexion
    contador_retiros = 0;

    //Mantiene la conexion con el servidor
    while(1)
    {
        char server_reply[2000],buffer[MAX_BUFF];
        memset(server_reply,0,sizeof(server_reply)/sizeof(int));
        memset(buffer,0,sizeof(buffer)/sizeof(int));
        char opcion_s[1], nuevo_codigo[100];
        int opcion;
        printf("\n");
        printf("Ingrese el numero correspondiente a la accion deseada: \n");
        printf("1. Retiro/Deposito \n");
        printf("2. Salir \n");
        printf("\n");
        scanf("%s" , opcion_s);
        opcion = atoi(opcion_s);
        if ((opcion != 1) && (opcion != 2)){
            printf("\n");
            printf("Opción no valida. Intente de nuevo.\n");
            continue;
        }
        else if (opcion == 2){
            exit(0);
        }
        else if (opcion == 1){
            // Verificamos si la operacion a realizar es deposito
            if (strcmp(operacion,"d") == 0){

                printf("Ingrese el monto a depositar : ");
                scanf("%s" , message);
                if (atoi(message) <= 0){
                    printf("\n");
                    printf("Monto ingresado no valido. Intente de nuevo.\n");
                    printf("\n");
                    continue;
                }
                printf("Ingrese nuevamente su codigo de usuario : ");
                scanf("%s", nuevo_codigo);
                if (strcmp(id_usuario, nuevo_codigo) != 0){
                    printf("¡¡ALERTA DE SEGURIDAD!! Los identificadores del usuario no coinciden.\n");
                    printf("El sistema se cerrará automáticamente.\n");
                    printf("\n");
                    exit(1);
                }
            }

            // Verificamos si la operacion a realizar es retiro
            if (strcmp(operacion,"r") == 0){
                // Alertamos al usuario que quedan menos de 5000 para retirar
                if (monto_recalculado < 5000){
                    printf("Alerta: quedan menos de 5000 disponibles para retirar.\n");
                    printf("\n");
                }

                printf("Ingrese el monto a retirar : ");
                scanf("%s" , message);
                if (atoi(message) <= 0){
                    printf("\n");
                    printf("Monto ingresado no valido. Intente de nuevo.\n");
                    printf("\n");
                    continue;
                }
                printf("Ingrese nuevamente su codigo de usuario : ");
                scanf("%s", nuevo_codigo);
                if (strcmp(id_usuario, nuevo_codigo) != 0){
                    printf("\n");
                    printf("¡¡ALERTA DE SEGURIDAD!! Los identificadores del usuario no coinciden.\n");
                    printf("El sistema se cerrará automáticamente.\n");
                    printf("\n");
                    exit(1);
                }

                // Recibimos el monto, y verificamos que no pase de 3000
                int monto;

                monto = atoi(message);

                // No se puede hacer retiro de un monto mayor a 3000
                if (monto>3000){
                    printf("\n");
                    printf("Ingrese un monto menor o igual a 3000.\n");
                    continue;
                }

                else{
                    // Solo se pueden hacer 3 retiros por conexion
                    contador_retiros ++;

                    if (contador_retiros > 3){
                        printf("Ha excedido el numero maximo de retiros.\n");
                        exit(1);
                    }

                    // No se puede retirar un monto mayor al disponible en el cajero
                    if (monto>monto_recalculado){
                        printf("\n");
                        printf("Dinero no disponible.\n");
                        printf("El monto ingresado no puede ser retirado. Intente de nuevo.\n");
                        continue;
                    }
                }
            }

            // Copiamos la accion en el buffer
            strcpy(buffer,operacion);
            // Copiamos el monto en el buffer
            strcat(buffer," ");
            strcat(buffer,message);
            // Copiamos el codigo del usuario en el buffer
            strcat(buffer," ");
            strcat(buffer,id_usuario);

            //Envio de datos
            if( send(sock , buffer , strlen(buffer)+1 , 0) < 0)
            {
                printf("Error al enviar datos");
                exit(1);
            }

            if( recv(sock , server_reply , 2000 , 0) < 0)
            {
                printf("Fallo en recv");
                break;
            }

            char *operacion_realizada;
             if (strcmp(operacion,"r") == 0){
                monto_recalculado = monto_recalculado - atoi(message);
                operacion_realizada = strdup("Retiro");
             }
             else{
                operacion_realizada = strdup("Deposito");
             }

            // Se imprime ticket con verificacion de la operacion
            printf("\n");
            printf("\n");
            printf("**********************************\n");
            printf("Transaccion exitosa\n");
            printf("Datos de su transaccion\n");
            printf("\n");
            printf(" ---------------------------- \n");
            printf("|                             \n");
            printf("|                             \n");
            printf("| Hora: %s \n",server_reply);
            printf("| Operación realizada: %s \n",operacion_realizada);
            printf("| Monto: %s \n",message);
            printf("| Codigo de usuario: %s \n",id_usuario);
            printf("|                             \n");
            printf("|                             \n");
            printf(" ---------------------------- \n");
            printf("**********************************\n");
            printf("\n");
            printf("\n");
        }

    }

    close(sock);

}
