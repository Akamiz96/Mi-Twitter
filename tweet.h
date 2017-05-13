#define         TAM      	500
#define         RUTA     	45
#define         NPIXEL   	3
#define         MAXCHAR		140
#define 				TAMUSR 	 	80
#define 				LINE 			160


typedef enum {REGISTER, FOLLOW, UNFOLLOW, TWEET_C, RE_TWEETS, DESCONEXION} Operacion;
typedef enum {EXITO, FALLO, TWEET, SINCRONO, ASINCRONO, INVALIDO} Respuesta;

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

typedef struct Tweet
{
	int id;
  char texto[MAXCHAR];
  BMP imagen;
} Tweet;

typedef struct Cliente
{
	int id;
	int pipe_id;
	pid_t pid;
	char pipe_cliente[MAXCHAR];
} Cliente;

typedef struct EnvioCliente
{
	Operacion operacion;
	Tweet tweet;
	Cliente cliente;
}EnvioCliente;

typedef struct EnvioServer
{
	Respuesta respuesta;
	Tweet tweet;
}EnvioServer;
