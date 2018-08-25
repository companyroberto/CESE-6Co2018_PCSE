/*============================================================================
 * Autor:		Roberto Company
 * Fecha:		31/07/2018
 * Descripcion:	Programa principal para verificar procesos de comunicacion
 *===========================================================================*/

/*==================[inclusiones]============================================*/
#include <stdio.h>
#include <string.h>

#include "mensajeria.h"
#include "comunicacion.h"
#include "conexionTP.h"
#include "serviciosTCP.h"

#include <signal.h>
#include <pthread.h>

#include <RF24/RF24.h>


/*==================[macros y definiciones]==================================*/
#define _SerieIUM_				1234
#define _VersionPaquete_		1
#define _VersionFirmware_		1
#define _IDSistemaND_			"ND"	//_IDSistemaEmisor_
#define _IDSistemaGW_			"GW"	//_IDSistemaReceptor_
#define _TimeOut_			5

void*	reloj( void* );
int8_t segActuales = 0;
int8_t minActuales = 0;
int8_t horActuales = 0;
int8_t diaActuales = 0;

enum conf_modo {
	gateway = 1,
	nodo = 2
};

enum conf_conexion {
	teclado_pantalla = 1,
	socket = 2,
	radio_frecuencia = 3
};

enum conf_parametros {
	servidor = 1,
	cliente = 2
};

bufferCircular_t buffer_tx, buffer_rx;

#define _PUERTO_	10000
int  handlerSig		= 0;					// Utilizado para recibir Ctrl+C y salir de cada función. También como error en funciones.
char error[127]		= "";					// Variable global de error para mostrar el mensaje al final.
int  debug	  		= 100;					// Nivel de debug para mostrar el log. 0=Deshabilitado, 1=Especificos, >10=Generales...

void  handlerSeniales	(int sig);

pthread_t pt_comunicacion_consumidor;
pthread_t pt_comunicacion_productor;
pthread_t pt_comunicacion_procesar;
pthread_t pt_conexion_parametros;
pthread_t pt_reloj;

int modo = 0, conexion = 0, parametros = 0;


using namespace std;

RF24 radio(22,0); // SPI_DEV 
////const uint8_t pipes[][6] = {"2Node","1Node"};
const uint64_t pipes[6] = {0x65646f4e32LL,0x65646f4e31LL};

int radio_read(char* buffer, int bytes)
{
	while( 1 ){
		if ( handlerSig != 0 && handlerSig != 100 )
			break;

		if ( radio.available() ){
			memset( buffer, '\0', bytes );
				
    			char tmp[bytes+1];
       	     		memset( tmp, '\0', sizeof(tmp) );    
		        radio.read( &tmp, bytes );
		        strncpy( buffer, tmp, bytes);
                	if (debug){
				printf( "radio_read(%d)=%s\n\r", strlen(tmp), tmp );
        		}
			break;
    	}
		usleep(500000);	               	// 500000 = 0.5 seg
	}
	return 0;  						 	// no estoy devolviendo el error
}

int radio_write(char* buffer, int bytes)
{
	radio.stopListening();

	char tmp[bytes+1];
	memset( tmp, '\0', sizeof(tmp) );
	strncpy( tmp, buffer, bytes );
	radio.write( &tmp, strlen(tmp) );

	if (debug)
		printf( "radio_write(%d)=%s\n\r", strlen(tmp), tmp );

	radio.startListening();

	return 0;   						// no estoy devolviendo el error
}

// .\siscof.exe GW|ND TP|SC|RF uno dos (argc = 3 / argv[1] = uno)
int main(int argc, char *argv[]) 
{
	printf ("SISCOF - Sistema para el Control de la Cadena de Frío (v1.2) \n\n");

	//nRF24 configuración
	radio.begin();
	radio.setChannel(0x4c);
	radio.setAutoAck(1);
	radio.setRetries(15,15);
	radio.setPayloadSize(32);                                       //tamaño del paquete
	radio.openReadingPipe(1,pipes[0]);
	radio.openWritingPipe(pipes[1]);
	radio.printDetails();                                           //Debug
	radio.powerUp();
	radio.startListening();

	bcm2835_init();
	bcm2835_gpio_write(16, 1);


	/*==================[Parametros de configuración]================================*/

	if ( argc == 3 || argc == 4 ){
		if ( strncmp( argv[1], "GW", 2 ) == 0 )
			modo = gateway;
		else if ( strncmp( argv[1], "ND", 2 ) == 0 )
			modo = nodo;

		if ( strncmp( argv[2], "TP", 2 ) == 0 )
			conexion = teclado_pantalla;
		else if ( strncmp( argv[2], "SC", 2 ) == 0 ){
			conexion = socket;
			if ( strncmp( argv[3], "SERVIDOR", 8 ) == 0 )
				parametros = servidor;
			else if ( strncmp( argv[3], "CLIENTE", 7 ) == 0 )
				parametros = cliente;
			else
				conexion = 0;
		}else if ( strncmp( argv[2], "RF", 2 ) == 0 )
			conexion = radio_frecuencia;
	}
	
	// modo		= nodo;					// debug	gateway				/		nodo
	// conexion	= socket;				// debug	teclado_pantalla	/		socket
	// parametros	= servidor;			// debug	servidor			/		cliente

	if ( modo == 0 || conexion == 0 ){
		printf ("E R R O R - Los argumentos son 2: GW|ND TP|SC|RF \n\n");
		return 1;
	}

	/*==================[Configuración del Sistema]==================================*/
	comunicacion_inicializar_pendientes();
	switch ( modo ){
		case gateway:
			printf ( "modo: GW \n" );
			comunicacion_configurar_paquetes (_SerieIUM_, _VersionPaquete_, _VersionFirmware_, _IDSistemaGW_, _IDSistemaND_, _TimeOut_);
			break;

		case nodo:
			printf ( "modo: ND \n" );
			comunicacion_configurar_paquetes (_SerieIUM_, _VersionPaquete_, _VersionFirmware_, _IDSistemaND_, _IDSistemaGW_, _TimeOut_);
			break;

		default:
			printf ("E R R O R - Modo no configurado... \n\n");
			return 2;
			break;
	}
	switch ( conexion ){
		case teclado_pantalla:
			printf ( "conexion: TP \n" );
			// La función tp_read tiene que ser bloqueante y se llaman en un thread acá.
			// En embebido, hacerlas todas no bloqueante y llamarlas en un SO cooperativo.
			comunicacion_inicializar_buffer( tp_read, tp_write, &buffer_tx, &buffer_rx );
			break;

		case socket:
			printf ( "conexion: SC \n" );
			comunicacion_inicializar_buffer( tcp_read, tcp_write, &buffer_tx, &buffer_rx );	
			break;

		case radio_frecuencia:
			printf ( "conexion: RF \n" );
			comunicacion_inicializar_buffer( radio_read, radio_write, &buffer_tx, &buffer_rx );	
			break;

		default:
			printf ("E R R O R - Conexión no desarrollada... \n\n");
			return 3;
			break;
	}

	/*==================[Inicio del Programa]========================================*/
	struct sigaction sa;
	sa.sa_handler = handlerSeniales;
	sa.sa_flags   = 0;
	sigemptyset(&sa.sa_mask);
	if ( sigaction(SIGINT,&sa,NULL) == -1 ){						// detectar Ctrl+C
		printf("ERROR: No se pudo crear el handler para 'SIGINT'\r\n");
		return 4;
	}

	/*==================[schedulerDispatchTasks()]===================================*/
	if ( conexion == socket){
		if ( tcp_open(_PUERTO_)   ){
			printf ("ERROR tcp_open \n\n");
			return 3;
		}
		switch ( parametros ){
			case servidor:
				printf ( "parametros: servidor \n" );
				//netstat -aon
				if ( tcp_bind_listen()   ){
					printf ("ERROR tcp_bind_listen \n\n");
					return 3;
				}
				if (pthread_create (&pt_conexion_parametros, NULL, tcp_servidor, NULL) != 0 ){
					printf("ERROR: No se pudo crear el pthread para 'comunicacion_productor'\r\n");
					return 5;
				}
				break;

			case cliente:
				printf ( "parametros: cliente \n" );
				printf("E R R O R - No desarrollado...'\r\n");
				break;

			default:
				printf("E R R O R - parametros no desarrollado... \n\n");
				return 6;
				break;
		}
	}

	if (pthread_create (&pt_comunicacion_consumidor, NULL, comunicacion_consumidor, NULL) != 0 ){
		printf("ERROR: No se pudo crear el pthread para 'comunicacion_productor'\r\n");
		return 5;
	}

	if (pthread_create (&pt_comunicacion_productor, NULL, comunicacion_productor, NULL) != 0 ){
		printf("ERROR: No se pudo crear el pthread para 'comunicacion_productor'\r\n");
		return 5;
	}

	switch ( modo ){
		case gateway:
			if (pthread_create (&pt_comunicacion_procesar, NULL, comunicacion_procesar_gateway, NULL) != 0 ){
				printf("ERROR: No se pudo crear el pthread para 'pt_comunicacion_procesar'\r\n");
				return 5;
			}
			break;

		case nodo:
			if (pthread_create (&pt_comunicacion_procesar, NULL, comunicacion_procesar_nodo, NULL) != 0 ){
				printf("ERROR: No se pudo crear el pthread para 'pt_comunicacion_procesar'\r\n");
				return 5;
			}
			break;

		default:
			printf ("E R R O R - Modo no configurado... \n\n");
			return 2;
			break;
	}

	if (pthread_create (&pt_reloj, NULL, reloj, NULL) != 0 ){
		printf("ERROR: No se pudo crear el pthread para 'pt_reloj'\r\n");
		return 5;
	}

	while(1)
	{
        // Salir si se detecta Ctrl+C
        if ( handlerSig != 0 )
        	break;

        usleep(50000);
	}

	// Esperar que se terminen los thread...
	pthread_join (pt_comunicacion_consumidor, NULL);
	pthread_join (pt_comunicacion_productor, NULL);
	pthread_join (pt_comunicacion_procesar, NULL);
	pthread_join (pt_reloj, NULL);

	switch ( parametros ){
		case servidor:
			pthread_join (pt_conexion_parametros, NULL);
			break;
		case cliente:
			break;
	}

	if ( handlerSig == 99 )										// Errores personalizados en cada función
		printf("\r\nERROR: %s\r\n", error);
	else
		printf("\r\nSeñal Ctrl+C detectada, cerrando comunicaciones...\r\n");

	return 0;
}

void handlerSeniales(int sig)
{
    handlerSig = sig;

	switch ( conexion ){
		case teclado_pantalla:
			break;

		case socket:
			if ( tcp_closeFD() != 0 )
				printf ("ERROR tcp_closeFD \n\n");

			tcp_closeSOC();	
			break;

		case radio_frecuencia:
			break;
	}
}

void*
reloj( void* )
{
	while( 1 ){
		if ( handlerSig != 0 && handlerSig != 100 )
        	break;

		//if( debug > 9 )
		//	printf("segActuales=%d\nminActuales=%d\nhorActuales=%d\ndiaActuales=%d\n", segActuales, minActuales, horActuales, diaActuales);

		segActuales++;
		if (segActuales >= 60){
        	minActuales++;
			segActuales = 1;   			// No lo hago cero porque se usa como delay

        }
		if (minActuales >= 60){
			horActuales++;
			minActuales =  0;
        }
		if (horActuales >= 24){
			diaActuales++;
			horActuales =  0;
	    }
		usleep(1000000);				// 1000000 = 1 seg
	}
	printf("salio reloj\r\n");
}
