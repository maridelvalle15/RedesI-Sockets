all: server client clean

server: server.o
	gcc server.o -o bsb_svr -pthread

client: client.o
	gcc client.o -o bsb_cli

server.o: server.c
	gcc -c server.c -pthread

client.o: client.c
	gcc -c client.c

clean:
	rm -f *.o
