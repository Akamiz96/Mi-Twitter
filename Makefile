cliente.exe servidor.exe: Client.o Server.o
	gcc Client.o -o cliente.exe
	gcc Server.o -o servidor.exe
Server.o: Server.c tweet.h
	gcc -c Server.c
Client.o: Client.c tweet.h
	gcc -c Client.c
clean:
	rm Server.o Client.o
