/*============================================================================
 * Autor:		Roberto Company
 * Fecha:		26/07/2018
 * Descripcion:	Propiedades y métodos propios de la mensajería
 *===========================================================================*/

#include <stdlib.h>

#ifndef _MENSAJERIA_H_
#define _MENSAJERIA_H_

#define L_PAQUETE         				32

typedef struct {
	int8_t VersionPaquete;              // 01
	int8_t VersionFirmware;             // 01
	char IDSistemaEmisor[2];            // PE / ND
	int8_t IDDispositivoEmisor;         // 01
	char IDSistemaReceptor[2];          // PE / ND
	int8_t IDDispositivoReceptor;       // 01
	int16_t IDMensaje;                  // 1012
	int8_t TimeOut;                     // 5
	char Password[3];                   // ***
	int8_t IdentificadorGrupo;          // 2
} Encabezado_t;

typedef struct {
	int16_t idMensaje;                  // 1012
	int8_t idError;                     // 00
} Grupo1_t;

typedef struct {
	int8_t Segundos;                    // 59
	int8_t Minutos;                     // 59
	int8_t Horas;                       // 44
	int16_t SerieIUM;                   // 0012
} Grupo2_t;

typedef struct {
	int16_t SerieIUM;                   // 0012
	int8_t idPuertaEnlaceAsignado;      // 01
	int8_t idNodoAsignado;              // 01
	int8_t segTrasmitirDatos;         	// 30
	int8_t segRetrasmitir;              // 2
} Grupo3_t;

typedef struct {
	int8_t idSensor;                    // 01
	int16_t Valor;                      // 001
} Grupo4_t;

typedef struct {
	Encabezado_t Encabezado;
	Grupo1_t Grupo1;
    Grupo2_t Grupo2;
    Grupo3_t Grupo3;
    Grupo4_t Grupo4;
} PaqueteV01_t;

int8_t mensajeria_desarmar							( PaqueteV01_t * paqueteV01, char * paquete_recibido );
int8_t mensajeria_armar								( char *  paquete_enviado, PaqueteV01_t   paqueteV01 );

#endif
