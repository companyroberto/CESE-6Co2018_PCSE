/*============================================================================
 * Autor:		Roberto Company
 * Fecha:		08/08/2018
 * Descripcion:	Conexión de entrada y salida por teclado y pantalla
 *===========================================================================*/

#ifndef _CONEXIONTP_H_
#define _CONEXIONTP_H_

    int tp_read ( char * buffer, int bytes );
    int tp_write( char * buffer, int bytes );

#endif