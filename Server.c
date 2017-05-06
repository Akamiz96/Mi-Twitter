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
#define TAMNOM 20

int main (int argc, char **argv)
{
  int server, pipe_cliente,  pid, n, cuantos,res,creado=0;
  tweet datos;

  mode_t fifo_mode = S_IRUSR | S_IWUSR;
  if(argc != 5)
  {
    printf("Error\nDebe ser ejecutado de la forma: %s N relaciones modo pipeNom\n", argv[0]);
    exit(1);
  }
  // Creacion del pipe inicial, el que se recibe como argumento del main
  unlink(argv[4]);
  if (mkfifo (argv[4], fifo_mode) == -1) {
    perror("server mkfifo");
    exit(1);
  }

  // Abre el pipe. No olviden validar todas las llmadas al sistema.
  server = open(argv[4], O_RDONLY);
  // El otro proceso (nom1) le envia el nombre para el nuevo pipe y el pid.
  if (read (server, &user, sizeof(cliente)) == -1) {
    perror("proceso lector");
    exit(1);
  }
  printf ("Lei el cliente %d\n", user.id);

  do {
    if ((pipe_cliente = open(user.pipe_cliente, O_WRONLY)) == -1) {
      perror(" server Abriendo el segundo pipe ");
      printf(" Se volvera a intentar despues\n");
      sleep(5); //los unicos sleeps que deben colocar son los que van en los ciclos para abrir los pipes.
      } else creado = 1;
   }  while (creado == 0);


    // Se escribe un mensaje para el  proceso (nom1)

   write(pipe_cliente, "hola", 5);
   exit(0);

}
