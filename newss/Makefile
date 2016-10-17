CC = gcc
CFLAGS = -g

all : tcpechoserver tcpechoclient

tcpechoserver : tcpechoserver.c
	$(CC) $(CFLAGS) -o tcpechoserver tcpechoserver.c -pthread

tcpechoclient : tcpechoclient.c
	$(CC) $(CFLAGS) -o tcpechoclient tcpechoclient.c

clear :
	rm *.o


