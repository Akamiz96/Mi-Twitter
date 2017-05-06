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


cliente user;
char mensaje[10];


int main (int argc, char **argv)
{
  int  server, pid, creado = 0, res;
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
  } while (creado == 0);

  user.id = atoi(argv[1]);
  strcpy(user.pipe_cliente, "cliente_");
  strcat(user.pipe_cliente, argv[1]+'\0');

  // Se crea un segundo pipe
  unlink(user.pipe_cliente);
  if (mkfifo (user.pipe_cliente, fifo_mode) == -1) {
    perror("pipe_cliente mkfifo");
    exit(1);
  }
  // se envia el nombre del pipe al otro proceso.
  write(server, &user , sizeof(cliente));

  // Se abre el segundo pipe
  creado = 0;
  do {
    if ((user.pipe_id = open(user.pipe_cliente, O_RDONLY)) == -1) {
      perror("Cliente Abriendo el segundo pipe. Se volvera a intentar ");
      sleep(5);
    } else
      creado = 1;
  } while (creado == 0);

   // Se lee un mensaje por el segundo pipe.

  read(user.pipe_id, mensaje, 10);
  printf("El proceso cliente termina y lee %s \n", mensaje);
  exit(0);
}
