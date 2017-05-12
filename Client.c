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

cliente user;
int message;

sighandler_t tweet_recive(void)
{
  message = read(user.pipe_id, datos, sizeof(tweet));
  printf("%s\n", datos.texto);
}

int main (int argc, char **argv)
{
  int  server, creado = 0, desconexion = 0, opcion;
  tweet datos;

  mode_t fifo_mode = S_IRUSR | S_IWUSR;

  if( argc != 3 )
  {
    printf("Error\nDebe ser ejecutado de la forma: %s ID pipe_server \n",argv[0]);
    exit(1);
  }

  // Se abre el pipe cuyo nombre se recibe como argumento del main.
  do {
    server = open(argv[2], O_WRONLY);
    if (server == -1) {
      perror("pipe_server");
      printf(" Se volvera a intentar despues\n");
      sleep(5);
    } else
      creado = 1;
  } while (!creado);

  user.id = atoi(argv[1]);
  user.pid = getpid();
  strcpy(user.pipe_cliente, "cliente_");
  strcat(user.pipe_cliente, argv[1]+'\0');

  // Se crea un segundo pipe
  unlink(user.pipe_cliente);
  if (mkfifo (user.pipe_cliente, fifo_mode) == -1) {
    perror("pipe_cliente mkfifo");
    exit(1);
  }
  // se envia el nombre del pipe al otro proceso.
  if(write(server, &user , sizeof(cliente)) == -1)
  {
    perror("En escritura");
    exit(1);
  }

  // Se abre el segundo pipe
  creado = 0;
  do {
    if ((user.pipe_id = open(user.pipe_cliente, O_RDONLY)) == -1) {
      perror("Cliente Abriendo el segundo pipe. Se volvera a intentar ");
      sleep(5);
    } else
      creado = 1;
  } while (!creado);

   // Se lee un mensaje por el segundo pipe.
  if(message == -1)
  {
    perror("En lectura");
    exit(1);
  }
  do
  {
    printf("Menu:\n\
            1. Follow\
            2. Unfollow\
            3. Tweet\
            4. Recuperar tweets\
            5. Desconexion" );
    scanf("%d\n", opcion);
    switch (opcion) {
      case 1:
        //TODO Follow
        break;
      case 2:
        //TODO Unfollow
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
