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
 * TimersControls.c
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

char memDinIn[100];  // Variable de recepcion de datos por puerto UART       // <-- File object needed for each open file

/*=====[Prototypes (declarations) of private functions]======================*/

/*=====[Implementations of public functions]=================================*/


/*!
 * @brief  Callback del Timer Hearbeat del sistema
 *
 * @param[in] TimerHandle_t
 *
 * @return  void
 */

void HeartbeatCallback(TimerHandle_t xTimer){
	gpioToggle(LEDB);
}

/*!
 * @brief  Callback del Timer para la salida de datos
 *
 * @param[in] TimerHandle_t
 *
 * @return  void
 */

void TimeToExitCallback(TimerHandle_t xTimer){
    if(xTimer == TimeToExit){
	    xTimerStop( TimeToExit , 0 ); // detiene el timer
	    xTimerReset( TimeToExit , 0 );
    }
}

/*!
 * @brief  Callback del Timer para la llegada de datos
 *
 * @param[in] TimerHandle_t
 *
 * @return  void
 */

void TimeoutCallback(TimerHandle_t xTimer){

  bool_t ColaLlena;

  struct node *temp;

  if(xTimer == TimerTimeout){

	 if( xTimerStop( TimerTimeout, 0) != pdPASS )
	 {
		/* Fallo el comando de detencion del timer. Enciende el led de ERROR */
		 gpioToggle(LED2);
	 }

	 // Si se produjo un error por exceder el limite de caracteres de recepcion
	 if(fsmUARTRXState==ErrorLimite){

		 EnvioErrorUartLim();
		 // activa el callback de TX UART
		 BorrarBufferIn();
		 fsmUARTRXState = StandBy;
		 return;// Sale del callback del timer
     }

	 // Verifica si la cola esta llena, si es asi, avisa por callback de salida que no puede recibir mas datos
	 // de lo contrario tratara de procesar el paquete entrante
	 fsmUARTRXState = StandBy;

	 // Verifica si la cola esta llena o tiene espacio
	 ColaLlena=VerificaColaLlena();

	 // Si la cola esta llena, avisa al usuario utilizando un mensaje de error
     if(ColaLlena==TRUE){

       EnvioErrorUartLim();
       // activa el callback de TX UART
	   BorrarBufferIn();                                 // clear the array bufferin
       return;// Sale del callback del timer
     }

     //--------------------------------------------------------------------------------------------------------
     // en el caso que haya memoria en el sistema, crea un nuevo bloque de memoria en la cola de memorias dinamicas
     // y acomoda los punteros de la cola dinamica de memorias
     //

     // Creo el bloque de memoria dinamica
	 temp = (struct node*)pvPortMalloc(sizeof(struct node));
	 // Copia el contenido del buffer entrante de datos en el dato que ira al frente
	 for(int i = 0 ; i < strlen(memDinIn); i++){
		 temp->datos[i] = memDinIn[i]; // copia el contenido del buffer entrante en ultimo elemento de la cola dinamica
	 }

	 BorrarBufferIn();

	 // el bloque creado, sera el ultimo de la cola, entonces apuntara a NULL
	 temp->link = NULL;

	 // Si el primer elemento es NULL, significa que la cola esta vacia, entonces el elemento que creo
	 // sera el primero y el ultimo (FRONT & REAR)
	 // en caso que no sea el primero, lo acomodará al final de la cola dinamica de memoria

	 if(front == NULL){
	  	 front = rear = temp;
	 }
	 else
	 {
	  	 rear->link = temp;
	  	 rear = temp;
	 }

  }
}

/*!
 * @brief  Elimina el bloque de memoria ya utilizado, limpia los buffers acomoda los punteros de la cola
 * 		   de memoria dinamica y activa el callback de salida de UART para el envio de los datos
 *
 * @param[in] void
 *
 * @return  void
 */

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


/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/

/*!
 * @brief  Verifica si la cola de memorias dinamicas esta llena
 *
 * @param[in] void
 *
 * @return  bool_t
 */

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

/*!
 * @brief  Borra el contenido del bufferin
 *
 * @param[in] void
 *
 * @return  void
 */

void BorrarBufferIn(){
	memset(&memDinIn[0], 0, sizeof(memDinIn));
}

/*!
 * @brief  Envia por UART el mensaje de ERROR por limite de caracteres o cola llena
 *
 * @param[in] void
 *
 * @return  void
 */

void EnvioErrorUartLim(){

	BaseType_t xStatusTX;    // Variable de status de la queue
	char Error[] = "ERROR "; // Mensaje de error para el envio por la queue
	char caracter_out;

	Error[5] = '\0';
	for(int j = 0 ; j < 6; j++){
	   	  caracter_out = Error[j];
	   	  xStatusTX = xQueueSend( xQueueRecibe, &caracter_out, 0 );
	}
	// Inicia el timer de salida
	xTimerStart( TimeToExit , 0 );
	uartCallbackSet(UART_USB, UART_TRANSMITER_FREE, uartUsbSendCallback, NULL);
}

/*!
 * @brief  Envia por UART el mensaje de ERROR
 *
 * @param[in] void
 *
 * @return  void
 */

void EnvioErrorUart(){

	BaseType_t xStatusTX;    // Variable de status de la queue
	char Error[] = "ERROR "; // Mensaje de error para el envio por la queue
	char caracter_out;

	Error[5] = '\0';
	for(int j = 0 ; j < 6; j++){
	   	  caracter_out = Error[j];
	   	  xStatusTX = xQueueSend( xQueueRecibe, &caracter_out, 0 );
	}
	// Inicia el timer de salida
	xTimerStart( TimeToExit , 0 );
	EliminaBloqueMemoriaDinamica();
}


