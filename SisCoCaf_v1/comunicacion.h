/*============================================================================
 * Autor:		Roberto Company
 * Fecha:		30/07/2018
 * Descripcion:	Propiedades y métodos propios de la comunicacion
 *===========================================================================*/

#include <stdlib.h>
#include <unistd.h>
#include "mensajeria.h"

#ifndef _COMUNICACION_H_
#define _COMUNICACION_H_

    #include "bufferCircular.h"     
    void comunicacion_inicializar_buffer                ( int (*pFuncion_read)(char * buffer, int bytes), int (*pFuncion_write)(char * buffer, int bytes), bufferCircular_t * buffer_tx, bufferCircular_t * buffer_rx );
    void* comunicacion_productor                        ( void* );
    void* comunicacion_consumidor                       ( void* );
    void* comunicacion_procesar_nodo                    ( void* );
    void* comunicacion_procesar_gateway                 ( void* );


    #define L_COMUNICACION_PENDIENTES       30

    typedef struct {
        int16_t IDMensaje;                  // 1012
        int8_t segTrasmitido;               // Segundos de reloj al momento de trasmitir
        char paquete_enviado[33];   		// Mensaje completo
    } msgPendientes_t;

    static msgPendientes_t mensajesPendientes[L_COMUNICACION_PENDIENTES];

    void   comunicacion_inicializar_pendientes          ( );
    void   comunicacion_configurar_paquetes             ( int16_t SerieIUM, int8_t VersionPaquete, int8_t VersionFirmware, char * IDSistemaEmisor, char * IDSistemaReceptor, int8_t TimeOut);
    int8_t comunicacion_mensaje_recibido                ( PaqueteV01_t * paqueteV01, int16_t idMensaje, int8_t idError );
    int8_t comunicacion_solicitar_configuracion         ( PaqueteV01_t * paqueteV01, int8_t Segundos, int8_t Minutos, int8_t Horas );
    int8_t comunicacion_establecer_configuracion        ( PaqueteV01_t * paqueteV01 );
    int8_t comunicacion_registrar           		    ( int16_t IDMensaje, int8_t segTrasmitido, char * paquete_enviado );
    int8_t comunicacion_desregistrar           		    ( int16_t IDMensaje );
    int8_t comunicacion_tomar_pendientes                ( int8_t segActuales, char * paquete_pendiente );

#endif
