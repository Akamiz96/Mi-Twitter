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

int main (int argc, char **argv)
{
  int server, pipe_cliente,  pid, N, cuantos, res, creado = 0;
  int grafo[TAMUSR][TAMUSR], i, j;
  tweet datos;
  FILE* file;
  char buffer[LINE];
  Cliente cliente[TAMUSR];

  mode_t fifo_mode = S_IRUSR | S_IWUSR;
  if(argc != 5)
  {
    printf("Error\nDebe ser ejecutado de la forma: %s N relaciones modo pipeNom\n", argv[0]);
    exit(1);
  }
  N = atoi(argv[1]);
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
  server = open(argv[4], O_RDONLY);
  // El otro proceso (nom1) le envia el nombre para el nuevo pipe y el pid.
  if (read (server, &user, sizeof(cliente)) == -1) {
    perror("En lectura");
    exit(1);
  }
  printf ("Lei el cliente %d\n", user.id);

  do {
    if ((pipe_cliente = open(user.pipe_cliente, O_WRONLY)) == -1) {
      perror(" server Abriendo el segundo pipe ");
      printf(" Se volvera a intentar despues\n");
      sleep(5); //los unicos sleeps que deben colocar son los que van en los ciclos para abrir los pipes.
      } else creado = 1;
   }  while (!creado);


    // Se escribe un mensaje para el  proceso (nom1)

   if(write(pipe_cliente, "hola", 5) == -1)
   {
     perror("En escritura");
     exit(1);
   }
   //TODO mandar señal
   exit(0);

}
