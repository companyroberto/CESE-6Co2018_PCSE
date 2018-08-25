/*============================================================================
 * Autor:		Roberto Company
 * Fecha:		26/07/2018
 * Descripcion:	Propiedades y métodos propios de la mensajería
 *===========================================================================*/

/*==================[inclusiones]============================================*/
#include <stdio.h>
#include <string.h>
#include "mensajeria.h"
#include "utiles.h"

/*==================[macros y definiciones]==================================*/
#define L_PAQUETE_CORTO         26      // Encabezado con el grupo mas corto
#define L_PAQUETE_LARGO         32      // Encabezado con el grupo mas largo
#define L_PAQUETE_ENCABEZADO    21      // Solo Encabezado
#define L_PAQUETE_GRUPO1        6       // Solo Grupo1
#define L_PAQUETE_GRUPO2        10      // Solo Grupo2
#define L_PAQUETE_GRUPO3        11      // Solo Grupo3
#define L_PAQUETE_GRUPO4        5       // Solo Grupo4

/*==================[definiciones de funciones internas]=====================*/
static void   mensajeria_desarmar_encabezado        ( PaqueteV01_t * paqueteV01, char * paquete_recibido  );
static int8_t mensajeria_desarmar_grupo1            ( PaqueteV01_t * paqueteV01, char * paquete_recibido  );
static int8_t mensajeria_desarmar_grupo2            ( PaqueteV01_t * paqueteV01, char * paquete_recibido  );
static int8_t mensajeria_desarmar_grupo3            ( PaqueteV01_t * paqueteV01, char * paquete_recibido  );
static int8_t mensajeria_desarmar_grupo4            ( PaqueteV01_t * paqueteV01, char * paquete_recibido  );

static int8_t mensajeria_armar_encabezado           ( char * paquete_enviado   , PaqueteV01_t  paqueteV01 );
static int8_t mensajeria_armar_grupo1               ( char * paquete_enviado   , PaqueteV01_t  paqueteV01 );
static int8_t mensajeria_armar_grupo2               ( char * paquete_enviado   , PaqueteV01_t  paqueteV01 );
static int8_t mensajeria_armar_grupo3               ( char * paquete_enviado   , PaqueteV01_t  paqueteV01 );
static int8_t mensajeria_armar_grupo4               ( char * paquete_enviado   , PaqueteV01_t  paqueteV01 );


////////////////////////////////////////////////////////////////////////////////////////
// Metodos para desarmar el paquete recibido                                          //
////////////////////////////////////////////////////////////////////////////////////////


int8_t
mensajeria_desarmar                                 ( PaqueteV01_t * paqueteV01, char * paquete_recibido )
{
    int8_t resultado = 0;
    memset( paqueteV01, 0, sizeof(paqueteV01) );

    while(1)
    {
        // El paquete_recibido debe tener el fin de cadena \0
        if (strlen(paquete_recibido) < L_PAQUETE_CORTO || strlen(paquete_recibido) > L_PAQUETE_LARGO ){
            resultado = 1;              // Longitud del paquete incorrecta
            break;
        }

        char dest[4];
        if ( strncmp( subChar( dest, paquete_recibido, 1, 2 ), "01", 2 ) != 0 ){
            resultado = 2;              // Version incorrecta de paquete
            break;
        }

        mensajeria_desarmar_encabezado ( paqueteV01, paquete_recibido );

        switch (paqueteV01->Encabezado.IdentificadorGrupo)
        {
            case 1: resultado = mensajeria_desarmar_grupo1 ( paqueteV01, paquete_recibido ); break;
            case 2:	resultado = mensajeria_desarmar_grupo2 ( paqueteV01, paquete_recibido ); break;
            case 3:	resultado = mensajeria_desarmar_grupo3 ( paqueteV01, paquete_recibido ); break;
            case 4:	resultado = mensajeria_desarmar_grupo4 ( paqueteV01, paquete_recibido ); break;

            default:
                resultado = 3;   // Numero incorrecto de grupo
                break;
        }
        break;
    }
    return resultado;
}

static
void
mensajeria_desarmar_encabezado                      ( PaqueteV01_t * paqueteV01, char * paquete_recibido )
{
    int pos = 1;
    int lon = 0;
    char dest[8];

    lon = 2; paqueteV01->Encabezado.VersionPaquete           = atoi(subChar( dest, paquete_recibido,pos,lon ));        pos += lon;
	lon = 2; paqueteV01->Encabezado.VersionFirmware          = atoi(subChar( dest, paquete_recibido,pos,lon ));        pos += lon;
	lon = 2; strncpy(paqueteV01->Encabezado.IDSistemaEmisor,        subChar( dest, paquete_recibido,pos,lon ),lon );   pos += lon;
	lon = 2; paqueteV01->Encabezado.IDDispositivoEmisor      = atoi(subChar( dest, paquete_recibido,pos,lon ));        pos += lon;
	lon = 2; strncpy(paqueteV01->Encabezado.IDSistemaReceptor,      subChar( dest, paquete_recibido,pos,lon ),lon );   pos += lon;
	lon = 2; paqueteV01->Encabezado.IDDispositivoReceptor    = atoi(subChar( dest, paquete_recibido,pos,lon ));        pos += lon;
	lon = 4; paqueteV01->Encabezado.IDMensaje                = atoi(subChar( dest, paquete_recibido,pos,lon ));        pos += lon;
	lon = 1; paqueteV01->Encabezado.TimeOut                  = atoi(subChar( dest, paquete_recibido,pos,lon ));        pos += lon;
	lon = 3; strncpy(paqueteV01->Encabezado.Password,               subChar( dest, paquete_recibido,pos,lon ),lon );   pos += lon;
	lon = 1; paqueteV01->Encabezado.IdentificadorGrupo       = atoi(subChar( dest, paquete_recibido,pos,lon ));
}

static
int8_t
mensajeria_desarmar_grupo1                          ( PaqueteV01_t * paqueteV01, char * paquete_recibido )
{
    int8_t resultado = 0;
    char dest[8];

    if (strlen(paquete_recibido) > L_PAQUETE_ENCABEZADO + L_PAQUETE_GRUPO1)
        resultado = 4;              // Longitud del paquete incorrecta (Grupo)
    else{
        int pos = L_PAQUETE_ENCABEZADO + 1;
        int lon = 0;

        lon = 4; paqueteV01->Grupo1.idMensaje              = atoi(subChar( dest, paquete_recibido, pos, lon ));        pos += lon;
        lon = 2; paqueteV01->Grupo1.idError                = atoi(subChar( dest, paquete_recibido, pos, lon ));
    }
    
    return resultado;
}

static
int8_t
mensajeria_desarmar_grupo2                          ( PaqueteV01_t * paqueteV01, char * paquete_recibido )
{
    int8_t resultado = 0;
    char dest[8];

    if (strlen(paquete_recibido) > L_PAQUETE_ENCABEZADO + L_PAQUETE_GRUPO2)
        resultado = 4;              // Longitud del paquete incorrecta (Grupo)
    else{
        int pos = L_PAQUETE_ENCABEZADO + 1;
        int lon = 0;

        lon = 2; paqueteV01->Grupo2.Segundos               = atoi(subChar( dest, paquete_recibido, pos, lon ));        pos += lon;
        lon = 2; paqueteV01->Grupo2.Minutos                = atoi(subChar( dest, paquete_recibido, pos, lon ));        pos += lon;
        lon = 2; paqueteV01->Grupo2.Horas                  = atoi(subChar( dest, paquete_recibido, pos, lon ));        pos += lon;
        lon = 4; paqueteV01->Grupo2.SerieIUM               = atoi(subChar( dest, paquete_recibido, pos, lon ));
    }
    
    return resultado;
}

static
int8_t
mensajeria_desarmar_grupo3                          ( PaqueteV01_t * paqueteV01, char * paquete_recibido )
{
    int8_t resultado = 0;
    char dest[8];

    if (strlen(paquete_recibido) > L_PAQUETE_ENCABEZADO + L_PAQUETE_GRUPO3)
        resultado = 4;              // Longitud del paquete incorrecta (Grupo)
    else{
        int pos = L_PAQUETE_ENCABEZADO + 1;
        int lon = 0;

        lon = 4; paqueteV01->Grupo3.SerieIUM               = atoi(subChar( dest, paquete_recibido, pos, lon ));        pos += lon;
        lon = 2; paqueteV01->Grupo3.idPuertaEnlaceAsignado = atoi(subChar( dest, paquete_recibido, pos, lon ));        pos += lon;
        lon = 2; paqueteV01->Grupo3.idNodoAsignado         = atoi(subChar( dest, paquete_recibido, pos, lon ));        pos += lon;
        lon = 2; paqueteV01->Grupo3.segTrasmitirDatos      = atoi(subChar( dest, paquete_recibido, pos, lon ));        pos += lon;
        lon = 1; paqueteV01->Grupo3.segRetrasmitir         = atoi(subChar( dest, paquete_recibido, pos, lon ));
    }
    
    return resultado;
}

static
int8_t
mensajeria_desarmar_grupo4                          ( PaqueteV01_t * paqueteV01, char * paquete_recibido )
{
    int8_t resultado = 0;
    char dest[8];

    if (strlen(paquete_recibido) > L_PAQUETE_ENCABEZADO + L_PAQUETE_GRUPO4)
        resultado = 4;              // Longitud del paquete incorrecta (Grupo)
    else{
        int pos = L_PAQUETE_ENCABEZADO + 1;
        int lon = 0;

        lon = 2; paqueteV01->Grupo4.idSensor              = atoi(subChar( dest, paquete_recibido, pos, lon ));        pos += lon;
        lon = 3; paqueteV01->Grupo4.Valor                 = atoi(subChar( dest, paquete_recibido, pos, lon ));
    }
    
    return resultado;
}


////////////////////////////////////////////////////////////////////////////////////////
// Metodos para armar el paquete a enviar                                             //
////////////////////////////////////////////////////////////////////////////////////////


int8_t
mensajeria_armar		                            ( char *  paquete_enviado, PaqueteV01_t   paqueteV01 )
{
    int8_t resultado = 0;
    char paquete_BLANCO[33] = "";
    strncpy(paquete_enviado, paquete_BLANCO, 33);

    resultado = mensajeria_armar_encabezado ( paquete_enviado, paqueteV01 );
	if (!resultado){
        switch (paqueteV01.Encabezado.IdentificadorGrupo)
        {
            case 1: resultado = mensajeria_armar_grupo1 ( paquete_enviado, paqueteV01 ); break;
            case 2:	resultado = mensajeria_armar_grupo2 ( paquete_enviado, paqueteV01 ); break;
            case 3:	resultado = mensajeria_armar_grupo3 ( paquete_enviado, paqueteV01 ); break;
            case 4:	resultado = mensajeria_armar_grupo4 ( paquete_enviado, paqueteV01 ); break;

            default:
                resultado = 3;   // Numero incorrecto de grupo
                break;
        }
    }
    
    return resultado;
}

static
int8_t
mensajeria_armar_encabezado                         ( char *  paquete_enviado, PaqueteV01_t   paqueteV01 )
{
    int8_t resultado = 0;
    int pos = 0;
    int lon = 0;
    char dest[8];

    lon = 2; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Encabezado.VersionPaquete          ,lon), lon);   pos += lon;
    lon = 2; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Encabezado.VersionFirmware         ,lon), lon);   pos += lon;
    lon = 2; strncpy(paquete_enviado + pos,                 paqueteV01.Encabezado.IDSistemaEmisor              , lon);   pos += lon;
    lon = 2; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Encabezado.IDDispositivoEmisor     ,lon), lon);   pos += lon;
    lon = 2; strncpy(paquete_enviado + pos,                 paqueteV01.Encabezado.IDSistemaReceptor            , lon);   pos += lon;
    lon = 2; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Encabezado.IDDispositivoReceptor   ,lon), lon);   pos += lon;
    lon = 4; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Encabezado.IDMensaje               ,lon), lon);   pos += lon;
    lon = 1; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Encabezado.TimeOut                 ,lon), lon);   pos += lon;
    lon = 3; strncpy(paquete_enviado + pos,                 paqueteV01.Encabezado.Password                     , lon);   pos += lon;
    lon = 1; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Encabezado.IdentificadorGrupo      ,lon), lon);   pos += lon;

    return resultado;
}

static
int8_t
mensajeria_armar_grupo1                             ( char *  paquete_enviado, PaqueteV01_t   paqueteV01 )
{
    int8_t resultado = 0;

    int pos = L_PAQUETE_ENCABEZADO;
    int lon = 0;
    char dest[8];
    
    lon = 4; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Grupo1.idMensaje                   ,lon), lon);   pos += lon;
    lon = 2; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Grupo1.idError                     ,lon), lon);

    return resultado;
}

static
int8_t
mensajeria_armar_grupo2                             ( char *  paquete_enviado, PaqueteV01_t   paqueteV01 )
{
    int8_t resultado = 0;
    int pos = L_PAQUETE_ENCABEZADO;
    int lon = 0;
    char dest[8];

    lon = 2; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Grupo2.Segundos                    ,lon), lon);   pos += lon;
    lon = 2; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Grupo2.Minutos                     ,lon), lon);   pos += lon;
    lon = 2; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Grupo2.Horas                       ,lon), lon);   pos += lon;
    lon = 4; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Grupo2.SerieIUM                    ,lon), lon);
    
    return resultado;
}

static
int8_t
mensajeria_armar_grupo3                             ( char *  paquete_enviado, PaqueteV01_t   paqueteV01 )
{
    int8_t resultado = 0;
    int pos = L_PAQUETE_ENCABEZADO;
    int lon = 0;
    char dest[8];

    lon = 4; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Grupo3.SerieIUM                  ,lon), lon);   pos += lon;
    lon = 2; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Grupo3.idPuertaEnlaceAsignado    ,lon), lon);   pos += lon;
    lon = 2; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Grupo3.idNodoAsignado            ,lon), lon);   pos += lon;
    lon = 2; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Grupo3.segTrasmitirDatos	        ,lon), lon);   pos += lon;
    lon = 1; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Grupo3.segRetrasmitir            ,lon), lon);
    
    return resultado;
}

static
int8_t
mensajeria_armar_grupo4                             ( char *  paquete_enviado, PaqueteV01_t   paqueteV01 )
{
    int8_t resultado = 0;
    int pos = L_PAQUETE_ENCABEZADO;
    int lon = 0;
    char dest[8];

    lon = 2; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Grupo4.idSensor                  ,lon), lon);   pos += lon;
    lon = 3; strncpy(paquete_enviado + pos, intToChar(dest, paqueteV01.Grupo4.Valor                     ,lon), lon);

    return resultado;
}
