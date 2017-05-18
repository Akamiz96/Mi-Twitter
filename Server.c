/*
==================MI_TWITER========================
 */
//*****************************************************************
/*
Realizado por Pablo Ariza y Alejandro Castro
Proyecto Sistemas Operativos 2017-10
Compilacion: gcc Client.c -o cliente.exe
Observaciones: Para el correcto funcionamiento de este programa debe
                ser ejecutado antes de haber ejecutado el programa
                Client.c
  ->Temas principales: Comunicacion a traves de pipes y senales
 */
//*****************************************************************

//*****************************************************************
//LIBRERIAS INCLUIDAS
//*****************************************************************
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "tweet.h"

//****************************************************************************************************************************************************
//Función para la apertura del pipe segun los flags ingresados
//Parametros de entrada: pathname-> nombre que tendra el pipe,
//                       flags-> modificadores con los cuales se abrira el pipe
//Parametro que devuelve: entero indicando si el pipe fue abierto correctamente
//                        Si se abre correctamente-> id del pipe
//                        De lo contrario-> -1
//****************************************************************************************************************************************************
int abrir_pipe(const char* pathname, int flags)
{
  int abierto = 0, id_pipe;

  do {
    id_pipe = open(pathname, flags);
    if (id_pipe == -1) {
      perror(pathname);
      printf("Se volvera a intentar despues\n");
      sleep(5);
    } else
      abierto = 1;
  } while (abierto == 0);
  return id_pipe;
}

//****************************************************************************************************************************************************
//Función para obtener el nombre del archivo con el cual se guardaran los tweets pendientes
//Parametros de entrada: aux_id-> id numerico del usuario al cual se le creara el archivo
//                       archivo_tweet-> cadena de caracteres donde se almacenara el nombre creado
//Parametro que devuelve: NINGUNO
//****************************************************************************************************************************************************
void nombre_archivo(int aux_id, char* archivo_tweet)
{
  char final[LINE];
  sprintf(final, "%d.dat", aux_id);
  strcpy(archivo_tweet, "./tweet_pendientes/cliente_");
  strcat(archivo_tweet, final);
  printf("%s\n", archivo_tweet);
}

//****************************************************************************************************************************************************
//Función para buscar un usuario/cliente dado a traves del pid (process id) entre los clientes existentes
//Parametros de entrada: N-> cantidad de posibles clientes que se pueden conectar al sistema
//                       clientes-> estructura que contiene los clientes del sistema
//                       pid_cliente-> id del cliente a buscar
//Parametro que devuelve: id con el esta registrado dicho pid (process id)
//                        si no lo encuentra devuelve -1
//****************************************************************************************************************************************************
int buscar_cliente_pid(int N, Cliente clientes[], pid_t pid_cliente)
{
  int i, encontrado = 0, id = -1;

  for (i = 0; (i < N) && (!encontrado); i++)
  {
    if(clientes[i].pid == pid_cliente)
    {
      encontrado = 1;
      id = clientes[i].id;
    }
  }
  printf("id encontrado: %d\n", id);
  return id;
}

//****************************************************************************************************************************************************
//Función para registrar un usuario dentro del sistema
//Parametros de entrada: N-> cantidad de posibles clientes que se pueden conectar al sistema
//                       clientes-> estructura que contiene los clientes del sistema
//                       modo-> modo en el cual se encuentra subido el servidor (SINCRONO/ASINCRONO)
//                       mensaje_cliente-> mensaje enviado por el cliente para su registro en el sistema
//Parametro que devuelve: NINGUNO
//****************************************************************************************************************************************************
void registrar(int N, Cliente clientes[], Respuesta modo, EnvioCliente mensaje_cliente)
{
  char archivo_tweet[LINE];
  int num_tweets, i;
  FILE* file;
  Cliente aux = (mensaje_cliente.cliente);
  EnvioServer mensaje_server;

  if(aux.id <= N && aux.id >= 1 )
  {
    if(clientes[aux.id - 1].id == -1)
    {
      clientes[aux.id - 1] = mensaje_cliente.cliente;
      clientes[aux.id - 1].id = aux.id;
      clientes[aux.id - 1].pid = aux.pid;
      clientes[aux.id - 1].pipe_id = abrir_pipe(aux.pipe_cliente, O_WRONLY|O_NONBLOCK);
      mensaje_server.respuesta = EXITO;
    }
    else
      mensaje_server.respuesta = INVALIDO;
  }
  else
    mensaje_server.respuesta = INVALIDO;
  printf("write %d %s\n", clientes[aux.id - 1].pipe_id, aux.pipe_cliente);
  nombre_archivo(aux.id, archivo_tweet);
  file = fopen(archivo_tweet, "rb");
  if(file == NULL)
    kill(aux.pid, SIGUSR2);
  if(write(clientes[aux.id - 1].pipe_id, &mensaje_server, sizeof(EnvioServer)) == -1)
    perror("En escritura");
  else
    printf("escribio Registro\n");
  printf("nombre_archivo\n");
  /*nombre_archivo(aux.id, archivo_tweet);
  file = fopen(archivo_tweet, "rb");*/
  if(file != NULL)
  {
    while (!feof(file))
    {
      if(fread(&mensaje_server, sizeof(EnvioServer), 1, file) != 0)
      {
        if(write(clientes[aux.id - 1].pipe_id, &mensaje_server, sizeof(EnvioServer)) == -1)
          perror("En escritura");
      }
      else
        kill(aux.pid, SIGUSR2);
      printf("salio if registrar\n");
    }
    printf("salio while registrar\n");
    remove(archivo_tweet);
  }
  printf("salio de registrar\n");
}

//****************************************************************************************************************************************************
//Función para que un usuario empiece a seguir a un usuario
//Parametros de entrada: N-> cantidad de posibles clientes que se pueden conectar al sistema
//                       clientes-> estructura que contiene los clientes del sistema
//                       grafo-> estructura de datos (matriz de adyacencia) de las relaciones entre clientes
//                       mensaje_cliente-> mensaje enviado por el cliente para seguir a otro usuario
//Parametro que devuelve: NINGUNO
//****************************************************************************************************************************************************
void follow(int N, Cliente clientes[], int grafo[TAMUSR][TAMUSR],
            EnvioCliente mensaje_cliente)
{
  int id_cliente, escribir;
  Cliente aux = (mensaje_cliente.cliente);
  EnvioServer mensaje_server;

  if(aux.id <= N && aux.id >= 1 )
  {
    id_cliente = buscar_cliente_pid(N, clientes, aux.pid);
    if(id_cliente != -1)
    {
      if(grafo[id_cliente - 1][aux.id - 1] == 0)
      {
        grafo[id_cliente - 1][aux.id - 1] = 1;
        mensaje_server.respuesta = EXITO;
      }
      else
        mensaje_server.respuesta = FALLO;
    }
    else
      mensaje_server.respuesta = INVALIDO;
  }
  else
    mensaje_server.respuesta = INVALIDO;
  printf("\nESCRIBIendo\n");
  printf("%d %d %s\n", id_cliente - 1, clientes[id_cliente].pipe_id, clientes[id_cliente].pipe_cliente);
  escribir = write(clientes[id_cliente - 1].pipe_id, &mensaje_server, sizeof(EnvioServer));
  printf("\nESCRIBIII\n");
  if(escribir == -1)
    perror("En escritura");
  else
    printf("\nEscribi\n");
}

//****************************************************************************************************************************************************
//Función para que un usuario deje de seguir a un usuario
//Parametros de entrada: N-> cantidad de posibles clientes que se pueden conectar al sistema
//                       clientes-> estructura que contiene los clientes del sistema
//                       grafo-> estructura de datos (matriz de adyacencia) de las relaciones entre clientes
//                       mensaje_cliente-> mensaje enviado por el cliente para dejar de seguir a otro usuario
//Parametro que devuelve: NINGUNO
//****************************************************************************************************************************************************
void unfollow(int N, Cliente clientes[], int grafo[TAMUSR][TAMUSR],
              EnvioCliente mensaje_cliente)
{
  int id_cliente;
  Cliente aux = (mensaje_cliente.cliente);
  EnvioServer mensaje_server;

  if(aux.id <= N && aux.id >= 1 )
  {
    id_cliente = buscar_cliente_pid(N, clientes, aux.pid);
    if(id_cliente != -1)
    {
      if(grafo[id_cliente][aux.id] == 1)
      {
        grafo[id_cliente][aux.id] = 0;
        mensaje_server.respuesta = EXITO;
      }
      else
        mensaje_server.respuesta = FALLO;
    }
    else
      mensaje_server.respuesta = INVALIDO;
  }
  else
    mensaje_server.respuesta = INVALIDO;
  if(write(clientes[id_cliente - 1].pipe_id, &mensaje_server, sizeof(EnvioServer)) == -1)
    perror("En escritura");
}

//****************************************************************************************************************************************************
//Función para el control de un tweet enviado por un usuario
//Parametros de entrada: N-> cantidad de posibles clientes que se pueden conectar al sistema
//                       clientes-> estructura que contiene los clientes del sistema
//                       grafo-> estructura de datos (matriz de adyacencia) de las relaciones entre clientes
//                       modo-> modo en el cual se encuentra subido el servidor (SINCRONO/ASINCRONO)
//                       mensaje_cliente-> mensaje enviado por el cliente que contiene el tweet enviado
//Parametro que devuelve: NINGUNO
//****************************************************************************************************************************************************
void tweet(int N, Cliente clientes[], int grafo[TAMUSR][TAMUSR], Respuesta modo,
           EnvioCliente mensaje_cliente)
{
  int i, num_tweets;
  Cliente aux = (mensaje_cliente.cliente);
  EnvioServer mensaje_server;
  FILE* file;
  char archivo_tweet[LINE];

  if(aux.id <= N && aux.id >= 1 )
  {
    printf("IF\n");
    mensaje_server.respuesta = TWEET;
    mensaje_server.tweet = mensaje_cliente.tweet;
    if(write(clientes[aux.id - 1].pipe_id, &mensaje_server, sizeof(EnvioServer)) == -1)
      perror("En escritura");
    for(i = 0; i < N; i++)
    {
      if(grafo[i][aux.id - 1] == 1 && aux.id - 1 != i)
      {
        if(modo == ASINCRONO)
        {
          if(clientes[i].id != -1 ){
            if(write(clientes[i].pipe_id, &mensaje_server, sizeof(EnvioServer)) == -1)
              perror("En escritura");
            printf("Signal a: %d\n", aux.id);
            kill(aux.pid, SIGUSR1);
          }
          else
          {
            nombre_archivo(i + 1, archivo_tweet);
            file = fopen(archivo_tweet, "ab");
            fwrite(&mensaje_server, sizeof(EnvioServer), 1, file);
            fclose(file);
          }
        }
        else
        {
          nombre_archivo(i + 1, archivo_tweet);
          file = fopen(archivo_tweet, "ab");
          fwrite(&mensaje_server, sizeof(EnvioServer), 1, file);
          fclose(file);
        }
      }
    }
  }
  else
    mensaje_server.respuesta = INVALIDO;
}

//****************************************************************************************************************************************************
//Función para que un usuario pueda recuperar tweets que se encuentren en el sistema para dicho usuario
//UNICAMENTE funciona en modo SINCRONO
//Parametros de entrada: clientes-> estructura que contiene los clientes del sistema
//                       mensaje_cliente-> mensaje enviado por el cliente que contiene el tweet enviado
//                       modo-> modo en el cual se encuentra subido el servidor (SINCRONO/ASINCRONO)
//Parametro que devuelve: NINGUNO
//****************************************************************************************************************************************************
void recuperar_tweets(Cliente clientes[], EnvioCliente mensaje_cliente, Respuesta modo)
{
  char archivo_tweet[LINE];
  FILE* file;
  Cliente aux = (mensaje_cliente.cliente);
  EnvioServer mensaje_server;

  if(modo == SINCRONO)
  {
    nombre_archivo(aux.id, archivo_tweet);
    file = fopen(archivo_tweet, "rb");
    if(file != NULL);
    {
      while (!feof(file))
      {
        if(fread(&mensaje_server, sizeof(EnvioServer), 1, file) != 0)
        {
          mensaje_server.respuesta = TWEET;
          if(write(clientes[aux.id - 1].pipe_id, &mensaje_server, sizeof(EnvioServer)) == -1)
              perror("En escritura");
        }
        else
          kill(aux.pid, SIGUSR2);
      }
      remove(archivo_tweet);
    }
  }
  else
  {
    mensaje_server.respuesta = ASINCRONO;
    kill(aux.pid, SIGUSR2);
    if(write(clientes[aux.id - 1].pipe_id, &mensaje_server, sizeof(EnvioServer)) == -1)
      perror("En escritura");
  }
}

//****************************************************************************************************************************************************
//Función para que un usuario pueda desconectarse del sistema exitosamente
//Parametros de entrada: clientes-> estructura que contiene los clientes del sistema
//                       mensaje_cliente-> mensaje enviado por el cliente que contiene el tweet enviado
//Parametro que devuelve: NINGUNO
//****************************************************************************************************************************************************
void desconexion(Cliente clientes[], EnvioCliente mensaje_cliente)
{
  Cliente aux = (mensaje_cliente.cliente);
  EnvioServer mensaje_server;
  int escribio;

  clientes[aux.id - 1].id = -1;
  mensaje_server.respuesta = EXITO;
  printf("write %d %s\n", clientes[aux.id - 1].pipe_id, aux.pipe_cliente);
  if(write(clientes[aux.id - 1].pipe_id, &mensaje_server, sizeof(EnvioServer)) != -1)
  {
    printf("hola\n");
    close(clientes[aux.id - 1].pipe_id);
  }
  else
    perror("En escritura");
}

/*
/*********************************************************************************************************
//PROGRAMA PRINCIPAL
//*********************************************************************************************************
Parametros para su ejecucion:
  ->N: Numero de clientes que el sistema debe controlar
      (No todos estan conectados al mismo tiempo)
  ->Relaciones: Archivo donde se encuentran las relaciones
      descritas entre los clientes
  ->modo: sincrono o asincrono
  ->pipeNom: nombre del pipe de comunicacion entre los clientes y el servidor
*Los parametros deben ser insertado en este orden*

 */

int main (int argc, char **argv)
{
  //Declaracion de variables necesarias
  int server,  pid, N, cuantos, res, creado = 0;
  int grafo[TAMUSR][TAMUSR], i, j, line = LINE;
  Tweet datos;
  FILE* file;
  char *buffer;
  Cliente clientes[TAMUSR];
  EnvioCliente mensaje_cliente;
  EnvioServer mensaje_server;
  Respuesta modo;

  //Definicion del tipo de lectura o escritura del pipe de comunicacion
  mode_t fifo_mode = S_IRUSR | S_IWUSR;

  //Validacion de parametros del Main
  if(argc != 5)
  {
    printf("Error\nDebe ser ejecutado de la forma: %s N relaciones modo pipeNom\n",
           argv[0]);
    exit(1);
  }

  //Validacion de la cantidad de clientes
  N = atoi(argv[1]);
  if(N < 1)
  {
    printf("Numero de clientes invalido debe ser mayor a 0\n");
    exit(1);
  }

  /*
    Inicializacion del arreglo para control de los distintos clientes
    -1 -> No se encuentra conectado
     1 -> Se encuentra conectado.
    *Tipo del arreglo Cliente*
  */
  for(i = 0; i < N; i++)
  {
    clientes[i].id = -1;
  }

  //Apertura del archivo para lectura de las relaciones entre clientes
  file = fopen(argv[2], "r");

  //Validacion de la apertura del archivo de relaciones
  if(file == NULL)
  {
    printf("Error al abrir el archivo %s\n", argv[2]);
    return 1;
  }

  /*
    Lectura del archivo de relaciones
    Matriz de relaciones entre clientes
    (id del cliente - 1 es la posicion de la matriz)
   */
  for(i = 0; i < N; i++)
  {
    getline(&buffer, (size_t*)&line, file);
    for(j = 0; j < N && (buffer[j * 2] != '\0'); j++){
      grafo[i][j] = (buffer[j * 2] == '1');
    }
  }

  //Cerrar archivo de relaciones
  fclose ( file );

  //Seleccion del modo de funcionamiento para el servidor
  //Validacion de valores "sincrono" y "asincrono"
  if(strcasecmp(argv[3],"sincrono") == 0)
    modo = SINCRONO;
  else{
    if(strcasecmp(argv[3],"asincrono") == 0)
      modo = ASINCRONO;
    else
    {
      printf("Modo invalido debe ser sincrono o asincrono\n");
      exit(1);
    }
  }

  // Creacion del pipe inicial, el que se recibe como argumento del main
  unlink(argv[4]);
  //Validacion de correcta creacion del pipe de comunicacion
  if (mkfifo (argv[4], fifo_mode) == -1) {
    perror("server mkfifo");
    exit(1);
  }

  // Apertura del pipe de comunicacion
  server = abrir_pipe(argv[4], O_RDONLY);

  //Ciclo infinito para la atencion de solicitudes de los clientes
  while (1)
  {

    //Lectura de datos del pipe de comunicacion entre clientes y servidor
    printf("read while true\n");
    if (read (server, &mensaje_cliente, sizeof(EnvioCliente)) == -1) {
      perror("En lectura");
      exit(1);
    }
    printf("lei\n");

    //Validacion de la opcion correcta dependiendo del mensaje recibido por el servidor
    switch (mensaje_cliente.operacion) {
      //Caso para el REGISTRO de un cliente
      case REGISTER:
        registrar(N, clientes, modo, mensaje_cliente);
        break;
      //Caso para FOLLOW
      case FOLLOW:
        follow(N, clientes, grafo, mensaje_cliente);
        break;
      //Caso para UNFOLLOW
      case UNFOLLOW:
        unfollow(N, clientes, grafo, mensaje_cliente);
        break;
      case TWEET_C:
        //TODO realizar un tweet
        printf("TWEET\n");
        tweet(N, clientes, grafo, modo, mensaje_cliente);
        break;
      case RE_TWEETS:
        recuperar_tweets(clientes, mensaje_cliente, modo);
        break;
      case DESCONEXION:
        desconexion(clientes, mensaje_cliente);
        break;
    }
    //TODO mandar señal
  }
  exit(0);
}
