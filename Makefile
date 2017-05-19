all: cliente.exe servidor.exe clean clean_pipes clean_archives create_archives clean_images

cliente.exe: Client.o
	gcc Client.o -o cliente.exe

servidor.exe: Server.o
	gcc Server.o -o servidor.exe

Server.o: Server.c tweet.h
	gcc -c Server.c

Client.o: Client.c tweet.h
	gcc -c Client.c

clean:
	-rm -f *.o

clean_pipes:
	-rm -f cliente_*

clean_archives:
	-rm -r tweet_pendientes

create_archives:
	-mkdir tweet_pendientes

clean_images:
	-rm imagen*_*.bmp
