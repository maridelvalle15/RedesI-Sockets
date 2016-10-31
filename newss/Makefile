all : tcpserver tcpclient

tcpserver : tcpserver.c
	gcc -o bsb_svr tcpserver.c -pthread

tcpclient : tcpclient.c
	gcc -o bsb_cli tcpclient.c

clear :
	rm *.o
