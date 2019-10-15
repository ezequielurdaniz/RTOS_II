/*============================================================================
 *
 * Autores: Luciano Perren       <lgperren@gmail.com>
 *          Juan Pablo Menditto  <jpmenditto@gmail.com>
 *          Pablo Zizzutti       <pablozizzutti@gmail.com>
 *
 * RTOS II - TP1
 * Date: 2019/09/10
 * Docentes: Franco Bucafusco y Sergio Renato De Jesús Melean
 *
 * Driver.c
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
#define UART_232       UART_232

#define TIMEOUT_VALIDATION 3 // 3 ms de timeout

#define LED_ROJO      LED2
#define LED_AMARILLO  LED1
#define LED_VERDE     LED3

#define MEMORIADINAMICA 75

#define ELEMENTOS_MEMORIA 4

/*=====[Private function-like macros]========================================*/

/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Prototypes (declarations) of private functions]======================*/


/*=====[Implementations of public functions]=================================*/

/*!
 * @brief  Tarea principal del Driver
 *
 * @param[in] pvParameters
 *
 * @return  void
 */

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
	int TamCola;

	uint8_t crc_temp_rx;
	uint8_t crc_temp_tx;

	char caracter_out;

	struct node *temp;

    /* loop infinito de la tarea */
    for( ;; ) {

       TamCola=TamanioCola();

       // Lee la cantidad de datos que hay en la cola dinamica de datos, para procesarlos o quedarse aqui
       while(TamCola==0){
    	  TamCola=TamanioCola();
       }

       // Guardo en indice la cantidad de datos que recibi por el puerto serie
       indice = strlen(front->datos);

       // realizo el calculo del CRC para verificar el dato entrante
       crc_temp_rx = crc8_calc(0 , front->datos , indice-1);


   	   // chequeo que el CRC8 calculado sea igual al CRC8 entrante con los datos por el puerto serie
   	   if(crc_temp_rx == front->datos[indice-1]){

   	 	   /* Llego un paquete bueno, paso a la siguiente capa */
   		   // Sobreescribo el caracter del CRC8 por el \0
   		   front->datos[indice -1] = '\0';

   		   // Chequea el paquete entrante para verificar si son letras
   		   EstadoPaquete = CheckLettersFnc(front->datos);

   		   // Si el paquete esta ok, tiene todas letras, lo convierte a mayusculas y lo envia por la queue
   		   // sino devuelve error por la queue
   		   if(EstadoPaquete == true ){
   			    lValueToSend = MinusToMayus(front->datos); // Se envia el mensaje convertido

   				// calcula el CRC8 de salida y lo suma al string
   				// Al dato recibido, se le agrega el crc8 y se reenvia por uart
   				lValueToSend = CalculaCRC8(lValueToSend);

   				EnvioMensajeUART(lValueToSend, indice);

   				indice = 0;
   				memset(&lValueToSend[0], 0, sizeof(lValueToSend));                      // clear the array

   		   }
   		   else
   		   {
   			    // envio un error por transmision de uart, ya que el paquete contiene otros caracteres, no letras
   				EnvioErrorUart();
   		   }

   	    }
   	    else
   	    {
   	       // Llego un paquete con mal CRC8, devuelvo error por el puerto
   	    	EnvioErrorUart();
   	    }

   	//EliminaBloqueMemoriaDinamica();

    }
}


/*=====[Implementations of interrupt functions]==============================*/


/*=====[Implementations of private functions]================================*/

/*!
 * @brief  Inicializacion del puerto UART indicado
 *
 * @param[in] uart
 *
 * @return  NULL
 */

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

/*!
 * @brief  Calcula el tamaño de la cola de memoria dinamica y verifica si hay datos pendientes en la misma
 *
 * @param[in] void
 *
 * @return  uint8_t
 */

int TamanioCola(){

	struct node *temporal;   // estructura temporal para el calculo del tamanio de la cola
	int cnt;             // Variabe que contiene el tamaño de la cola

		 // Verificar el tamaño de la cola que no sea mas grande que el maximo permitido : ELEMENTOS_MEMORIA
		 temporal = front; // guarda el primer elemento de la cola en un temporal

		 cnt = 0;  // inicializa la variable para el recorrido

		 // va recorriendo toda la cola desde el primer elemento
		 while (temporal){
			temporal = temporal->link;
			cnt++;
		 }

	return cnt;   // devuelve la cantidad de elementos presentes en la cola dinamica

}

/*!
 * @brief  Envia por UART el mensaje str
 *
 * @param[in] char * str
 * @param[in] int indice
 *
 * @return  void
 */


void EnvioMensajeUART(char * str, int indice){

	BaseType_t xStatusTX;  // Variable de status de la queue
	char caracter_out;

	for(int i = 0 ; i < indice+1 ; i++){
	   caracter_out = str[i];
	   xStatusTX = xQueueSend( xQueueRecibe, &caracter_out, 0 ); // envio datos por Queue
	}

	// Inicia el timer de salida
	xTimerStart( TimeToExit , 0 );
	EliminaBloqueMemoriaDinamica();

}


