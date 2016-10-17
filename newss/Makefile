all : tcpechoserver tcpechoclient

tcpechoserver : tcpechoserver.c
	gcc -o bsb_svr tcpechoserver.c -pthread

tcpechoclient : tcpechoclient.c
	gcc -o bsb_cli tcpechoclient.c

clear :
	rm *.o
