/* Copyright 2014, ChaN
 * Copyright 2016, Matias Marando
 * Copyright 2016, Eric Pernia
 * All rights reserved.
 *
 * This file is part of Workspace.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*==================[inclusiones]============================================*/

#include "sapi.h"     // <= sAPI header
#include "sd_spi.h"   // <= own header (optional)
#include "ff.h"       // <= Biblioteca FAT FS

#include <string.h>

/*==================[definiciones y macros]==================================*/

#define FILENAME "m1.txt"		// "hola.txt"

/*==================[definiciones de datos internos]=========================*/

static FATFS fs;           // <-- FatFs work area needed for each volume
static FIL fp;             // <-- File object needed for each open file

/*==================[definiciones de datos externos]=========================*/



/*==================[declaraciones de funciones internas]====================*/


/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.

 */
char* itoa(int value, char* result, int base) {
   // check that the base if valid
   if (base < 2 || base > 36) { *result = '\0'; return result; }

   char* ptr = result, *ptr1 = result, tmp_char;
   int tmp_value;

   do {
      tmp_value = value;
      value /= base;
      *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
   } while ( value );

   // Apply negative sign
   if (tmp_value < 0) *ptr++ = '-';
   *ptr-- = '\0';
   while(ptr1 < ptr) {
      tmp_char = *ptr;
      *ptr--= *ptr1;
      *ptr1++ = tmp_char;
   }
   return result;
}

/* Devuelvo fecha y hora en formato "DD/MM/YYYY, HH:MM:SS" */
void formatoDateAndTime( rtc_t * rtc,  char* buffer ){

	/* Buffer temporal*/
	char TMPbuffer[20]="";

	/* Envio el año */
	if( (rtc->year)<10 )
		strncat(buffer,"0",1);
	itoa( (int) (rtc->year), (char*)TMPbuffer, 10 ); /* 10 significa decimal */
	strncat(buffer,TMPbuffer,strlen(TMPbuffer));
	strncat(buffer,"/",1);

	/* Envio el mes */
	if( (rtc->month)<10 )
		strncat(buffer,"0",1);
	itoa( (int) (rtc->month), (char*)TMPbuffer, 10 ); /* 10 significa decimal */
	strncat(buffer,TMPbuffer,strlen(TMPbuffer));
	strncat(buffer,"/",1);

	/* Envio el dia */
	if( (rtc->mday)<10 )
		strncat(buffer,"0",1);
	itoa( (int) (rtc->mday), (char*)TMPbuffer, 10 ); /* 10 significa decimal */
	strncat(buffer,TMPbuffer,strlen(TMPbuffer));

	strncat(buffer,"_",2);

	/* Envio la hora */
	if( (rtc->hour)<10 )
		strncat(buffer,"0",1);
	itoa( (int) (rtc->hour), (char*)TMPbuffer, 10 ); /* 10 significa decimal */
	strncat(buffer,TMPbuffer,strlen(TMPbuffer));
	strncat(buffer,":",1);

	/* Envio los minutos */
	if( (rtc->min)<10 )
		strncat(buffer,"0",1);
	itoa( (int) (rtc->min), (char*)TMPbuffer, 10 ); /* 10 significa decimal */
	strncat(buffer,TMPbuffer,strlen(TMPbuffer));
	strncat(buffer,":",1);

	/* Envio los segundos */
	if( (rtc->sec)<10 )
		strncat(buffer,"0",1);
	itoa( (int) (rtc->sec), (char*)TMPbuffer, 10 ); /* 10 significa decimal */
	strncat(buffer,TMPbuffer,strlen(TMPbuffer));

}


/*==================[declaraciones de funciones externas]====================*/

// FUNCION que se ejecuta cada vezque ocurre un Tick
void diskTickHook( void *ptr );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void ){

   // ---------- CONFIGURACIONES ------------------------------
   // Inicializar y configurar la plataforma
   boardConfig();

   // SPI configuration
   spiConfig( SPI0 );

   // Inicializar el conteo de Ticks con resolucion de 10ms,
   // con tickHook diskTickHook
   tickConfig( 10 );
   tickCallbackSet( diskTickHook, NULL );

   // ------ PROGRAMA QUE ESCRIBE EN LA SD -------

   UINT nbytes;

   // Give a work area to the default drive
   if( f_mount( &fs, "", 0 ) != FR_OK ){
      // If this fails, it means that the function could
      // not register a file system object.
      // Check whether the SD card is correctly connected
   }


   // Integracion de lo recibido por RTC	///////////////////////////////////////////////////////////////////

   /* Inicializar UART_USB a 115200 baudios */
   uartConfig( UART_USB, 115200 );

   /* Estructura RTC */
   rtc_t rtc;

   // Inicializar reloj
   rtc.year = 2018;
   rtc.month = 6;
   rtc.mday = 28;
   rtc.wday = 5;
   rtc.hour = 13;
   rtc.min = 19;
   rtc.sec= 0;

   /* Inicializar RTC */
   bool_t val = rtcConfig( &rtc );

   delay_t delay1s;
   delayConfig( &delay1s, 1000 );

   delay(2000); // El RTC tarda en setear la hora, por eso el delay


   // Integracion con ADC /////////////////////////////////////////////////////////////////////////////////////

   /* Inicializar AnalogIO */
   /* Posibles configuraciones:
    *    ADC_ENABLE,  ADC_DISABLE,
    */
   adcConfig( ADC_ENABLE ); /* ADC */

   /* Buffer */
   static char adcBuff_CH1[10]="";
   static char adcBuff_CH2[10]="";
   static char adcBuff_CH3[10]="";

   /* Variable para almacenar el valor leido del ADC */
   uint16_t muestra = 0;

   uartWriteString( UART_USB, "Inicio programa... v3\r\n\r\n");

   /* Buffer */
   static char salidaBuff[50]="";	//32

   /* ------------- REPETIR POR SIEMPRE ------------- */
   if( f_open( &fp, FILENAME, FA_WRITE | FA_OPEN_APPEND ) == FR_OK ){
	   while(gpioRead( TEC1 )) {												// Salir si se presiona TEC1 para cerrar corectamente el archivo

		   gpioWrite( LEDG, OFF );
		   gpioWrite( LEDR, OFF );

		   if( delayRead( &delay1s ) ){

		       muestra = adcRead( CH1 );										/* Leo la Entrada Analogica AI0 - ADC0 CH1 */
		       itoa( muestra, adcBuff_CH1, 10 ); 								/* Conversión de muestra entera a ascii con base decimal. 10 significa decimal */

		       muestra = adcRead( CH2 );
		       itoa( muestra, adcBuff_CH2, 10 );

		       muestra = adcRead( CH3 );
		       itoa( muestra, adcBuff_CH3, 10 );

			   /* Leer fecha y hora */
			   val = rtcRead( &rtc );

			   char buffer[20]="";
			   formatoDateAndTime( &rtc,  buffer);

			   // Limpio buffer
			   memset(salidaBuff, 0, strlen(salidaBuff));
			   strncat(salidaBuff,adcBuff_CH1,strlen(adcBuff_CH1));
			   strncat(salidaBuff,";",1);
			   strncat(salidaBuff,adcBuff_CH2,strlen(adcBuff_CH2));
			   strncat(salidaBuff,";",1);
			   strncat(salidaBuff,adcBuff_CH3,strlen(adcBuff_CH3));
			   strncat(salidaBuff,";",1);
			   strncat(salidaBuff,buffer,strlen(buffer));
			   strncat(salidaBuff,";",1);
			   strncat(salidaBuff,"\r\n",2);

			   uartWriteString( UART_USB, salidaBuff );

			   f_write( &fp, salidaBuff, strlen(salidaBuff), &nbytes );

			   if( nbytes == strlen(salidaBuff) ){
				   // Turn ON LEDG if the write operation was successful
				   gpioWrite( LEDG, ON );
			   } else{
				   // Turn ON LEDR if the write operation was fail
				   gpioWrite( LEDR, ON );
			   }
		   }
	   }
	   f_close(&fp);
   }else{
	   // Turn ON LEDR if the write operation was fail
	   gpioWrite( LEDR, ON );
   }

   gpioWrite( LED1, ON );

   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE )
   {
      sleepUntilNextInterrupt();
   }

   return 0;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/

// FUNCION que se ejecuta cada vezque ocurre un Tick
void diskTickHook( void *ptr ){
   disk_timerproc();   // Disk timer process
}


/*==================[fin del archivo]========================================*/
