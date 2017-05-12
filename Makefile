servidor.exe: Server.o
	gcc Server.o -o servidor.exe
Server.o: Server.c tweet.h
	gcc -c Server.c
