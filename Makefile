servidor.exe: Server.o
	gcc Server.o -o servidor.exe
Server.o: Server.c tweet.h
	gcc -c Server.c
cliente.exe: Client.o
	gcc Client.o -o cliente.exe
Client.o: Client.c tweet.h
	gcc -c Client.c
