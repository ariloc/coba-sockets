#makefile for sockets project
all: client server
client: client.o
	gcc -o client client.o
server: server.o
	gcc -o server server.o
client.o: client.c
	gcc -c client.c
server.o: server.c
	gcc -c server.c