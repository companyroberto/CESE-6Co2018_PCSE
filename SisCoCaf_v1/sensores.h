/*============================================================================
 * Autor:		Roberto Company
 * Fecha:		12/08/2018
 * Descripcion:	Métodos de propósito general
 *===========================================================================*/

#include <stdlib.h>

#ifndef _SENSORES_H_
#define _SENSORES_H_

    enum sensor_modo {
        Sensado = 1,
        sinSensar = 2
    };
    typedef struct {
        int8_t IDSensor;                    	// 01
        int8_t pin;                         	// gpioMap_t pin;
        int8_t estado;                      	// Sensado = Hay Datos / Sin Sensar = No hay datos.
        int8_t valor;                       	// 30 = 30 grados Centigrados.
        int8_t segTrasmitido;               	// Segundos de reloj al momento de trasmitir.
    } Sensor_t;


    int8_t  sensor_inicializar_hardware         ( int8_t segTrasmitirDatos );
    int8_t  sensor_chequear                     ( int8_t segActuales );
    int8_t  sensor_leer_dato                    ( Sensor_t * sensor );

#endif
