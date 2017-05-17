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

//*****************************************************************
//DECLARACIÓN DE TYPEDEF PARA EL MANEJO DE SENALES
//*****************************************************************
typedef void (*sighandler_t)(int);

//*****************************************************************
//DECLARACIÓN DE VARIABLES PARA EL MANEJO DE SENALES
//*****************************************************************
Cliente user;
EnvioServer datos;
unsigned int tweets_leer = 1;
unsigned int tweets_imagen = 1;
Respuesta modoOperacion;

//*****************************************************************
//DECLARACIÓN DE FUNCIONES
//*****************************************************************
void CrearImagen(BMP *imagen, char ruta[]);
int AbrirImagen(BMP *imagen, char *ruta);
int abrir_pipe(const char* pathname, int flags);
void follow(EnvioCliente envioCliente, EnvioServer envioServer, int server, int id);
void unfollow(EnvioCliente envioCliente, EnvioServer envioServer, int server, int id);
int registrar(EnvioCliente envioCliente, EnvioServer envioServer, Cliente user, int server);
int desconexion(EnvioCliente envioCliente, EnvioServer envioServer, Cliente user, int server);
void recuperarTweets(Cliente user, int server, EnvioCliente envioCliente, EnvioServer envioServer);
void enviarTweet(Cliente user, int server, EnvioCliente envioCliente, EnvioServer envioServer);

sighandler_t tweet_receive(void)
{
  char nombre_imagen[LINE], strNum[TAMUSR];
  // Se lee un mensaje por el segundo pipe.
  if(read(user.pipe_id, &datos, sizeof(Tweet) == -1))
  {
   perror("En lectura");
   exit(1);
  }
  printf("Tweet enviado por: %d\n %s\n", datos.tweet.id , datos.tweet.texto);
  if(datos.tweet.conImagen == 1){
    printf("Tweet enviado contiene una imagen\n");
    strcpy(nombre_imagen,"imagen");
    sprintf(strNum,"%d%d_%d",tweets_imagen,user.id,datos.tweet.id);
    strcat(nombre_imagen,strNum);
    CrearImagen(&datos.tweet.imagen,nombre_imagen);
  }
}

sighandler_t tweets(void)
{
  tweets_leer = 0;
}

//*************************************************************************************************************************************************
//Función para abrir la imagen, colocarla en escala de grisis en la estructura imagen imagen (Arreglo de bytes de alto*ancho  --- 1 Byte por pixel 0-255)
//Parametros de entrada: Referencia a un BMP (Estructura BMP), Referencia a la cadena ruta char ruta[]=char *ruta
//Parametro que devuelve: Ninguno
//*************************************************************************************************************************************************
int AbrirImagen(BMP *imagen, char *ruta)
{
	FILE *archivo;	//Puntero FILE para el archivo de imágen a abrir
	int i,j,k;
  unsigned char P[3];
  printf("Imagen => %s\n", ruta);
	//Abrir el archivo de imágen
	archivo = fopen( ruta, "rb+" );
	if(!archivo)
	{
		//Si la imágen no se encuentra en la ruta dada
		printf( "La imagen %s no fue encontrada\n",ruta);
    return 0;
	}
  else{
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
      return 0;
  	}
    else{
    	if (imagen->profundidadColor!= 24)
    	{
    		printf ("La imagen debe ser de 24 bits.\n");
        return 0;
    	}
      else{
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
      }
  	}
    //Cerrar el archivo
  	fclose(archivo);
    return 1;
  }
  printf("SALI\n");
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
	}
  else{
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
      modoOperacion = SINCRONO;
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

//****************************************************************************************************************************************************
//Función para que un usuario se pueda desconectar del servidor
//Parametros de entrada: cliente-> datos del cliente a desconectar del servidor
//                       server-> identificador del pipe por medio del cual sse envian datos al servidor
//                       envioCliente-> variable para almacenar datos del envio del cliente al servidor
//                       envioServer-> variable para recibir los datos que se envian desde el servidor
//Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************
void recuperarTweets(Cliente user, int server, EnvioCliente envioCliente, EnvioServer envioServer){
  int tweetLeido = 0;
  envioCliente.operacion = RE_TWEETS;
  envioCliente.cliente = user;
  if(modoOperacion == SINCRONO){
    if(write(server, &envioCliente , sizeof(envioCliente)) == -1)
    {
      perror("En escritura");
      exit(1);
    }
    tweets_leer = 1;
    while(tweets_leer == 1){
      if (read (user.pipe_id, &envioServer, sizeof(envioServer)) == -1) {
        perror("En lectura");
        exit(1);
      }
      if(envioServer.respuesta == TWEET){
        tweetLeido++;
        printf("Tweet enviado por: %d\n %s", envioServer.tweet.id , envioServer.tweet.texto);
        if(envioServer.tweet.conImagen == 1){
          printf("Tweet enviado contiene una imagen\n");
        }
      }
      else{
        if(envioServer.respuesta == ASINCRONO){
          printf("Esta opcion no es valida en este modo de operacion\n");
          modoOperacion = ASINCRONO;
          tweetLeido = 1;
        }
      }
    }
    if(tweetLeido == 0){
      printf("No hay tweets pendientes de ser recuperados\n");
    }
  }
  else{
    printf("Esta opcion no es valida en este modo de operacion\n");
  }
}

//****************************************************************************************************************************************************
//Función para que un usuario envie un tweet con o sin imagen y texto
//Parametros de entrada: user-> informacion del usuario que va a enviar el tweet
//                       server-> identificador del pipe por medio del cual sse envian datos al servidor
//                       envioCliente-> variable para almacenar datos del envio del cliente al servidor
//                       envioServer-> variable para recibir los datos que se envian desde el servidor
//Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************
void enviarTweet(Cliente user, int server, EnvioCliente envioCliente, EnvioServer envioServer){

  int opc, imagenCorrecta;
  char ruta[TAM], strNum[TAMUSR];
  char nombre_imagen[LINE];
  char* tweet;
  int line = LINE;
  BMP *img = NULL;

  tweet = malloc(LINE);
  printf("Ingrese la opcion que desea.\n");
  printf("1. Tweet con texto sin imagen.\n2. Tweet con texto y con imagen\n");
  printf("3. Tweet con imagen y sin texto.\n");
  scanf("%d", &opc);
  switch(opc){
    case 1:
    fflush(stdin);
    printf("Escriba el tweet a enviar: \n");
    scanf("%s", envioCliente.tweet.texto);
    //fgets(tweet,LINE,stdin);
    //strcpy(envioCliente.tweet.texto,tweet);
    printf("=>%s\n", envioCliente.tweet.texto);
    imagenCorrecta = 1;
    break;
    case 2:
    printf("Escriba el tweet a enviar: \n");
    fgets(envioCliente.tweet.texto,LINE,stdin);
    printf("Digite la ruta de la imagen a enviar: \n");
    scanf("%s", ruta);
    envioCliente.tweet.conImagen = 1;
    img = malloc(sizeof(BMP));
    imagenCorrecta=AbrirImagen(img,ruta);
    if(imagenCorrecta == 1){
      envioCliente.tweet.imagen = *img;
    }
    break;
    case 3:
    envioCliente.tweet.texto[0] = '\0';
    printf("Digite la ruta de la imagen a enviar: \n");
    scanf("%s", ruta);
    envioCliente.tweet.conImagen = 1;
    img = malloc(sizeof(BMP));
    imagenCorrecta=AbrirImagen(img,ruta);
    if(imagenCorrecta == 1){
      envioCliente.tweet.imagen = *img;
    }
    break;
    default:
    printf("Opcion invalida.\n");
  }
  envioCliente.tweet.id = user.id;
  envioCliente.operacion = TWEET_C;
  envioCliente.cliente = user;
  if(imagenCorrecta == 1){
    printf("Write\n");
    if(write(server, &envioCliente , sizeof(envioCliente)) == -1)
    {
      perror("En escritura");
      exit(1);
    }
    printf("read\n");
    if (read (user.pipe_id, &envioServer, sizeof(envioServer)) == -1) {
      perror("En lectura");
      exit(1);
    }
    printf("If\n");
    if(envioServer.respuesta == EXITO){
      printf("Tweet enviado por: %d\n %s", envioServer.tweet.id , envioServer.tweet.texto);
      if(envioServer.tweet.conImagen == 1){
        printf("Tweet enviado contiene una imagen\n");
        strcpy(nombre_imagen,"imagen");
        sprintf(strNum,"%d%d_%d",tweets_imagen,user.id,envioServer.tweet.id);
        strcat(nombre_imagen,strNum);
        CrearImagen(&envioServer.tweet.imagen,nombre_imagen);
      }
    }
  }
  else{
    printf("HOLA\n");
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
  //Instalador del manejador de la senal SIGUSR1 (ASINCRONO)
  signal(SIGUSR1, (sighandler_t)tweet_receive);
  //Instalador del manejador de la senal SIGUSR2 (SINCRONO)
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
    perror("Cliente ya registrado.");
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
          enviarTweet(user, server, envioCliente, envioServer);
          break;
        case 4:
          //TODO Recuperar
          recuperarTweets(user, server, envioCliente, envioServer);
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
