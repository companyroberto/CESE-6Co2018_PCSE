/*============================================================================
 * Autor:		Roberto Company
 * Fecha:		06/08/2018
 *===========================================================================*/

/*==================[inclusiones]============================================*/

#include <stdio.h>
#include <string.h>

#include "bufferCircular.h"

/*==================[definiciones de funciones internas]=====================*/

uint8_t
leerBuffer( bufferCircular_t *bufferCircular, char *dato, uint8_t bytes )
{
	if ( bufferCircular->lectura == bufferCircular->escritura )
		return 0;									// FALSO: Nada para leer
	else {
		memset( dato, '\0', sizeof(dato) );
		strncpy(dato, bufferCircular->buffer + bufferCircular->lectura - 1, bytes);
		bufferCircular->lectura	= ( bufferCircular->lectura + bytes ) % L_BUFFER;
		return bytes;
	}
}

uint8_t
escribirBuffer( bufferCircular_t *bufferCircular, char *dato, uint8_t bytes )
{
	if ( (bufferCircular->escritura + 1) % L_BUFFER == bufferCircular->lectura )
		return 0;									// FALSO: Buffer lleno
	else {
		strncpy(bufferCircular->buffer + bufferCircular->escritura - 1, dato, bytes);
		bufferCircular->escritura = (bufferCircular->escritura + bytes) % L_BUFFER;
		return bytes;
	}
}
