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

//*************************************************************************************************************************************************
//Función para abrir la imagen, colocarla en escala de grisis en la estructura imagen imagen (Arreglo de bytes de alto*ancho  --- 1 Byte por pixel 0-255)
//Parametros de entrada: Referencia a un BMP (Estructura BMP), Referencia a la cadena ruta char ruta[]=char *ruta
//Parametro que devuelve: Ninguno
//*************************************************************************************************************************************************
void AbrirImagen(BMP *imagen, char *ruta)
{
	FILE *archivo;	//Puntero FILE para el archivo de imágen a abrir
	int i,j,k;
  unsigned char P[3];

	//Abrir el archivo de imágen
	archivo = fopen( ruta, "rb+" );
	if(!archivo)
	{
		//Si la imágen no se encuentra en la ruta dada
		printf( "La imágen %s no fue encontrada\n",ruta);
	}

	//Leer la cabecera de la imagen y almacenarla en la estructura a la que apunta imagen
	fseek( archivo,0, SEEK_SET);
	fread(&imagen->bm,sizeof(char),2, archivo);
	fread(&imagen->tamano,sizeof(int),1, archivo);
	fread(&imagen->reservado,sizeof(int),1, archivo);
	fread(&imagen->offset,sizeof(int),1, archivo);
	fread(&imagen->tamanoMetadatos,sizeof(int),1, archivo);
	fread(&imagen->alto,sizeof(int),1, archivo);
	fread(&imagen->ancho,sizeof(int),1, archivo);
	fread(&imagen->numeroPlanos,sizeof(short int),1, archivo);
	fread(&imagen->profundidadColor,sizeof(short int),1, archivo);
	fread(&imagen->tipoCompresion,sizeof(int),1, archivo);
	fread(&imagen->tamanoEstructura,sizeof(int),1, archivo);
	fread(&imagen->pxmh,sizeof(int),1, archivo);
	fread(&imagen->pxmv,sizeof(int),1, archivo);
	fread(&imagen->coloresUsados,sizeof(int),1, archivo);
	fread(&imagen->coloresImportantes,sizeof(int),1, archivo);

	//Validar ciertos datos de la cabecera de la imágen
	if (imagen->bm[0]!='B'||imagen->bm[1]!='M')
	{
		printf ("La imagen debe ser un bitmap.\n");
	}
	if (imagen->profundidadColor!= 24)
	{
		printf ("La imagen debe ser de 24 bits.\n");
	}

	//Pasar la imágen a el arreglo reservado en escala de grises
	//unsigned char R,B,G;
	for (i=0;i<imagen->alto;i++)
	{
		for (j=0;j<imagen->ancho;j++){
		  for (k=0;k<3;k++) {
        fread(&P[k],sizeof(char),1, archivo); //Lectura de los pixeles
        imagen->pixel[i][j][k]=(unsigned char)P[k]; 	//asignacion del valor del pixel al arreglo
      }
		}
	}
	//Cerrrar el archivo
	fclose(archivo);
}


//****************************************************************************************************************************************************
//Función para crear una imagen BMP, a partir de la estructura imagen imagen (Arreglo de bytes de alto*ancho  --- 1 Byte por pixel 0-255)
//Parametros de entrada: Referencia a un BMP (Estructura BMP), Referencia a la cadena ruta char ruta[]=char *ruta
//Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************
void CrearImagen(BMP *imagen, char ruta[])
{
	FILE *archivo;	//Puntero FILE para el archivo de imágen a abrir
	int i,j,k;

	//Abrir el archivo de imágen
	archivo = fopen( ruta, "wb+" );
	if(!archivo)
	{
		//Si la imágen no se encuentra en la ruta dada
		printf( "La imágen %s no se pudo crear\n",ruta);
		exit(1);
	}

	//Escribir la cabecera de la imagen en el archivo
	fseek( archivo,0, SEEK_SET);
	fwrite(&imagen->bm,sizeof(char),2, archivo);
	fwrite(&imagen->tamano,sizeof(int),1, archivo);
	fwrite(&imagen->reservado,sizeof(int),1, archivo);
	fwrite(&imagen->offset,sizeof(int),1, archivo);
	fwrite(&imagen->tamanoMetadatos,sizeof(int),1, archivo);
	fwrite(&imagen->alto,sizeof(int),1, archivo);
	fwrite(&imagen->ancho,sizeof(int),1, archivo);
	fwrite(&imagen->numeroPlanos,sizeof(short int),1, archivo);
	fwrite(&imagen->profundidadColor,sizeof(short int),1, archivo);
	fwrite(&imagen->tipoCompresion,sizeof(int),1, archivo);
	fwrite(&imagen->tamanoEstructura,sizeof(int),1, archivo);
	fwrite(&imagen->pxmh,sizeof(int),1, archivo);
	fwrite(&imagen->pxmv,sizeof(int),1, archivo);
	fwrite(&imagen->coloresUsados,sizeof(int),1, archivo);
	fwrite(&imagen->coloresImportantes,sizeof(int),1, archivo);

	//Pasar la imágen del arreglo reservado en escala de grises a el archivo (Deben escribirse los valores BGR)
	for (i=0;i<imagen->alto;i++)
	{
		for (j=0;j<imagen->ancho;j++)
		{
    	for (k=0;k<3;k++)
		  	fwrite(&imagen->pixel[i][j][k],sizeof(char),1, archivo);  //Escribir el Byte Blue del pixel
		}
	}

	//Cerrar el archivo
	fclose(archivo);
}

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
      printf(" Se volvera a intentar despues\n");
      sleep(5);
    } else
      abierto = 1;
  } while (!abierto);
  return id_pipe;
}

//****************************************************************************************************************************************************
//Función para que un usuario pueda empezar a seguir a otro.
//Parametros de entrada: envioCliente-> variable para almacenar datos del envio del cliente al servidor
//                       envioServer-> variable para recibir los datos que se envian desde el servidor
//                       server-> identificador del pipe por medio del cual sse envian datos al servidor
//                       id-> identificador del usuario al cual se desea empezar a seguir
//Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************
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

//****************************************************************************************************************************************************
//Función para que un usuario deje de seguir a otro.
//Parametros de entrada: envioCliente-> variable para almacenar datos del envio del cliente al servidor
//                       envioServer-> variable para recibir los datos que se envian desde el servidor
//                       server-> identificador del pipe por medio del cual sse envian datos al servidor
//                       id-> identificador del usuario al cual se desea empezar a seguir
//Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************
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

//****************************************************************************************************************************************************
//Función para que un usuario se registre con el servidor
//Parametros de entrada: envioCliente-> variable para almacenar datos del envio del cliente al servidor
//                       envioServer-> variable para recibir los datos que se envian desde el servidor
//                       server-> identificador del pipe por medio del cual sse envian datos al servidor
//                       user-> Datos correspondientes al cliente que se desea registrar en el sistema
//Parametro que devuelve: el identificador del pipe en caso de que el registro sea exitoso
//                        -1 en caso contrario
//****************************************************************************************************************************************************
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

//****************************************************************************************************************************************************
//Función para que un usuario se pueda desconectar del servidor
//Parametros de entrada: envioCliente-> variable para almacenar datos del envio del cliente al servidor
//                       envioServer-> variable para recibir los datos que se envian desde el servidor
//                       server-> identificador del pipe por medio del cual sse envian datos al servidor
//                       cliente-> datos del cliente a desconectar del servidor
//Parametro que devuelve: 1 en caso de que la desconexion sea correcta
//                        0 en caso de que la desconexion sea incorrecta
//****************************************************************************************************************************************************
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
    unlink(user.pipe_cliente);
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
/*********************************************************************************************************
//PROGRAMA PRINCIPAL
//*********************************************************************************************************
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
  signal(SIGUSR1, (sighandler_t)tweet_receive);
  //Instalador del manejador de la senal SIGUSR2
  signal(SIGUSR2, (sighandler_t)tweets);

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
  //Validacion de correcta creacion del pipe de comunicacion
  if (mkfifo (user.pipe_cliente, fifo_mode) == -1) {
    perror("Cliente ya creado");
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
      printf("5. Desconexion\n\n> ");
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
