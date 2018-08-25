/*============================================================================
 * Autor:		Roberto Company
 * Fecha:		26/07/2018
 * Descripcion:	Propiedades y métodos propios de la comunicacion
 *===========================================================================*/

/*==================[inclusiones]============================================*/
#include <stdio.h>
#include <string.h>
#include "utiles.h"

/*==================[macros y definiciones]==================================*/

/*==================[definiciones de funciones internas]=====================*/


///////////////////////////////////////////////////////////////////////////////
// Metodos para realizar conversiones y busquedas                            //
///////////////////////////////////////////////////////////////////////////////

char*
subChar
(char *dest, char *string, int pos, int lon)
{
    memset( dest, 0, sizeof(dest) );
    strncpy(dest, string + pos - 1, lon);
    dest[lon] = '\0';

    return dest;
}

char*
intToChar
(char *dest, int num, int dig)
{
    memset( dest, 0, sizeof(dest) );
    sprintf(dest,"%0*d\n\r", dig, num);
    return dest;
}
