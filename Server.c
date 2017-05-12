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
      printf(" Se volvera a intentar despues\n");
      sleep(5);
    } else
      abierto = 1;
  } while (!abierto);
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
  return id;
}

void registrar(int N, Cliente clientes[], EnvioCliente mensaje_cliente)
{
  Cliente aux = (mensaje_cliente.cliente);
  EnvioServer mensaje_server;
  if(aux.id <= N && aux.id >= 1 )
  {
    if(clientes[aux.id - 1].id != -1)
    {
      clientes[aux.id - 1] = mensaje_cliente.cliente;
      clientes[aux.id - 1].pipe_id = abrir_pipe(aux.pipe_cliente, O_WRONLY);
      mensaje_server.respuesta = EXITO;
    }
    else
      mensaje_server.respuesta = INVALIDO;
  }
  else
    mensaje_server.respuesta = INVALIDO;
  if(write(clientes[aux.id - 1].pipe_id, &mensaje_server, sizeof(EnvioServer)) == -1)
  {
    perror("En escritura");
    exit(1);
  }
}

void follow(int N, Cliente clientes[], int grafo[TAMUSR][TAMUSR], EnvioCliente mensaje_cliente)
{
  int id_cliente;
  Cliente aux = (mensaje_cliente.cliente);
  EnvioServer mensaje_server;
  if(aux.id <= N && aux.id >= 1 )
  {
    id_cliente = buscar_cliente_pid(N, clientes, aux.pid);
    if(id_cliente != -1)
    {
      if(grafo[id_cliente][aux.id] == 0)
      {
        grafo[id_cliente][aux.id] = 1;
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
  if(write(clientes[aux.id - 1].pipe_id, &mensaje_server, sizeof(EnvioServer)) == -1)
  {
    perror("En escritura");
    exit(1);
  }
}

void unfollow(int N, Cliente clientes[], int grafo[TAMUSR][TAMUSR], EnvioCliente mensaje_cliente)
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
  if(write(clientes[aux.id - 1].pipe_id, &mensaje_server, sizeof(EnvioServer)) == -1)
  {
    perror("En escritura");
    exit(1);
  }
}

int main (int argc, char **argv)
{
  int server,  pid, N, cuantos, res, creado = 0;
  int grafo[TAMUSR][TAMUSR], i, j;
  Tweet datos;
  FILE* file;
  char buffer[LINE];
  Cliente clientes[TAMUSR];
  EnvioCliente mensaje_cliente;
  EnvioServer mensaje_server;

  mode_t fifo_mode = S_IRUSR | S_IWUSR;
  if(argc != 5)
  {
    printf("Error\nDebe ser ejecutado de la forma: %s N relaciones modo pipeNom\n", argv[0]);
    exit(1);
  }
  N = atoi(argv[1]);
  for(i = 0; i < N; i++)
  {
    clientes[i].id = -1;
  }
  file = fopen(argv[2], "r");
  if(file == NULL)
  {
    printf("Error al abrir el archivo %s\n", argv[2]);
    return 1;
  }
  for(i = 0; (i < N) && (!feof(file)); i++)
  {
    fgets(buffer, LINE, file);
    for(j = 0; j < N && (buffer[j * 2] != '\0'); j++)
      grafo[i][j] = buffer[j * 2] == '1';
  }
  fclose ( file );
  // Creacion del pipe inicial, el que se recibe como argumento del main
  unlink(argv[4]);
  if (mkfifo (argv[4], fifo_mode) == -1) {
    perror("server mkfifo");
    exit(1);
  }

  // Abre el pipe. No olviden validar todas las llmadas al sistema.
  server = abrir_pipe(argv[4], O_RDONLY);
  // El otro proceso (nom1) le envia el nombre para el nuevo pipe y el pid.
  while (1)
  {
    if (read (server, &mensaje_cliente, sizeof(EnvioCliente)) == -1) {
      perror("En lectura");
      exit(1);
    }
    switch (mensaje_cliente.operacion) {
      case REGISTER:
        registrar(N, clientes, mensaje_cliente);
        break;
      case FOLLOW:
        follow(N, clientes, grafo, mensaje_cliente);
        break;
      case UNFOLLOW:
        unfollow(N, clientes, grafo, mensaje_cliente);
        break;
      case TWEET_C:
        //TODO Recuperar
        break;
      case RE_TWEETS:
        //TODO Desconexion
        break;
      default:
        printf("Opcion Invalida\nIntente de nuevo");
    }
    //TODO mandar señal
  }
  exit(0);
}
