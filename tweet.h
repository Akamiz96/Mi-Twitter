//*****************************************************************
//DEFINICION DE CONSTANTES DEL PROGRAMA
//*****************************************************************
#define         TAM      	500
#define         RUTA     	45
#define         NPIXEL   	3
#define         MAXCHAR		140
#define 				TAMUSR 	 	80
#define 				LINE 			160

/*
	Definicion de una enumeracion para las funcionalidades presentadas para el usuario
 */
typedef enum {REGISTER, FOLLOW, UNFOLLOW, TWEET_C, RE_TWEETS, DESCONEXION} Operacion;

/*
	Definicion de una enumeracion para las respuestas del servidor
 */
typedef enum {EXITO, FALLO, TWEET, SINCRONO, ASINCRONO, INVALIDO, INCORRECTO} Respuesta;

/*
	Estructura de datos para la imagen BMP que puede ser enviada por los usuarios
 */
typedef struct BMP
{
	char bm[2];					//(2 Bytes) BM (Tipo de archivo)
	int tamano;					//(4 Bytes) Tamaño del archivo en bytes
	int reservado;					//(4 Bytes) Reservado
	int offset;						//(4 Bytes) offset, distancia en bytes entre la img y los píxeles
	int tamanoMetadatos;			//(4 Bytes) Tamaño de Metadatos (tamaño de esta estructura = 40)
	int alto;						//(4 Bytes) Ancho (numero de píxeles horizontales)
	int ancho;					//(4 Bytes) Alto (numero de pixeles verticales)
	short int numeroPlanos;			//(2 Bytes) Numero de planos de color
	short int profundidadColor;		//(2 Bytes) Profundidad de color (debe ser 24 para nuestro caso)
	int tipoCompresion;				//(4 Bytes) Tipo de compresión (Vale 0, ya que el bmp es descomprimido)
	int tamanoEstructura;			//(4 Bytes) Tamaño de la estructura Imagen (Paleta)
	int pxmh;					//(4 Bytes) Píxeles por metro horizontal
	int pxmv;					//(4 Bytes) Píxeles por metro vertical
	int coloresUsados;				//(4 Bytes) Cantidad de colores usados
	int coloresImportantes;			//(4 Bytes) Cantidad de colores importantes
	unsigned char pixel[TAM][TAM][NPIXEL]; 			//Puntero a una tabla dinamica de caracteres de 3 dimensiones para almacenar los pixeles
}BMP;

/*
	Estructura de datos para la definicion de un Tweet
 */
typedef struct Tweet
{
	int id; //id del cliente que envia el tweet
  char texto[MAXCHAR]; //Espacio de caracteres para el texto del tweet
	int conImagen; //Entero para identificar si el tweet tiene imagen incluida
  BMP imagen; //Espacio para la imagen que puede ser adicionada al tweet
} Tweet;

/*
	Estructura de datos para la definicion de un Cliente
 */
typedef struct Cliente
{
	int id; //id del cliente dentro del sistema
	int pipe_id; //pipe de comunicacion entre el servidor y el cliente
	pid_t pid; //id dado por el Sistema Operativo al proceso que corre el cliente
	char pipe_cliente[MAXCHAR]; //nombre del pipe de comunicacion entre el servidor y el cliente
} Cliente;

/*
	Estructura de datos para ser enviada entre los clientes y el servidor
 */
typedef struct EnvioCliente
{
	Operacion operacion; // operacion requerida por el usuario al servidor
	Tweet tweet; // Tweet que puede haber enviado el cliente al servidor
	Cliente cliente; // Informacion del cliente el cual requiere la solicitud o al cual se le envia la solicitud
}EnvioCliente;

/*
	Estructura de datos para ser enviada entre el servidor y los clientes
 */
typedef struct EnvioServer
{
	Respuesta respuesta; // respuesta del servidor a la solicitud del cliente
	Tweet tweet; // Envio posible de tweet
}EnvioServer;
