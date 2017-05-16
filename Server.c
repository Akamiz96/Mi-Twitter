/*
Autor: M. Curiel
funcion: Ilustra la creaci'on de pipe nominales.
Nota: todas las llamadas al sistema no estan validadas.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "tweet.h"

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

void registrar(int N, Cliente clientes[], EnvioCliente mensaje_cliente)
{
  Cliente aux = (mensaje_cliente.cliente);
  EnvioServer mensaje_server;
  if(aux.id <= N && aux.id >= 1 )
  {
    if(clientes[aux.id - 1].id == -1)
    {
      clientes[aux.id - 1] = mensaje_cliente.cliente;
      clientes[aux.id - 1].id = aux.id;
      clientes[aux.id - 1].pipe_id = abrir_pipe(aux.pipe_cliente, O_WRONLY);
      mensaje_server.respuesta = EXITO;
    }
    else
      mensaje_server.respuesta = INVALIDO;
  }
  else
    mensaje_server.respuesta = INVALIDO;
  printf("write %d %s\n", clientes[aux.id - 1].pipe_id, aux.pipe_cliente);
  if(write(clientes[aux.id - 1].pipe_id, &mensaje_server, sizeof(EnvioServer)) == -1)
    perror("En escritura");
  else
    printf("escribio\n");
}

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

void tweet(int N, Cliente clientes[], int grafo[TAMUSR][TAMUSR], Respuesta modo,
           EnvioCliente mensaje_cliente)
{
  int i, num_tweets;
  Cliente aux = (mensaje_cliente.cliente);
  EnvioServer mensaje_server;
  FILE* file;
  char archivo_tweet[MAXCHAR];

  if(aux.id <= N && aux.id >= 1 )
  {
    for(i = 0; i < N; i++)
    {
      mensaje_server.respuesta = TWEET;
      mensaje_server.tweet = mensaje_cliente.tweet;
      if(grafo[i][aux.id - 1] == 1)
      {
        if(modo == ASINCRONO)
        {
          if(clientes[i].id != -1 )
            if(write(clientes[i].pipe_id, &mensaje_server, sizeof(EnvioServer)) == -1)
              perror("En escritura");
            else
              kill(aux.pid, SIGUSR1);
          else
          {
            strcpy(archivo_tweet, "./tweet_pendientes/cliente_");
            strcat(archivo_tweet, aux.id + ".dat\0");
            file = fopen(archivo_tweet, "+rb");
            if(file == NULL)
              printf("Error al abrir el archivo %s\n", archivo_tweet);
            else
            {
              if(fread(&num_tweets, sizeof(int), 1, file) == 0)
                num_tweets = 0;
              else
                num_tweets++;
              fwrite(&num_tweets, sizeof(int), 1, file);
              fclose(file);
              file = fopen(archivo_tweet, "ab");
              fwrite(&mensaje_server, sizeof(EnvioServer), 1, file);
            }
          }
        }
        else
        {
          strcpy(archivo_tweet, "./tweet_pendientes/cliente_");
          strcat(archivo_tweet, aux.id + ".dat\0");
          file = fopen(archivo_tweet, "ab");
          if(file == NULL)
            printf("Error al abrir el archivo %s\n", archivo_tweet);
          else
          {
            if(fread(&num_tweets, sizeof(int), 1, file) == 0)
              num_tweets = 0;
            else
              num_tweets++;
            fwrite(&num_tweets, sizeof(int), 1, file);
            fclose(file);
            file = fopen(archivo_tweet, "ab");
            fwrite(&mensaje_server, sizeof(EnvioServer), 1, file);
            kill(aux.pid, SIGUSR2);
          }
        }
      }
    }
  }
  else
    mensaje_server.respuesta = INVALIDO;
}

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
    //sleep(30);
  }
  else
  {
    perror("En escritura");
  }
}

/*

Main ejecutable del programa servidor
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
  if(strcasecmp(argv[3],"sincrono"))
    modo = SINCRONO;
  else{
    if(strcasecmp(argv[3],"asincrono"))
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
        registrar(N, clientes, mensaje_cliente);
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
        break;
      case RE_TWEETS:
        //TODO recuperar tweet
        break;
      case DESCONEXION:
        //TODO Desconexion
        desconexion(clientes, mensaje_cliente);
        break;
    }
    //TODO mandar señal
  }
  exit(0);
}
