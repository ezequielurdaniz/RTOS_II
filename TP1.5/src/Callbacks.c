/*=============================================================================
 *
 *  RTOS II : TP 1
 *
 *  Alumnos :
 *
 *  Luciano Perren
 *  Juan Pablo Menditto
 *  Pablo Zizzutti
 *
 *===========================================================================*/

/*=====[Inclusion of own header]=============================================*/

#include "Callbacks.h"

/*=====[Inclusions of private function dependencies]=========================*/
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"
#include "crc8.h"
#include "Driver.h"
#include "TimersControl.h"
#include "ProcessLetters.h"
#include "timers.h"
#include "queue.h"
#include "stdio.h"
#include "ctype.h"
#include "string.h"


/*=====[Definition macros of private constants]==============================*/


/*=====[Private function-like macros]========================================*/


/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/


/*=====[Definitions of private global variables]=============================*/




/*=====[Prototypes (declarations) of private functions]======================*/

void uartUsbReceiveCallback( void *unused );
void uartUsbSendCallback( void *unused );



/*=====[Implementations of public functions]=================================*/


/*=====[Implementations of interrupt functions]==============================*/



/*****************************************************************************************
 *
 *
 *   UART Callback RX
 *
 *
 *****************************************************************************************/

void uartUsbReceiveCallback( void *unused )
{
	static uint8_t indicerx;
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	char Caracter;

	// FSM Recepcion de Uart
	switch( fsmUARTRXState ){

	         case StandBy:

	        	 indicerx=0;
	        	 // Recibimos el primer elemento por UART
	        	 bufferin[indicerx] = uartRxRead(UART_USB);
	        	 //uartTxWrite(UART_USB, bufferin[indicerx] );
	             xTimerStartFromISR( TimerTimeout , &xHigherPriorityTaskWoken );
	             fsmUARTRXState = Recibiendo;

	         break;

	         case Recibiendo:
	        	 // Compruebo que no excede el limite de la memoria dinamica
	        	 if(indicerx > (MEMORIADINAMICA-1)){
	        		fsmUARTRXState = ErrorLimite;
	        	 }
	        	 else
	        	 {
	        		 xTimerResetFromISR( TimerTimeout , &xHigherPriorityTaskWoken );
	        		 indicerx++;
	        		 bufferin[indicerx] = uartRxRead(UART_USB);
	        		 //uartTxWrite(UART_USB, bufferin[indicerx] );
	        	 }
	         break;

	         case ErrorLimite:
	        	 xTimerResetFromISR( TimerTimeout , &xHigherPriorityTaskWoken );
	        	 Caracter = uartRxRead(UART_USB);
	         break;
 	}
}

/*****************************************************************************************
 *
 *
 *   UART Callback TX
 *
 *
 *****************************************************************************************/

void uartUsbSendCallback( void *unused )
{
	char crc_temp_tx;
	char caracter_in;
	static char lReceivedValue[MEMORIADINAMICA];
	static uint8_t indice = 0;
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	BaseType_t xTaskWokenByReceive = pdFALSE;

	 // Recepcion de los datos de la queue
	 /* Se recibe un caracter y se almacena */
	 if(xQueueReceiveFromISR( xQueueRecibe, &caracter_in, &xTaskWokenByReceive ) == pdTRUE){
		 /* Se recibe un caracter y se almacena */
		 lReceivedValue[indice] = caracter_in;

		  if(lReceivedValue[indice] != '\0'){
		     /* Final del mensaje */
			 uartWriteByte(UART_USB, caracter_in );
			 indice++;
		  }
		  else
		  {
			 // Calcula el CRC8
			 // Al dato recibido, se le agrega el crc8 y se reenvia por uart
			 crc_temp_tx = crc8_calc(0 , lReceivedValue , strlen(lReceivedValue)); // realizo el calculo del CRC8
			 uartWriteByte(UART_USB, crc_temp_tx);                                 // envio el CRC por UART

			 indice = 0;

			 memset(&lReceivedValue[0], 0, sizeof(lReceivedValue)); // clear the array
			 uartWriteByte(UART_USB, '\r' );
			 uartWriteByte(UART_USB, '\n' );
			 uartCallbackClr( UART_USB, UART_TRANSMITER_FREE);

			// xTimerStartFromISR( TimeToExit , xHigherPriorityTaskWoken );
		  }
	 }
}









/*=====[Implementations of private functions]================================*/



