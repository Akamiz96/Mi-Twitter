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
int tweets_leer = 0;

sighandler_t tweet_receive(void)
{
  // Se lee un mensaje por el segundo pipe.
  if(read(user.pipe_id, &datos, sizeof(Tweet) == -1))
  {
   perror("En lectura");
   exit(1);
  }
  printf("Tweet enviado por: %d\n %s", datos.id , datos.texto);
}

sighandler_t tweets(void)
{
  // Se lee un mensaje por el segundo pipe.
  if(read(user.pipe_id, &datos, sizeof(Tweet) == -1))
  {
   perror("En lectura");
   exit(1);
  }
  tweets_leer++;
}

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
  int leer = -1;
  if(user.id == id){
    printf("El id es el mismo con el cual se encuentra conectado\n");
  }
  else{
    envioCliente.operacion = FOLLOW;
    envioCliente.cliente.pid = user.pid;
    envioCliente.cliente.id = id;
    printf("Write\n");
    if(write(server, &envioCliente , sizeof(EnvioCliente)) == -1)
    {
      perror("En escritura");
      exit(1);
    }
    printf("Reades\n");
    leer = read (user.pipe_id, &envioServer, sizeof(EnvioServer));
    printf("Leer\n");
    if (leer == -1) {
      perror("En lectura");
      exit(1);
    }
    printf("Leyo\n");
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
}

void unfollow(EnvioCliente envioCliente, EnvioServer envioServer, int server, int id){
  if(user.id == id){
    printf("El id es el mismo con el cual se encuentra conectado\n");
  }
  else{
    envioCliente.operacion = UNFOLLOW;
    envioCliente.cliente.pid = user.pid;
    envioCliente.cliente.id = id;
    printf("Write\n");
    if(write(server, &envioCliente , sizeof(EnvioCliente)) == -1)
    {
      perror("En escritura");
      exit(1);
    }
    if (read (user.pipe_id, &envioServer, sizeof(EnvioServer)) == -1) {
      perror("En lectura");
      exit(1);
    }
    printf("Leyo\n");
    switch(envioServer.respuesta){
      case EXITO:
        printf("Ahora no sigue al usuario con id: %d\n\n", id);
        break;
      case FALLO:
        printf("Ya no se encuentra siguiendo al usuario con el id: %d\n\n", id);
        break;
      case INVALIDO:
        printf("Id ingresado invalido\nSe ingreso el id: %d\n\n", id);
        break;
      default:
        printf("Error desconocido\nContacte al desarrollador");
    }
  }
}

int registrar(EnvioCliente envioCliente, EnvioServer envioServer, Cliente user, int server){
  int pipe_id, leer;
  envioCliente.operacion = REGISTER;
  envioCliente.cliente = user;
  printf("write\n");
  if(write(server, &envioCliente , sizeof(envioCliente)) == -1)
  {
    perror("En escritura");
    exit(1);
  }
  printf("Abrir\n");
  pipe_id = abrir_pipe(user.pipe_cliente, O_RDONLY);
  if (read (pipe_id, &envioServer, sizeof(envioServer)) == -1) {
    perror("En lectura");
    exit(1);
  }
  printf("leyo\n");
  switch(envioServer.respuesta){
    case EXITO:
      printf("Registrado exitosamente.\n");
      return pipe_id;
      break;
    case INVALIDO:
      printf("Registro no completado\nYa se encuentra registrado en el sistema.");
      return -1;
      break;
    default:
      printf("Error desconocido\nContacte al desarrollador");
      return -1;
  }
}

int desconexion(EnvioCliente envioCliente, EnvioServer envioServer, Cliente user, int server){
  envioCliente.operacion = DESCONEXION;
  envioCliente.cliente = user;
  if(write(server, &envioCliente , sizeof(envioCliente)) == -1)
  {
    perror("En escritura");
    exit(1);
  }
  if (read (user.pipe_id, &envioServer, sizeof(envioServer)) == -1) {
    perror("En lectura");
    exit(1);
  }
  switch(envioServer.respuesta){
    case EXITO:
    printf("Desconectado exitosamente.\n");
    return 1;
    break;
    case INVALIDO:
    printf("Desconexion no completada\n");
    return 0;
    break;
    default:
    printf("Error desconocido\nContacte al desarrollador");
    return 0;
  }
}

/*
Main ejecutable del programa cliente
Parametros para su ejecucion:
  ->id: identificacion del usuario a conectarse al sistema
  ->pipeNom: nombre del pipe de comunicacion entre el usuario y el cliente
 */

int main (int argc, char **argv)
{
  //Declaracion de variable necesarias
  int  server, descon = 0, opcion, id;
  Tweet datos;
  EnvioCliente envioCliente;
  EnvioServer envioServer;

  //Definicion del tipo de lectura o escritura del pipe
  mode_t fifo_mode = S_IRUSR | S_IWUSR;

  //Validacion de parametros del main
  if( argc != 3 )
  {
    printf("Error\nDebe ser ejecutado de la forma: %s ID pipe_server \n",argv[0]);
    exit(1);
  }
  //Instalador del manejador de la senal SIGUSR1
  //signal(SIGUSR1, tweet_receive);

  // Se abre el pipe cuyo nombre se recibe como argumento del main.
  server = abrir_pipe(argv[2], O_WRONLY);

  /*
   Creacion del pipe del cliente a partir de:
   "cliente_" + id + '\0'
   */
  user.id = atoi(argv[1]);
  user.pid = getpid();
  strcpy(user.pipe_cliente, "cliente_");
  strcat(user.pipe_cliente, argv[1]+'\0');

  unlink(user.pipe_cliente);
  //Validacion de correcta creacion del pipe de comunicacion
  if (mkfifo (user.pipe_cliente, fifo_mode) == -1) {
    perror("pipe_cliente mkfifo");
    exit(1);
  }

  /*
   Registro del cliente en el servidor
   -1 -> Registro incompleto
   Cualquier otro caso -> registro exitoso
   */
  user.pipe_id = registrar(envioCliente, envioServer, user, server);

  //Validacion de registro exitoso del cliente en el servidor
  if(user.pipe_id != -1){

    //Ciclo de control de opciones sobre el cliente
    do
    {
      //Impresion del menu para el cliente
      printf("Menu:\n1. Follow\n2. Unfollow\n3. Tweet\n4. Recuperar tweets\n");
      printf("5. Desconexion\n\nOpcion:");
      //Lectura de la opcion del cliente
      scanf("%d", &opcion);
      //Eleccion de la funcionalidad segun la eleccion del cliente
      switch (opcion) {
        //Caso para la opcion de FOLLOW
        case 1:
          printf("Ingresar id del usuario a seguir: ");
          scanf("%d", &id);
          follow(envioCliente, envioServer, server, id);
          break;
        //Caso para la opcion de Unfollow
        case 2:
          //TODO Unfollow
          printf("Ingresar id del usuario a no seguir mas: ");
          scanf("%d", &id);
          unfollow(envioCliente, envioServer, server, id);
          break;
        case 3:
          //TODO Tweet
          printf("Desea enviar una imagen: \n");
          //scanf("%d\n", &id);
          printf("Escriba el tweet a enviar: \n");
          //scanf("%d\n", &id);
          //if(imagen == "s" || imagen == "S")
          printf("Digite la ruta de la imagen a enviar: \n");
          //scanf("%d\n", &id);
          break;
        case 4:
          //TODO Recuperar
          break;
        //Caso para la desconexion del cliente respecto al servidor
        case 5:
          //TODO Desconexion
          descon = desconexion(envioCliente, envioServer, user, server);
          break;
        default:
          printf("Opcion Invalida\nIntente de nuevo");
      }//Fin seleccion de la opcion
    } while(!descon);//Fin ciclo de control
  }//Fin if
  exit(0);
}
