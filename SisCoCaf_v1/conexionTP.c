/*============================================================================
 * Autor:		Roberto Company
 * Fecha:		08/08/2018
 * Descripcion:	Conexión de entrada y salida por teclado y pantalla
 *===========================================================================*/

/*==================[inclusiones]============================================*/
#include <stdio.h>
#include <string.h>
#include "conexionTP.h"

/*==================[macros y definiciones]==================================*/

/*==================[definiciones de funciones internas]=====================*/


///////////////////////////////////////////////////////////////////////////////
// Metodos para realizar lectura y escritura                                 //
///////////////////////////////////////////////////////////////////////////////

int tp_read(char* buffer, int bytes)
{
// tcp_read es bloqueante y este también...
// se ingresa una cadena y luego enter (se graba solo la cadena sin el \n)

	char tmp[bytes+1];
	memset( tmp, '\0', sizeof(tmp) );
	fflush(stdout);
	scanf("%s", tmp);
	strncpy( buffer, tmp, bytes );

	return 0;
}

int tp_write(char* buffer, int bytes)
{
    char tmp[bytes+1];
	memset( tmp, '\0', sizeof(tmp) );
	strncpy( tmp, buffer, bytes );
    printf ("%s\n\r", tmp);
	
	return 0;
}