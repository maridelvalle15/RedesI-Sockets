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
#include  <signal.h>

// Tamano maximo del buffer
#define MAX_BUFF 100

// Para crear hilos
void *connection_handler(void *);
// Para captar ctrl+C
void INThandler(int);
// Procesa los datos del buffer
void procesar_buffer(char bf[],char h[], char *bd, char *br);

// Total disponible del cajero
int TotalDisponible = 5000;
// Nombres de los archivos de deposito y retiro
char *b_deposito, *b_retiro;

// Estructura para pasar los datos requeridos a los hilos
struct Datos {
    char *nombre_entrada;   //nombre del archivo de entreda
    char *nombre_salida;    //nombre del archivo de salida
    int socket;             //socket
    FILE *archivo_deposito; //archivo de deposito
    FILE *archivo_retiro;   //archivo de retiro
};


// Funcion principal
// Recibe el numero de argumentos y los argumentos ingresados por consola
void main(int numArgs , char *args[]){

    // Verificamos los parametros de entrada.
    if (numArgs != 7){
        printf("Error de argumentos: numero equivocado de argumentos.\n");
        printf("Sintaxis: \n");
        printf("bsb_svr -l <puerto_bsb_svr> -i <bitácora_deposito> -o <bitácora_retiro>\n" );
        exit(1);
    }

    // Variables para guardar los flags ingresados por consola
    char *puerto;
    int i;

    // Los argumentos pueden ser ingresados en desorden
    // Dependiendo del flag, los guardamos en una variable
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
            printf("Sintaxis: \n");
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
        printf("Fallo en la creacion del socket");
    }

    port = atoi(puerto);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {

        perror("Fallo en bind. Error");
        exit(1);
    }

    //Listen
    listen(socket_desc , 3);

    //Acepta la conexion
    puts("Esperando conexiones entrantes...");
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

    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        printf("Se obtuvo una conexión desde %s\n",
             inet_ntoa(server.sin_addr) );

        // Señal que detecta si se ejecuta ctrl+C
        signal(SIGINT, INThandler);

        // Creamos los hilos para multiples conexiones
        pthread_t sniffer_thread;

        // Creamos la estructura para pasarle los datos a los hilos
        struct Datos datos;
        datos.socket = client_sock;
        datos.nombre_entrada = b_deposito;
        datos.nombre_salida = b_retiro;
        datos.archivo_deposito = archivo_deposito;
        datos.archivo_retiro = archivo_retiro;

        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void *)&datos) < 0)
        {
            perror("No fue posible crear el hilo");
            exit(1);
        }

    }

    if (client_sock < 0)
    {
        perror("Fallo accept");
        exit(1);
    }
}


// Procedimiento para captar ctrl+C y realizar el cierre de caja
void  INThandler(int sig)
{
     char  c;

     signal(sig, SIG_IGN);

     // Archivos de logs de deposito y retiro
    FILE *archivo_deposito, *archivo_retiro;

    archivo_deposito = fopen(b_deposito,"a");
    archivo_retiro = fopen(b_retiro,"a");

    // Al final de la conexion, se escribe en cada archivo el total disponible actualizado
    fprintf(archivo_retiro, "Total Disponible: %d\n",TotalDisponible);
    fprintf(archivo_deposito, "Total Disponible: %d\n",TotalDisponible);

    fclose(archivo_retiro);
    fclose(archivo_deposito);

    printf("\n");
    printf("Cierre de caja realizado.\n");
    printf("\n");

    exit(0);

}



// Manejo de conexion con varios clientes a traves de hilos
// Recibe una estructura con los datos necesarios para realizar operaciones
void *connection_handler(void *datos){

    int read_size, *monto;
    // Reasignamos los datos recibidos en la estructura a nuevas variables
    struct Datos* mis_datos = (struct Datos*)datos;
    int sock = mis_datos->socket;
    char *b_deposito = mis_datos->nombre_entrada;
    char *b_retiro = mis_datos->nombre_salida;
    FILE *archivo_deposito = mis_datos->archivo_deposito;
    FILE *archivo_retiro = mis_datos->archivo_retiro;

    char buff_rcvd[MAX_BUFF], buff_snt[MAX_BUFF], HOUR[30], monto_restante[100];

    archivo_deposito = fopen(b_deposito,"a");
    archivo_retiro = fopen(b_retiro,"a");

    //Enviamos y recibimos mensajes del cliente
    sprintf(monto_restante, "%d", TotalDisponible);
    write(sock, monto_restante, 100);

    char signal[100];

    while(recv(sock , signal , 100 , 0) > 0)
    {
        // Debe aparecer un mensaje en el servidor indicando que
        // quedan 5000 disponibles
        if (TotalDisponible < 5000){
            puts("Total disponible menor a 5000");
        }

        sprintf(monto_restante, "%d", TotalDisponible);
        write(sock, monto_restante, 100);

        if (read_size = recv(sock , buff_rcvd , MAX_BUFF , 0) < 0 ){
            printf("Fallo en recv datos");
            exit(1);
        }

        if (strcmp(buff_rcvd,"restart") == 0){
            continue;
        }

        //Obtenemos la fecha y hora
        time_t now;
        struct tm *ts;
        struct tm *tsalida;
        now = time(0);
        ts = localtime(&now);
        strftime(HOUR, sizeof(HOUR), "%a %Y-%m-%d %H:%M:%S %Z", ts);

        // Leemos del buffer
        procesar_buffer(buff_rcvd, HOUR, b_deposito, b_retiro);

        // Enviamos respuesta al usuario de su solicitud
        memset(buff_snt ,0, sizeof(buff_snt)/sizeof(int));
        // Copiamos la accion en el buffer
        strcpy(buff_snt, HOUR);
        write(sock , buff_snt , MAX_BUFF+1);

    }

    // Verificaciones en caso que haya error al leer del socket
    if(read_size == 0)
    {
        printf("Se cerró una conexión\n");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("Fallo al recibir datos del cliente\n");
    }

    close(sock);

}

// Procedimiento que procesa los datos recibidos en el buffer
    /* Parametros:
        buff_rcvd: datos recibidos en el buffer
        HOUR: hora de la transaccion
        b_deposito: nombre del archivo de deposito
        b_retiro: nombre del archivo de retiro
    */
void procesar_buffer(char buff_rcvd[], char HOUR[], char *b_deposito, char *b_retiro){
     // Archivos de logs de deposito y retiro
    FILE *archivo_deposito, *archivo_retiro;

    archivo_deposito = fopen(b_deposito,"a");
    archivo_retiro = fopen(b_retiro,"a");

    // Variables para la lectura del buffer
    int i, j, monto_modificar, contador_espacios;
    contador_espacios = 1;
    char monto[100];
    char id_usuario[100];

    // Creamos el monto
    memset(monto,0,sizeof(monto)/sizeof(int));
    for (i = 2; i < MAX_BUFF; i = i + 1){
        if (buff_rcvd[i] == ' '){
            contador_espacios ++;
        }
        else{
            if (contador_espacios == 1){
                monto[i-2] = buff_rcvd[i];
            }
            else{
                j = i;
                break;
            }
        }
    }

    // Creamos el codigo usuario
    memset(id_usuario,0,sizeof(id_usuario)/sizeof(int));
    for (i = j; i < MAX_BUFF; i = i + 1){
        id_usuario[i-j] = buff_rcvd[i];
    }

    // Convertimos el monto de string a entero
    sscanf(monto, "%d", &monto_modificar);

    // Decrementamos el total disponible
    // Si es retiro, decrementamos
    if (buff_rcvd[0] == 'r'){
        TotalDisponible = TotalDisponible - monto_modificar;
    }
    // Si es deposito, incrementamos
    else if (buff_rcvd[0] == 'd'){
        TotalDisponible = TotalDisponible + monto_modificar;
    }

    // Escribimos la informacion pertinente en el archivo
    // Si es retiro
    if (buff_rcvd[0] == 'r'){
    fprintf(archivo_retiro, "Fecha y hora del retiro: %s, Monto: %s, Codigo de usuario: %s\n", HOUR, monto, id_usuario);
    fclose(archivo_retiro);
    }
    // Si es deposito
    else if (buff_rcvd[0] == 'd'){
        fprintf(archivo_deposito, "Fecha y hora del deposito: %s, Monto: %s, Codigo de usuario: %s\n", HOUR, monto, id_usuario);
    fclose(archivo_deposito);
    }
}
