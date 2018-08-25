/*============================================================================
 * Autor:		Roberto Company
 * Fecha:		12/08/2018
 * Descripcion:	Métodos de propósito general
 *===========================================================================*/

/*==================[inclusiones]============================================*/
#include <stdio.h>
#include "sensores.h"

/*==================[macros y definiciones]==================================*/

#define N_SENSORES       1                          			// Solo un sensor usado para temperatura

static Sensor_t sensor_conectado[N_SENSORES];

static int8_t segTrasmitirDatos;

int8_t IDSensor = 0;


/*==================[definiciones de funciones internas]=====================*/

void    sensor_inicializar( Sensor_t * Sensor, int8_t pin);		// gpioMap_t pin = 1


///////////////////////////////////////////////////////////////////////////////
// Metodos publicos                                                          //
///////////////////////////////////////////////////////////////////////////////


int8_t
sensor_inicializar_hardware
( int8_t _segTrasmitirDatos  )
{
    int8_t resultado = 14;                          			// Error al inicializar hardware

    segTrasmitirDatos = _segTrasmitirDatos;
    sensor_inicializar( &sensor_conectado[0], 1 );  			// gpioMap_t pin = 1

    resultado = 0;                                  			// Sin errores

    return resultado;
}

int8_t
sensor_chequear
(  int8_t segActuales )
{
    static int8_t pruebas = 30;                     			// pruebas

    int8_t resultado = 0;                           			// No hay datos

    int8_t i=0;
    while( i < N_SENSORES ){
        int8_t diferencia = (sensor_conectado[i].segTrasmitido > segActuales) ? sensor_conectado[i].segTrasmitido - segActuales : segActuales - sensor_conectado[i].segTrasmitido;

        if ( diferencia >= segTrasmitirDatos ){
            sensor_conectado[i].valor = pruebas++;				// debug
            sensor_conectado[i].estado = Sensado;
            sensor_conectado[i].segTrasmitido = segActuales;
            resultado = 1;                          			// Hay datos
        }
        i++;
    }

    return resultado;
}

int8_t
sensor_leer_dato
( Sensor_t * sensor )
{
    int8_t resultado = 0;                           			// No hay datos

    int8_t i=0;
    while( i < N_SENSORES ){
        if ( sensor_conectado[i].estado == Sensado ){
            sensor->IDSensor        = sensor_conectado[i].IDSensor;
            sensor->pin             = sensor_conectado[i].pin;
            sensor->estado          = sensor_conectado[i].estado;
            sensor->valor           = sensor_conectado[i].valor;
            sensor->segTrasmitido   = sensor_conectado[i].segTrasmitido;
            resultado = 1;                          			// Hay datos

            sensor_conectado[i].estado = sinSensar;
            break;
        }
        i++;
    }
    return resultado;
}


///////////////////////////////////////////////////////////////////////////////
// Metodos privados                                                          //
///////////////////////////////////////////////////////////////////////////////


void
sensor_inicializar
( Sensor_t * Sensor, int8_t pin  )                      // gpioMap_t pin;
{
    IDSensor++;

    Sensor->IDSensor = IDSensor;
    Sensor->pin = pin;
    Sensor->estado =  sinSensar;
    Sensor->valor = 0;
    Sensor->segTrasmitido = 0;
}