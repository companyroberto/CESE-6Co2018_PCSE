/*============================================================================
 * Autor:		Roberto Company
 * Fecha:		26/07/2018
 * Descripcion:	Propiedades y métodos propios de la comunicacion
 *===========================================================================*/

/*==================[inclusiones]============================================*/
#include <stdio.h>
#include <string.h>
#include "utiles.h"
#include "mensajeria.h"
#include "comunicacion.h"
#include "sensores.h"

/*==================[macros y definiciones]==================================*/

static int16_t  _SerieIUM = 0;
static int8_t   _VersionPaquete = 0;
static int8_t   _VersionFirmware = 0;
static char     _IDSistemaEmisor[2] = "";
static char     _IDSistemaReceptor[2] = "";
static int8_t   _TimeOut = 0;

static int8_t   _idPuertaEnlaceAsignado = 0;           // IDDispositivoReceptor para los ND
static int8_t   _idNodoAsignado = 0;                   // IDDispositivoEmisor para los ND
static char     _Password[3] = "";
static int8_t   _segTrasmitirDatos = 0;
static int8_t   _segRetrasmitir = 5;

static int16_t  _IDMensaje = 0;

static int16_t  _IDMensajePE = 0;

int (*pFuncion_read)(char * buffer, int bytes);
int (*pFuncion_write)(char * buffer, int bytes);
static bufferCircular_t *buffer_tx;
static bufferCircular_t *buffer_rx;

extern int  handlerSig;
extern char error[127];
extern int  debug;

typedef enum { enINICIADO, enCONFIGURANDO, enCONFIGURADO, enSENSADO } Estados_nodo;
typedef enum { egINICIADO, egPROCESANDO } Estados_gateway;

extern int8_t segActuales;
extern int8_t minActuales;
extern int8_t horActuales;
extern int8_t diaActuales;

/*==================[definiciones de funciones internas]=====================*/

static void comunicacion_inicializar_paquete                ( PaqueteV01_t * paqueteV01 );
void        comunicacion_configurar_paquetes                ( int16_t SerieIUM, int8_t VersionPaquete, int8_t VersionFirmware, char * IDSistemaEmisor, char * IDSistemaReceptor, int8_t TimeOut);
int8_t      comunicacion_solicitar_configuracion            ( PaqueteV01_t * paqueteV01, int8_t Segundos, int8_t Minutos, int8_t Horas );
int8_t      comunicacion_crear_establecer_configuracion     ( PaqueteV01_t * paqueteV01, int16_t SerieIUM );
int8_t      comunicacion_procesar_establecer_configuracion  ( PaqueteV01_t * paqueteV01 );
int8_t      comunicacion_mensaje_recibido                   ( PaqueteV01_t * paqueteV01, int16_t idMensaje, int8_t idError );
int8_t      comunicacion_no_para_mi                         ( PaqueteV01_t * paqueteV01 );
int8_t      comunicacion_fuera_de_secuencia                 ( PaqueteV01_t * paqueteV01 );
int8_t      comunicacion_registrar                          ( int16_t IDMensaje, int8_t segActuales, char * paquete_enviado );
int8_t      comunicacion_desregistrar                       ( int16_t IDMensaje );
int8_t      comunicacion_tomar_pendientes                   ( int8_t segActuales, char * paquete_pendiente );
static void comunicacion_inicializar_paquete                ( PaqueteV01_t * paqueteV01 );
int8_t      comunicacion_datos_sensados                     ( PaqueteV01_t * paqueteV01, int8_t IDSensor, int8_t valor );

///////////////////////////////////////////////////////////////////////////////
// Configuración - Productor - Consumidor  - Procesar                        //
///////////////////////////////////////////////////////////////////////////////

void
comunicacion_inicializar_buffer( int (*_pFuncion_read)(char * buffer, int bytes), int (*_pFuncion_write)(char * buffer, int bytes), bufferCircular_t * _buffer_tx, bufferCircular_t * _buffer_rx )
{
    pFuncion_read  = _pFuncion_read;
    pFuncion_write = _pFuncion_write;

	buffer_tx = _buffer_tx;
	buffer_tx->escritura = 0;
	buffer_tx->lectura   = 0;

	buffer_rx = _buffer_rx;
	buffer_rx->escritura = 0;
	buffer_rx->lectura   = 0;

    	strncpy( _Password, "***", 3);
}

void*
comunicacion_productor( void* parametros )
{
    char tmp[L_PAQUETE] = "";
    while( 1 ){
        if ( handlerSig == 0 ){
            while( leerBuffer( buffer_tx, tmp, L_PAQUETE) ){
                if ( pFuncion_write(tmp, L_PAQUETE) ){
                    printf("Error en pFuncion_write.\r\n");
                    break;
                }
            }
        }
        usleep(1000000);	                                    // 500000 = 0.5 seg     // 1000000 = 1 seg
        if ( handlerSig != 0 && handlerSig != 100 )
        	break;
    }
    printf("salio comunicacion_productor\r\n");
}

void*
comunicacion_consumidor( void* parametros )
{
    char tmp[L_PAQUETE] = "";
    while( 1 ){
        if ( handlerSig == 0 ){
            memset( tmp, '\0', sizeof(tmp) );        
            if ( pFuncion_read(tmp, L_PAQUETE) ){
                printf("Error en pFuncion_read. Posiblemente el cliente se desconecto...\r\n");
                handlerSig = 100;
            }else
                if (!escribirBuffer( buffer_rx, tmp,  L_PAQUETE))
                    printf ("E R R O R: Buffer lleno...\n\r");
            //usleep(500000);	// 0.5seg       // No utilizado porque 'pFuncion_read' es bloqueante
        }else
            usleep(500000);	    // 0.5seg       // Si no hay función bloqueante, se espera 0.5 seg

        // Salir si se detecta Ctrl+C, pero no por error de lectura
        if ( handlerSig != 0 && handlerSig != 100 )
        	break;
    }
    printf("salio comunicacion_consumidor\r\n");
}

void*
comunicacion_procesar_nodo( void* parametros )
{
    static Estados_nodo estado_nodo = enINICIADO;
    PaqueteV01_t paq;
    int8_t r;

    char paquete_recibido[33] = "";		// agrego uno para \0
    char paquete_enviado[33] = "";		// agrego uno para \0

    while ( 1 ){
        if ( handlerSig != 0 )
        	break;

        // Nada que procesar. No hace falta limpiar todo el paquete.
        paq.Encabezado.IdentificadorGrupo = 0;

        if ( leerBuffer( buffer_rx, paquete_recibido, L_PAQUETE) ){
            if ( mensajeria_desarmar( &paq, paquete_recibido ) ){
                printf ("NO OK mensajeria_desarmar: r= %d \n\n", r);
                continue;				// no proceso
            }else
                if ( debug > 1)
                    printf ("OK mensajeria_desarmar: paq.Encabezado.IdentificadorGrupo = %d \n\n", paq.Encabezado.IdentificadorGrupo);

            if ( paq.Encabezado.VersionFirmware != _VersionFirmware )
			    continue;				// no proceso

            if ( comunicacion_no_para_mi( &paq )  ){
                if ( paq.Encabezado.TimeOut-- > 0 ){
                    r = mensajeria_armar( paquete_enviado, paq );
                    if (r)
                        printf ("Salida con error: %d\n\n", r);
                    else
                        if ( debug > 9 )
                            printf ("paquete_reenviado = -%s- \n\r", paquete_enviado);
										// Estos paquetes no se registran porque no son propios

                    if (!escribirBuffer( buffer_tx, paquete_enviado,  L_PAQUETE))
                        printf ("E R R O R: Buffer lleno...\n\r");
                }
                continue;       		// buscar mas mensajes en el buffer
            }
            if ( 0 ){//comunicacion_fuera_de_secuencia( &paq ) ){
                    r = comunicacion_mensaje_recibido( &paq, paq.Encabezado.IDMensaje, r );
                    if (r)
                        printf ("NO (FdS) OK comunicacion_mensaje_recibido: r= %d \n\n", r);
                    else
                        printf ("OK (FdS) comunicacion_mensaje_recibido \n\n");
                
                continue;       		// buscar mas mensajes en el buffer
            }
            //if ( debug > 1)
                printf ("Procesar: paq.Encabezado.IdentificadorGrupo = %d \n\n", paq.Encabezado.IdentificadorGrupo);
        }

        switch (estado_nodo){           
            case enINICIADO:
                r = comunicacion_solicitar_configuracion( &paq, segActuales, minActuales, horActuales );
                if (r)
                    printf ("NO OK comunicacion_solicitar_configuracion: r= %d \n\n", r);
                else
                    if ( debug > 1)
                        printf ("OK comunicacion_solicitar_configuracion \n\n");

                r = mensajeria_armar( paquete_enviado, paq );
                if (r)
                    printf ("Salida con error: %d\n\n", r);
                else
                    if ( debug > 9)
                        printf ("paquete_enviado = -%s- \n\r", paquete_enviado);

                r = comunicacion_registrar(paq.Encabezado.IDMensaje, segActuales, paquete_enviado);
                if (r)
                    printf ("NO OK comunicacion_registrar: r= %d \n\n", r);
                else
                    if ( debug > 9)
                        printf ("OK comunicacion_registrar \n\n");

                if (!escribirBuffer( buffer_tx, paquete_enviado,  L_PAQUETE))
                    printf ("E R R O R: Buffer lleno...\n\r");

                estado_nodo = enCONFIGURANDO;
                if ( debug > 1)
                    printf ("estado_comunicacion = ecCONFIGURANDO \n\n");
                break;

            case enCONFIGURANDO:
                if ( paq.Encabezado.IdentificadorGrupo == 3){
                    r = comunicacion_procesar_establecer_configuracion ( &paq );
                    r = comunicacion_desregistrar( _IDMensaje );            // Configurar es el primer mensaje y no recibo confirmacion

                    if ( sensor_inicializar_hardware( _segTrasmitirDatos ) )
                        printf ("E R R O R: Inicializando hardware...\n\r");

                    estado_nodo = enCONFIGURADO;
                    if ( debug > 1)
                        printf ("estado_comunicacion = ecCONFIGURADO \n\n");
                }
                break;

            case enCONFIGURADO:
                r = 0;
                switch (paq.Encabezado.IdentificadorGrupo)
                {
                    case 0:
                        // Nada que procesar
                        break;

                    case 1:
                        r = comunicacion_desregistrar( paq.Grupo1.idMensaje );
                        if ( r )
                            printf( "Mensaje no encontrado en la cola: %d\n", paq.Grupo1.idMensaje );
                        continue;       // buscar mas mensajes en el buffer

                    case 3:
                        if (comunicacion_procesar_establecer_configuracion ( &paq ) )
                        {
                            // El GATEWAY perdió la configuración, re-configurar (Reinicio la maquina de estados para los ND)
                            estado_nodo = enINICIADO;
                            continue;   // buscar mas mensajes en el buffer
                        }
                        if ( sensor_inicializar_hardware( _segTrasmitirDatos ) )
                            printf ("E R R O R: Inicializando hardware...\n\r");
                        else
                            if ( debug > 1)
                                printf ( "Hardware reconfigurado! \n" );

                        continue;       // buscar mas mensajes en el buffer

                    default:
										//case 2:	No debería recibir nunca un Grupo2
										//case 4:	No debería recibir nunca un Grupo4
                        r = 3;   		// Numero incorrecto de grupo. No proceso
                        break;
                }

                if (r)
                    printf ("E R R O R - Procesar mensaje: r= %d \n\n", r);

                if ( sensor_chequear( segActuales ) )
                    estado_nodo = enSENSADO;

                break;

            case enSENSADO:
                ;
                Sensor_t sensor;
                while (sensor_leer_dato( &sensor )){
                    r = comunicacion_datos_sensados( &paq, sensor.IDSensor, sensor.valor );
                    r = mensajeria_armar( paquete_enviado, paq );
                    if (r)
                        printf ("NO mensajeria_armar: %d\n\n", r);
                    else
                        if ( debug > 1)
                            printf ("SI mensajeria_armar= -%s- \n\r", paquete_enviado);
                    
                    if (!escribirBuffer( buffer_tx, paquete_enviado,  L_PAQUETE))
                        printf ("E R R O R: Buffer lleno...\n\r");

                    r = comunicacion_registrar(paq.Encabezado.IDMensaje, segActuales, paquete_enviado);
                    if (r)
                        printf ("NO OK comunicacion_registrar: r= %d \n\n", r);
                    else
                        printf ("OK comunicacion_registrar \n\n");
                }

                estado_nodo = enCONFIGURADO;
                break;

            default:
                estado_nodo = enINICIADO;		// Condicion de error, reinicio la maquina de estados
                break;
        }

        while (! comunicacion_tomar_pendientes( segActuales, paquete_enviado ) ){
            if (!escribirBuffer( buffer_tx, paquete_enviado,  L_PAQUETE))
                printf ("E R R O R: Buffer lleno...\n\r");
            //usleep(100000);	                // 100000 = 0.1 seg
        } 

        usleep(1000000);	                    // 500000 = 0.5 seg     // 1000000 = 1 seg
    }
    printf("salio comunicacion_procesar_nodo\r\n");
}

void*
comunicacion_procesar_gateway( void* parametros )
{
    static Estados_gateway estado_gateway = egINICIADO;
    
    PaqueteV01_t paq;
    int8_t r;
    int8_t idError = 0;

    char paquete_recibido[33] = "";				// agrego uno para \0
    char paquete_enviado[33] = "";				// agrego uno para \0

    while ( 1 ){
        if ( handlerSig != 0 )
        	break;

        // Nada que procesar. No hace falta limpiar todo el paquete.
        paq.Encabezado.IdentificadorGrupo = 0;

        // capa de transporte
        if ( leerBuffer( buffer_rx, paquete_recibido, L_PAQUETE) ){
            if ( mensajeria_desarmar( &paq, paquete_recibido ) ){
                printf ("NO OK mensajeria_desarmar: r= %d \n\n", r);
                continue;						// no proceso
            }else
                if ( debug > 1)
                    printf ("OK mensajeria_desarmar: paq.Encabezado.IdentificadorGrupo = %d \n\n", paq.Encabezado.IdentificadorGrupo);

            if ( paq.Encabezado.VersionFirmware != _VersionFirmware )
			    continue;						// no proceso

            if ( comunicacion_no_para_mi( &paq )  ){
                continue;						// el gateway no re-trasmite...
            }
            if ( 0 ){							//comunicacion_fuera_de_secuencia( &paq ) ){
                    r = comunicacion_mensaje_recibido( &paq, paq.Encabezado.IDMensaje, r );
                    if (r)
                        printf ("NO (FdS) OK comunicacion_mensaje_recibido: r= %d \n\n", r);
                    else
                        printf ("OK (FdS) comunicacion_mensaje_recibido \n\n");
                
                continue;   					// buscar mas mensajes en el buffer
            }
            if ( debug > 1)
                printf ("Procesar: paq.Encabezado.IdentificadorGrupo = %d \n\n", paq.Encabezado.IdentificadorGrupo);
        }

        // procesado de paquetes
        switch (estado_gateway){
            case egINICIADO:

                _idPuertaEnlaceAsignado = 1;
                _idNodoAsignado = 1;
                _segTrasmitirDatos = 20;
                _segRetrasmitir = 5;

                // Como no se si vengo de un reinicio, pido reconfigurar a los nodos...
                r = comunicacion_crear_establecer_configuracion ( &paq, 0 );
                if ( mensajeria_armar( paquete_enviado, paq ) )
                    printf ("NO mensajeria_armar: %d\n\n", r);
                else
                    if ( debug > 1)
                        printf ("SI mensajeria_armar= -%s- \n\r", paquete_enviado);
                
                if (!escribirBuffer( buffer_tx, paquete_enviado,  L_PAQUETE))
                    printf ("E R R O R: Buffer lleno...\n\r");

                estado_gateway = egPROCESANDO;
                //if ( debug > 1)
                    printf ("estado_gateway = egPROCESANDO \n\n");
                break;

            case egPROCESANDO:
                r = 0;
                switch (paq.Encabezado.IdentificadorGrupo)
                {
                    case 0:
                        // Nada que procesar
                        break;

                    case 1:
                        r = comunicacion_desregistrar( paq.Grupo1.idMensaje );
                        if ( r )
                            printf( "Mensaje no encontrado en la cola: %d\n", paq.Grupo1.idMensaje );
                        continue;       // buscar mas mensajes en el buffer

                    case 2:
										// Aca debería consultar con otros GW en la misma red antes de asignar ID a nodo por si otro ya le respondió para que los 2 GW no le contesten al nodo.
                        r = comunicacion_crear_establecer_configuracion ( &paq, paq.Grupo2.SerieIUM );
                        if ( mensajeria_armar( paquete_enviado, paq ) )
                            printf ("NO mensajeria_armar: %d\n\n", r);
                        else
                            if ( debug > 1)
                                printf ("SI mensajeria_armar= -%s- \n\r", paquete_enviado);
                        
                        if (!escribirBuffer( buffer_tx, paquete_enviado,  L_PAQUETE))
                            printf ("E R R O R: Buffer lleno...\n\r");

                        continue;       // buscar mas mensajes en el buffer

                    case 4:
                        ;// Registrar el dato sensado en algún lado y además trasmitir al servidor central.
                        r = comunicacion_mensaje_recibido( &paq, paq.Encabezado.IDMensaje, idError );
                        r = mensajeria_armar( paquete_enviado, paq );
                        if (r)
                            printf ("NO mensajeria_armar: %d\n\n", r);
                        else
                            if ( debug > 1)
                                printf ("SI mensajeria_armar= -%s- \n\r", paquete_enviado);
                        
                        if (!escribirBuffer( buffer_tx, paquete_enviado,  L_PAQUETE))
                            printf ("E R R O R: Buffer lleno...\n\r");

                        break;

                    default:
                        //case 3:		// No debería recibir nunca un Grupo3
                        r = 3;   		// Numero incorrecto de grupo. No proceso
                        break;
                }

                if (r)
                    printf ("E R R O R - Procesar mensaje: r= %d \n\n", r);

                break;

            default:
                estado_gateway = egINICIADO;			// Condicion de error, reinicio la maquina de estados
                break;
        }

        while (! comunicacion_tomar_pendientes( segActuales, paquete_enviado ) ){
            if (!escribirBuffer( buffer_tx, paquete_enviado,  L_PAQUETE))
                printf ("E R R O R: Buffer lleno...\n\r");
            //usleep(100000);	                        // 100000 = 0.1 seg
        } 

        usleep(1000000);	                            // 500000 = 0.5 seg     // 1000000 = 1 seg
    }
    printf("salio comunicacion_procesar_gateway\r\n");
}

void*
comunicacion_procesar_gateway_pruebas()
{
    while( 1 )
    {
        // Salir si se detecta Ctrl+C
        if ( handlerSig != 0 )
        	break;

        printf("comunicacion_procesar_gateway\r\n");
        usleep(1000000);	// 1 seg
    }
    printf("salio comunicacion_procesar_gateway\r\n");
}


///////////////////////////////////////////////////////////////////////////////
// Metodos para trabajar con la mensajería                                   //
///////////////////////////////////////////////////////////////////////////////


void
comunicacion_inicializar_pendientes                 ( )
{
    int i=0;
    for( ; i<L_COMUNICACION_PENDIENTES; i++ ){
        //printf ("i = %d \n\n", i);
        mensajesPendientes[i].IDMensaje = 0;
        strncpy(mensajesPendientes[i].paquete_enviado, "\0", 33);
    }
}

void
comunicacion_configurar_paquetes                    ( int16_t SerieIUM, int8_t VersionPaquete, int8_t VersionFirmware, char * IDSistemaEmisor, char * IDSistemaReceptor, int8_t TimeOut)
{
    _SerieIUM = SerieIUM;
    _VersionPaquete = VersionPaquete;
    _VersionFirmware = VersionFirmware;
    strncpy( _IDSistemaEmisor, IDSistemaEmisor, 2 );
    strncpy( _IDSistemaReceptor, IDSistemaReceptor, 2 );
    _TimeOut = TimeOut;
}

int8_t
comunicacion_solicitar_configuracion                ( PaqueteV01_t * paqueteV01, int8_t Segundos, int8_t Minutos, int8_t Horas )
{
    int8_t resultado = 0;

    comunicacion_inicializar_paquete( paqueteV01 );
    paqueteV01->Encabezado.IdentificadorGrupo = 2;
    paqueteV01->Grupo2.Segundos = Segundos;
    paqueteV01->Grupo2.Minutos = Minutos;
    paqueteV01->Grupo2.Horas = Horas;
    paqueteV01->Grupo2.SerieIUM = _SerieIUM;
    

    return resultado;
}

int8_t
comunicacion_crear_establecer_configuracion         ( PaqueteV01_t * paqueteV01, int16_t SerieIUM )
{
    int8_t resultado = 0;

    comunicacion_inicializar_paquete( paqueteV01 );

    paqueteV01->Encabezado.IDDispositivoReceptor = 0;// Siempre envío nodo = 0 en destino para configurar.

    paqueteV01->Encabezado.IdentificadorGrupo   = 3;

    if ( SerieIUM == 0 ){
        paqueteV01->Grupo3.SerieIUM                 = 0;
        paqueteV01->Grupo3.idPuertaEnlaceAsignado   = 0;
        paqueteV01->Grupo3.idNodoAsignado           = 0;
        paqueteV01->Grupo3.segTrasmitirDatos        = 0;
        paqueteV01->Grupo3.segRetrasmitir           = 0;
    }else{
        paqueteV01->Grupo3.SerieIUM                 = SerieIUM;
        paqueteV01->Grupo3.idPuertaEnlaceAsignado   = _idPuertaEnlaceAsignado;
        paqueteV01->Grupo3.idNodoAsignado           = _idNodoAsignado;          // debería leer un a tabla
        paqueteV01->Grupo3.segTrasmitirDatos        = _segTrasmitirDatos;
        paqueteV01->Grupo3.segRetrasmitir           = _segRetrasmitir;
    }

    return resultado;
}

int8_t
comunicacion_procesar_establecer_configuracion      ( PaqueteV01_t * paqueteV01 )
{
    int8_t resultado = 15;                          // El GATEWAY perdió la configuración, re-configurar

    if ( paqueteV01->Grupo3.idPuertaEnlaceAsignado == _idPuertaEnlaceAsignado &&
         paqueteV01->Grupo3.idNodoAsignado         == 0 ){
             _idPuertaEnlaceAsignado = 0;
             _idNodoAsignado         = 0;
        }
    else{
        _idPuertaEnlaceAsignado = paqueteV01->Grupo3.idPuertaEnlaceAsignado;
        _idNodoAsignado         = paqueteV01->Grupo3.idNodoAsignado;
        _segTrasmitirDatos      = paqueteV01->Grupo3.segTrasmitirDatos;
        _segRetrasmitir         = paqueteV01->Grupo3.segRetrasmitir;

    	strncpy( _Password, "***", 3);

        resultado = 0;                              // Configurado
    }

    return resultado;
}

int8_t
comunicacion_mensaje_recibido                       ( PaqueteV01_t * paqueteV01, int16_t idMensaje, int8_t idError )
{
    int8_t resultado = 0;

    comunicacion_inicializar_paquete( paqueteV01 );
    paqueteV01->Encabezado.IdentificadorGrupo = 1;
    paqueteV01->Grupo1.idMensaje = idMensaje;
    paqueteV01->Grupo1.idError = idError;

    return resultado;
}

int8_t
comunicacion_datos_sensados                         ( PaqueteV01_t * paqueteV01, int8_t IDSensor, int8_t valor )
{
    int8_t resultado = 0;

    comunicacion_inicializar_paquete( paqueteV01 );
    paqueteV01->Encabezado.IdentificadorGrupo = 4;
    paqueteV01->Grupo4.idSensor = IDSensor;
    paqueteV01->Grupo4.Valor = valor;

    return resultado;
}

int8_t
comunicacion_no_para_mi                             ( PaqueteV01_t * paqueteV01 )
{
    /// Falta validar el Password = "***";
    int8_t resultado = 14;                                                      // Mensaje para otro destinatario
    while (1){

        char dest[4];
        if ( strncmp( subChar( dest, paqueteV01->Encabezado.IDSistemaReceptor, 1, 2 ), _IDSistemaEmisor, 2 ) != 0 ){
            break;                                                              // El sistema destinatario no es mi tipo
        }

        if ( paqueteV01->Encabezado.IDDispositivoReceptor == 0 &&
             paqueteV01->Encabezado.IdentificadorGrupo    == 3 &&
             paqueteV01->Grupo3.SerieIUM                  == _SerieIUM
            ){
            resultado = 0;                                                      // Es configuracion inicial enviada de un GW
            break;
        }

        if ( paqueteV01->Encabezado.IDDispositivoReceptor == 0 &&
             paqueteV01->Encabezado.IdentificadorGrupo    == 2
            ){
            resultado = 0;                                                      // Es configuracion inicial enviada de un ND
            break;
        }
        if ( strncmp( subChar( dest, paqueteV01->Encabezado.IDSistemaEmisor, 1, 2 ), _IDSistemaReceptor, 2 ) == 0   &&
                                     paqueteV01->Encabezado.IDDispositivoEmisor   == _idPuertaEnlaceAsignado        &&
                                     paqueteV01->Encabezado.IDDispositivoReceptor == _idNodoAsignado
            ){
            resultado = 0;                                                      // Es muy para mi
            break;
        }
        break;
    }

    return resultado;
}

int8_t
comunicacion_fuera_de_secuencia                     ( PaqueteV01_t * paqueteV01 )
{
    int8_t resultado = 0;

    if ( _IDMensajePE >= paqueteV01->Encabezado.IDMensaje )
            resultado = 15;                                                     // Mensaje fuera de secuencia
    else
        _IDMensajePE = paqueteV01->Encabezado.IDMensaje;                        // _IDMensajePE == 0 o el anterior

    return resultado;
}

int8_t
comunicacion_registrar                              ( int16_t IDMensaje, int8_t segActuales, char * paquete_enviado )
{
    int8_t resultado = 10;              		// Cola de mensajes pendientes lleno

    int8_t i=0;
    while( i < L_COMUNICACION_PENDIENTES ){
        //printf ("i = %d \n\n", i);        
        if ( mensajesPendientes[i].IDMensaje == 0 ){
            mensajesPendientes[i].IDMensaje = IDMensaje;
            mensajesPendientes[i].segTrasmitido = segActuales;
            strncpy(mensajesPendientes[i].paquete_enviado, paquete_enviado, 33);
            resultado = 0;
            break;
        }else{
            if ( mensajesPendientes[i].IDMensaje == IDMensaje ){
                resultado = 11;         		// Mensaje ya guardado en la cola
                break;
            }
        }
        i++;
    }

    return resultado;
}

int8_t
comunicacion_desregistrar                           ( int16_t IDMensaje )
{
    int8_t resultado = 12;              		// Mensaje no encontrado en la cola

    int8_t i=0;
    while( i < L_COMUNICACION_PENDIENTES ){
        if ( mensajesPendientes[i].IDMensaje == IDMensaje ){
            mensajesPendientes[i].IDMensaje = 0;
            strncpy(mensajesPendientes[i].paquete_enviado, "\0", 33);
            resultado = 0;
            break;
        }
        i++;
    }

    return resultado;
}

int8_t
comunicacion_tomar_pendientes                       ( int8_t segActuales, char * paquete_pendiente )
{
    int8_t resultado = 13;                          // No hay mensajes pendientes

    int8_t i=0;
    while( i < L_COMUNICACION_PENDIENTES ){
        if ( mensajesPendientes[i].IDMensaje != 0 ){
            int8_t diferencia = (mensajesPendientes[i].segTrasmitido > segActuales) ? mensajesPendientes[i].segTrasmitido - segActuales : segActuales - mensajesPendientes[i].segTrasmitido;
            if ( diferencia >= _segRetrasmitir ){
                strncpy(paquete_pendiente, mensajesPendientes[i].paquete_enviado, 33);
                mensajesPendientes[i].segTrasmitido = segActuales;
                resultado = 0;
                break;
            }
        }
        i++;
    }

    return resultado;
}

static
void
comunicacion_inicializar_paquete                    ( PaqueteV01_t * paqueteV01 )
{
    // Ini semaforos
    _IDMensaje++;
    // Fin semaforos

    memset( paqueteV01, 0, sizeof(paqueteV01) );

    paqueteV01->Encabezado.VersionPaquete = _VersionPaquete;
    paqueteV01->Encabezado.VersionFirmware = _VersionFirmware;
    strncpy( paqueteV01->Encabezado.IDSistemaEmisor, _IDSistemaEmisor, 2 );
    paqueteV01->Encabezado.IDDispositivoEmisor = _idNodoAsignado;
    strncpy( paqueteV01->Encabezado.IDSistemaReceptor, _IDSistemaReceptor, 2 );
    paqueteV01->Encabezado.IDDispositivoReceptor = _idPuertaEnlaceAsignado;
    paqueteV01->Encabezado.IDMensaje = _IDMensaje;
    paqueteV01->Encabezado.TimeOut = _TimeOut;
    strncpy(paqueteV01->Encabezado.Password, _Password, 3);
}
