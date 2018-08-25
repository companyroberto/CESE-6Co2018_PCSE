/*============================================================================
 * Autor:		Roberto Company
 * Fecha:		06/08/2018
 *===========================================================================*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#ifndef _BUFFER_CIRCULAR_H_
#define _BUFFER_CIRCULAR_H_

#define L_BUFFER 3200				// Capacidad para 100 mensajes de 32 bytes

typedef struct {
	char buffer[L_BUFFER];
	uint8_t	lectura;
	uint8_t	escritura;
} bufferCircular_t;


uint8_t leerBuffer 			( bufferCircular_t * bufferCircular, char * dato, uint8_t bytes );
uint8_t escribirBuffer 		( bufferCircular_t * bufferCircular, char * dato, uint8_t bytes );

#endif
