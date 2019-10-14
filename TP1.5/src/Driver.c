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

#include "Driver.h"

/*=====[Inclusions of private function dependencies]=========================*/
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"
#include "crc8.h"
#include "TimersControl.h"
#include "ProcessLetters.h"
#include "timers.h"
#include "queue.h"
#include "stdio.h"
#include "ctype.h"
#include "string.h"


/*=====[Definition macros of private constants]==============================*/


#define UART_PC        UART_USB
#define UART_BLUETOOTH UART_232

#define TIMEOUT_VALIDATION 100 // 100 ms de timeout

#define LED_ROJO      LED2
#define LED_AMARILLO  LED1
#define LED_VERDE     LED3

#define MEMORIADINAMICA 25

#define ELEMENTOS_MEMORIA 4



/*=====[Private function-like macros]========================================*/


/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/


/*=====[Prototypes (declarations) of private functions]======================*/

bool_t CheckLettersFnc(char *str);
char* MinusToMayus(char *str);
void uartDriverInit(uartMap_t uart);




/*=====[Implementations of public functions]=================================*/

/*****************************************************************************************
 *
 *
 *   Tarea Driver
 *
 *
 *****************************************************************************************/

void Driver( void* pvParameters )
{

	DEBUG_PRINT_ENABLE;  //Para configurar los mensajes por monitor

	/* Configuracion inicial de la tarea */
	char* lValueToSend;	 // el valor a enviar por la cola
	char lReceivedValue[MEMORIADINAMICA];

	uint8_t indice = 0;

	BaseType_t xStatusRX;  // Variable de status de la queue
	BaseType_t xStatusTX;  // Variable de status de la queue

	const TickType_t xTicksToWait = 0;
	bool_t EstadoPaquete;

	char Error[] = "ERROR "; // Mensaje de error para el envio por la queue

	char caracter_in;
	char caracter_out;

	struct node *temp;

    /* loop infinito de la tarea */
    while (TRUE){

    	if(xQueueReceive( xQueueEnvia, &caracter_in, xTicksToWait ) == pdTRUE){
    		/* Se recibe un caracter y se almacena */
    		lReceivedValue[indice] = caracter_in;

    		//uartWriteByte(UART_USB, caracter_in );

			if(lReceivedValue[indice] != '\0'){
    			/* Final del mensaje , lo procesa */
				indice++;
			}
			else
			{
			   /*
			   for(int i = 0 ; lReceivedValue[i] != '\0'; i++){
			   	   uartWriteByte(UART_USB, lReceivedValue[i] );
			   }*/

			   // Chequea el paquete entrante para verificar si son letras
			   EstadoPaquete = CheckLettersFnc(lReceivedValue);

			   // Si el paquete esta ok, tiene todas letras, lo convierte a mayusculas y lo envia por la queue
			   // sino devuelve error por la queue

               if(EstadoPaquete == true ){
				  lValueToSend = MinusToMayus(lReceivedValue); // Se envia el mensaje convertido
				  indice++;
				  lValueToSend[indice] = '\0'; // Agrego el caracter NULL para el envio

				  for(int i = 0 ; i < indice; i++){
				      caracter_out = lValueToSend[i];
				      xStatusTX = xQueueSend( xQueueRecibe, &caracter_out, 0 ); // envio datos por Queue
				   }

               }
			   else
			   {
				   Error[5] = '\0';
				   for(int j = 0 ; j < 6; j++){
					 caracter_out = Error[j];
				     xStatusTX = xQueueSend( xQueueRecibe, &caracter_out, 0 );
				   }
			   }

               indice = 0;
               memset(&lValueToSend[0], 0, sizeof(lValueToSend));                      // clear the array
               memset(&lReceivedValue[0], 0, sizeof(lReceivedValue));                  // clear the array
               EliminaBloqueMemoriaDinamica();

			}
        }
    }
}


/*****************************************************************************************
 *
 *
 *   Inicializacion de la UART
 *
 *
 *****************************************************************************************/

void uartDriverInit(uartMap_t uart) {

	/* Inicializar la UART_USB junto con las interrupciones de Tx y Rx */
	uartConfig(uart, 115200);
	// Seteo un callback al evento de recepcion y habilito su interrupcion
	uartCallbackSet(uart, UART_RECEIVE, uartUsbReceiveCallback, NULL);
	// Seteo un callback al evento de transmisor libre y habilito su interrupcion
	uartCallbackSet(uart, UART_TRANSMITER_FREE, uartUsbSendCallback, NULL);
	// Habilito todas las interrupciones de UART_USB
	uartInterrupt(uart, true);
	// Clear el callback para transmision del UART
	uartCallbackClr(uart, UART_TRANSMITER_FREE);
	// Mensaje de inicio
	uartWriteString(uart, "Iniciando...\r\n");

}





/*=====[Implementations of interrupt functions]==============================*/


/*=====[Implementations of private functions]================================*/




