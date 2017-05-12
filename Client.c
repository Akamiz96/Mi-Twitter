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

typedef void (*sighandler_t)(int);

Cliente user;
Tweet datos;

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

void follow(EnvioCliente envioCliente, EnvioServer envioServer, int server, int id){
  envioCliente.operacion = FOLLOW;
  envioCliente.cliente.pid = user.pid;
  envioCliente.cliente.id = id;
  if(write(server, &envioCliente , sizeof(EnvioCliente)) == -1)
  {
    perror("En escritura");
    exit(1);
  }
  if (read (server, &envioServer, sizeof(EnvioServer)) == -1) {
    perror("En lectura");
    exit(1);
  }
  switch(envioServer.respuesta){
    case EXITO:
    printf("Ahora sigue al usuario con id: %d\n", id);
    break;
    case FALLO:
    printf("Ya se encuentra siguiendo al usuario con el id: %d\n", id);
    break;
    case INVALIDO:
    printf("Id ingresado invalido\nSe ingreso el id: %d", id);
    break;
    default:
    printf("Error desconocido\nContacte al desarrollador");
  }
}

void unfollow(EnvioCliente envioCliente, EnvioServer envioServer, int server, int id){
  envioCliente.operacion = UNFOLLOW;
  envioCliente.cliente.pid = user.pid;
  envioCliente.cliente.id = id;
  if(write(server, &envioCliente , sizeof(EnvioCliente)) == -1)
  {
    perror("En escritura");
    exit(1);
  }
  if (read (server, &envioServer, sizeof(EnvioServer)) == -1) {
    perror("En lectura");
    exit(1);
  }
  switch(envioServer.respuesta){
    case EXITO:
    printf("Ahora no sigue al usuario con id: %d\n", id);
    break;
    case FALLO:
    printf("Ya no se encuentra siguiendo al usuario con el id: %d\n", id);
    break;
    case INVALIDO:
    printf("Id ingresado invalido\nSe ingreso el id: %d", id);
    break;
    default:
    printf("Error desconocido\nContacte al desarrollador");
  }
}

void registrar(EnvioCliente envioCliente, EnvioServer envioServer, Cliente user, int server){
  envioCliente.operacion = REGISTER;
  envioCliente.cliente = user;
  if(write(server, &envioCliente , sizeof(envioCliente)) == -1)
  {
    perror("En escritura");
    exit(1);
  }
  if (read (server, &envioServer, sizeof(envioServer)) == -1) {
    perror("En lectura");
    exit(1);
  }
  switch(envioServer.respuesta){
    case EXITO:
    printf("Registrado exitosamente.\n");
    break;
    case INVALIDO:
    printf("Registro no completado\nYa se encuentra registrado en el sistema.");
    break;
    default:
    printf("Error desconocido\nContacte al desarrollador");
  }
}
sighandler_t tweet_receive(void)
{
  // Se lee un mensaje por el segundo pipe.
  if(read(user.pipe_id, &datos, sizeof(Tweet) == -1))
  {
   perror("En lectura");
   exit(1);
  }
  printf("%s\n", datos.texto);
}


int main (int argc, char **argv)
{
  int  server, desconexion = 0, opcion, id;
  Tweet datos;
  EnvioCliente envioCliente;
  EnvioServer envioServer;

  mode_t fifo_mode = S_IRUSR | S_IWUSR;

  if( argc != 3 )
  {
    printf("Error\nDebe ser ejecutado de la forma: %s ID pipe_server \n",argv[0]);
    exit(1);
  }
  strcpy(user.pipe_cliente, argv[2]);
  // Se crea el pipe del cliente
  unlink(user.pipe_cliente);
  if (mkfifo (user.pipe_cliente, fifo_mode) == -1) {
    perror("pipe_cliente mkfifo");
    exit(1);
  }
  // Se abre el pipe cuyo nombre se recibe como argumento del main.
  server = abrir_pipe(argv[2], O_WRONLY);

  user.id = atoi(argv[1]);
  user.pid = getpid();
  strcpy(user.pipe_cliente, "cliente_");
  strcat(user.pipe_cliente, argv[1]+'\0');

  // se envia el nombre del pipe al otro proceso.
  printf("HOLA");
  registrar(envioCliente, envioServer, user, server);
  // Se abre el segundo pipe
  user.pipe_id = abrir_pipe(user.pipe_cliente, O_RDONLY);

  do
  {
    printf("Menu:\n\
            1. Follow\
            2. Unfollow\
            3. Tweet\
            4. Recuperar tweets\
            5. Desconexion" );
    scanf("%d\n", &opcion);
    switch (opcion) {
      case 1:
        //TODO Follow
        printf("Ingresar id del usuario a seguir: ");
        scanf("%d\n", &id);
        follow(envioCliente, envioServer, server, id);
        break;
      case 2:
        //TODO Unfollow
        printf("Ingresar id del usuario a no seguir mas: ");
        scanf("%d\n", &id);
        unfollow(envioCliente, envioServer, server, id);
        break;
      case 3:
        //TODO Tweet
        break;
      case 4:
        //TODO Recuperar
        break;
      case 5:
        //TODO Desconexion
        break;
      default:
        printf("Opcion Invalida\nIntente de nuevo");
    }
  } while(!desconexion);
  exit(0);
}
