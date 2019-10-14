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
#include "Driver.h"
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


char bufferin[100];  // Variable de recepcion de datos por puerto UART       // <-- File object needed for each open file




/*=====[Prototypes (declarations) of private functions]======================*/


void EliminaBloqueMemoriaDinamica();

bool_t VerificaColaLlena();
void uartUsbReceiveCallback( void *unused );
void uartUsbSendCallback( void *unused );


/*=====[Implementations of public functions]=================================*/



/**********************************************************************************************
 *
 *    Callbacks Timers - Implementaciones
 *
 **********************************************************************************************/

void HeartbeatCallback(TimerHandle_t xTimer){
	gpioToggle(LEDB);
}

void TimeToExitCallback(TimerHandle_t xTimer){
    if(xTimer == TimeToExit){
	    xTimerStop( TimeToExit , 0 ); // detiene el timer
	    xTimerReset( TimeToExit , 0 );
    }
}


void TimeoutCallback(TimerHandle_t xTimer){

  BaseType_t xStatusTX;
  BaseType_t xHigherPriorityTaskWoken = pdTRUE;
  uint8_t crc_temp_rx;

  uint8_t indice;
  bool_t ColaLlena;

  char caracter_out;
  struct node *temp;

  char Error[] = "ERROR1 ";                 // Mensaje de error para el envio por la queue
  char ErrorCola[] = "ERROR COLA LLENA ";  // Mensaje de error para el envio por la queue
  char ErrorLim[] = "ERROR LIMITE EXCEDIDO "; // Mensaje de error para el envio por la queue

  if(xTimer == TimerTimeout){

	 if( xTimerStop( TimerTimeout, 0) != pdPASS )
	 {
		  /* The stop command was not executed successfully. Take appropriate action here. */
	 }
	 gpioToggle(LED1);

	 // Si se produjo un error por exceder el limite de caracteres de recepcion

	 if(fsmUARTRXState==ErrorLimite){
		 ErrorLim[21]='\0';
		 for(int j = 0 ; j < 22; j++){
		    caracter_out = ErrorLim[j];
		    xStatusTX = xQueueSend( xQueueRecibe, &caracter_out, 0 );
		 }

		 // activa el callback de TX UART
		 memset(&bufferin[0], 0, sizeof(bufferin));                                    // clear the array bufferin
		 uartCallbackSet(UART_USB, UART_TRANSMITER_FREE, uartUsbSendCallback, NULL);
		 fsmUARTRXState = StandBy;
		 return;// Sale del callback del timer
     }

	 // Verifica si la cola esta llena, si es asi, avisa por callback de salida que no puede recibir mas datos
	 // de lo contrario tratara de procesar el paquete entrante
	 fsmUARTRXState = StandBy;

	 ColaLlena=VerificaColaLlena();

     if(ColaLlena==TRUE){
       ErrorCola[16]='\0';
       for(int j = 0 ; j < 17; j++){
    	 caracter_out = ErrorCola[j];
      	 xStatusTX = xQueueSend( xQueueRecibe, &caracter_out, 0 );
       }

       // activa el callback de TX UART
       memset(&bufferin[0], 0, sizeof(bufferin));                                    // clear the array bufferin
       uartCallbackSet(UART_USB, UART_TRANSMITER_FREE, uartUsbSendCallback, NULL);
       return;// Sale del callback del timer
     }

     //--------------------------------------------------------------------------------------------------------
     // en el caso que haya memoria en el sistema, crea un nuevo bloque de memoria en la cola de memorias dinamicas
     // y acomoda los punteros de la cola dinamica de memorias
     //

	 temp = (struct node*)pvPortMalloc(sizeof(struct node));

	 temp->link = NULL;

	 if(front == NULL){
	  	 front = rear = temp;
	 }
	 else
	 {
	  	 rear->link = temp;
	  	 rear = temp;
	 }

	 // Copia el contenido del buffer entrante de datos en el dato que ira al frente
	 for(int i = 0 ; i < strlen(bufferin); i++){
		  front->datos[i] = bufferin[i];
	 }

	 //gpioToggle(LED3);

	 indice = strlen(front->datos);

	 crc_temp_rx = crc8_calc(0 , front->datos , indice-1);                  // realizo el calculo del CRC

	 if(crc_temp_rx == front->datos[indice-1])
	 {
	 	 // Llego un paquete bueno, paso a la siguiente capa
		 front->datos[indice -1] = '\0'; // Sobreescribo el caracter del CRC8 por el \0

		 for(int i = 0 ; i < indice; i++){
			 caracter_out = front->datos[i];
		 	 xStatusTX = xQueueSend( xQueueEnvia, &caracter_out, 0 ); // envio datos por Queue
		 }

		 if( xStatusTX == pdPASS ){
		 	// OK EN EL ENVIO DE LA COLA
		 }
		 else
		 {
		 	// ERROR ENVIO EN LA COLA
		 }
	 }
	 else
	 {
	     // Llego un paquete malo, devuelvo error por el puerto
		 Error[6] = '\0';
		 for(int j = 0 ; j < 7 ; j++){
			caracter_out = Error[j];
			xStatusTX = xQueueSend( xQueueRecibe, &caracter_out, 0 );
		 }

		 EliminaBloqueMemoriaDinamica();
	 }

	 memset(&bufferin[0], 0, sizeof(bufferin));

  }
}






/*=====[Implementations of interrupt functions]==============================*/




/*=====[Implementations of private functions]================================*/

void EliminaBloqueMemoriaDinamica(){

	struct node *temp;
	// borro la memoria dinamica
	// Problema resuelto : se borro el bloque de memoria para no dejar basura en la proxima
	// transaccion
	uartCallbackSet(UART_USB, UART_TRANSMITER_FREE, uartUsbSendCallback, NULL);

	memset(&front->datos[0], 0, sizeof(front->datos));                     // clear the array
	temp = front;
	front = front->link;
	vPortFree(temp);
}




bool_t VerificaColaLlena(){

	 struct node *temporal;
	 int cnt;
	 // Verificar el tamaño de la cola que no sea mas grande que el maximo permitido : ELEMENTOS_MEMORIA
	 temporal = front;

	 cnt = 0;

	 while (temporal){
		temporal = temporal->link;
		cnt++;
	 }

	 if(cnt == ELEMENTOS_MEMORIA){
         return TRUE;
	  }

	  return FALSE;
}


