#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


socklen_t addr_len;
struct sockaddr_in clientaddr;
struct sockaddr_in serveraddr;

int newfd;
int n;
int s;

extern int handlerSig;
extern char error[127];

int tcp_open(int puerto)
{
	// Creamos socket
	s = socket(AF_INET,SOCK_STREAM, 0);

	// Cargamos datos de IP:PORT del server
	bzero((char*) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(puerto);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if(serveraddr.sin_addr.s_addr==INADDR_NONE)
	{
		fprintf(stderr,"ERROR invalid server IP\r\n");
		return 1;
	}
	return 0;
}

int tcp_bind_listen()
{
	// Abrimos puerto con bind()
	if (bind(s, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
		close(s);
		perror("bind");
		return 1;
	}

	// Seteamos socket en modo Listening
	if (listen (s, 10) == -1) // backlog=10
  	{
    	perror("listen");
    	return 1;;
  	}
	return 0;
}

int tcp_accept()
{
	// Ejecutamos accept() para recibir conexiones entrantes
	addr_len = sizeof(struct sockaddr_in);
	if ( (newfd = accept(s, (struct sockaddr *)&clientaddr, &addr_len)) == -1)
	{
		return 1;
	}
 	printf  ("Conexión TCP establecida: %s\n", inet_ntoa(clientaddr.sin_addr));

	return 0;
}

int tcp_read(char* buffer, int bytes)
{
	printf( "tcp_read\n" );
	printf("Paso1\n");
	// Leemos mensaje de cliente
	if( ( n = read(newfd, buffer, bytes) ) <= 0 )
	{
		printf("Paso2\n");
		return 1;
	}

	buffer[n]=0;

	printf("Paso3\n");

	return 0;
}

int tcp_write(char* buffer, int bytes)
{
	if (write (newfd, buffer, bytes) == -1)
		return 1;

	return 0;
}

int tcp_closeFD()
{
	// Cerramos FileDescriptor
	close(newfd);

	return 0;
}

int tcp_closeSOC()
{
	// Cerramos socket
	close(s);

	return 0;
}

void* tcp_servidor( void* parametros ){
	while(1)
	{
		printf ("Esperando conexión TCP entrante...\r\n");
		//netstat -aon
		if ( tcp_accept() )
		{
			strcpy( error , "Error en tcp_accept\r\n");
			handlerSig = 99;
		}
		else
		{
			//tcp_listo = 1;

			while(1)
			{
				//printf("tcp_servidor - handlerSig=%d\n", handlerSig);
				// Aca el comunicacion_consumidor() lee del socket
				if (handlerSig == 100){
					//printf("Error en pFuncion_read. Posiblemente el cliente se desconecto...\r\n");
					handlerSig = 0;
					break;
				}

				// Terminar función al recibir una señal
				if ( handlerSig )
					break;

				usleep(100000);	                                    // 100000 = 0.1 seg
			}
			if ( tcp_closeFD() != 0 )
			{
				strcpy( error , "Error en tcp_closeFD");
				handlerSig = 99;
			}
		}
		if ( handlerSig )
			break;
	}
}

void* tcp_cliente( void* parametros ){

}
