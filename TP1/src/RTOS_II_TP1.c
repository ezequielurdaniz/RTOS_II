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

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"
#include "crc8.h"
#include "timers.h"
#include "queue.h"
#include "stdio.h"
#include "ctype.h"
#include "string.h"

#define TIMEOUT_VALIDATION 100 // 100 ms de timeout

#define LED_ROJO      LED2
#define LED_AMARILLO  LED1
#define LED_VERDE     LED3

#define MEMORIADINAMICA 100

bool_t CheckLettersFnc(char *str);
char* MinusToMayus(char *str);

void HeartbeatCallback(TimerHandle_t xTimer);
void TimeoutCallback(TimerHandle_t xTimer);

void uartUsbReceiveCallback( void *unused );
void uartUsbSendCallback( void *unused );
void Driver( void* pvParameters );

/* Declaracion de los Timers */
TimerHandle_t TimerTimeout;
TimerHandle_t TimerHeartbeat;
TimerHandle_t TimeToExit;

/* Declaracion de las Queue*/
QueueHandle_t xQueueEnvia;
QueueHandle_t xQueueRecibe;

char bufferin[100];  // Variable de recepcion de datos por puerto UART

/*-----------------------------------------------------------------------------
 *
 *  Declaraciones FSM
 *
 * ---------------------------------------------------------------------------*/

// Nombres de FSM recepcion UART
typedef enum{
   StandBy,
   Recibiendo,
   Error,
} fsmUARTRXState_t;

fsmUARTRXState_t fsmUARTRXState; // Variable para guardar el valor de la maquina de estado

// Memoria dinamica
#define ELEMENTOS_MEMORIA 4

struct node
{
    char datos[MEMORIADINAMICA];
    struct node *link;

}*front, *rear;

/**********************************************************************************************
 *
 *   Funciones capa 3
 *
 **********************************************************************************************/

bool_t CheckLettersFnc(char *str){

	 for (int i = 0; str[i] != '\0'; i++){
		 if(!isalpha(str[i])){ // verifica si el caracter es alfabetico
			 return false;  // devuelve error si encuentra un caracter no alfabetico
		 }
	 }
	 return true; // Si llega hasta aca, recorrio toda la cadena y encontro caracteres alfabeticos
}

char* MinusToMayus(char *str){

	 for (int i = 0; str[i] != '\0'; i++){
		 str[i] = toupper((char)(str[i]));
	 }
	return str;
}

/**********************************************************************************************
 *
 *
 *
 *
 *
 **********************************************************************************************/

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
/*
	 uartWriteString(UART_USB, "Tamanio cola : ");
	 uartWriteByte(UART_USB, cnt );
	 uartWriteByte(UART_USB, '\r' );
	 uartWriteByte(UART_USB, '\n' );*/


	 if(cnt == ELEMENTOS_MEMORIA){
         return TRUE;
	  }

	  return FALSE;
}

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

  char Error[] = "ERROR1 "; // Mensaje de error para el envio por la queue
  char ErrorCola[] = "ERROR COLA LLENA "; // Mensaje de error para el envio por la queue

  if(xTimer == TimerTimeout){

	 if( xTimerStop( TimerTimeout, 0) != pdPASS )
	 {
		  /* The stop command was not executed successfully. Take appropriate action here. */
	 }

	 // Verifica si la cola esta llena, si es asi, avisa por callback de salida que no puede recibir mas datos
	 // de lo contrario tratara de procesar el paquete entrante
	 fsmUARTRXState = StandBy;

	 ColaLlena=VerificaColaLlena();

     if(ColaLlena==TRUE){
       ErrorCola[16] = '\0';
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
	     gpioToggle(LED1);
	 }
	 else
	 {
	  	 rear->link = temp;
	  	 rear = temp;
	  	 gpioToggle(LED2);
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

		 for(int j = 0 ; j < 7; j++){
			caracter_out = Error[j];
			xStatusTX = xQueueSend( xQueueRecibe, &caracter_out, 0 );

		 }
		 EliminaBloqueMemoriaDinamica();
	 }

	 memset(&bufferin[0], 0, sizeof(bufferin));

  }
}

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
	char Error[] = "ERROR LIMITE EXCEDIDO "; // Mensaje de error para el envio por la queue
	char caracter_out;
	BaseType_t xStatusTX;

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
	        	 	// ERROR : se supero el limite de memoria para un paquete
	        		Error[21] = '\0';

	        		for(int j = 0 ; j < 22; j++){
	        		 	caracter_out = Error[j];
	        		 	xStatusTX = xQueueSendFromISR( xQueueRecibe, &caracter_out, 0 );
	        		}

	        		xTimerStopFromISR( TimerTimeout , &xHigherPriorityTaskWoken );
	        		memset(&bufferin[0], 0, sizeof(bufferin));
	        		uartCallbackSet(UART_USB, UART_TRANSMITER_FREE, uartUsbSendCallback, NULL);
	        		fsmUARTRXState = StandBy;

	        	 }
	        	 else
	        	 {
	        		 xTimerResetFromISR( TimerTimeout , &xHigherPriorityTaskWoken );
	        		 indicerx++;
	        		 bufferin[indicerx] = uartRxRead(UART_USB);
	        		 //uartTxWrite(UART_USB, bufferin[indicerx] );
	        	 }
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

/*****************************************************************************************
 *
 *
 *   Tarea Driver
 *
 *
 *****************************************************************************************/

void Driver( void* pvParameters )
{
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
    for( ;; ) {

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

/*****************************************************************************************
 *
 *
 *   Funcion Main
 *
 *
 *****************************************************************************************/

int main(void)
{
   /* Inicializar la placa */
   boardConfig();

   uartDriverInit(UART_USB);

   gpioInit( LED_ROJO, GPIO_OUTPUT );
   gpioInit( LED_AMARILLO, GPIO_OUTPUT );
   gpioInit( LED_VERDE, GPIO_OUTPUT );

   xTaskCreate(
	  Driver,                  // Funcion de la tarea a ejecutar
      (const char *)"Driver",    // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+1,         // Prioridad de la tarea
      0                           // Puntero a la tarea creada en el sistema
   );

   // Creacion del timer "Timeout"
   TimerTimeout = xTimerCreate ("Timeout in", TIMEOUT_VALIDATION / portTICK_RATE_MS , pdFALSE, (void *) 0 , TimeoutCallback);

   // Creacion del timer "TimeToExit"
   TimeToExit = xTimerCreate ("Time To Exit", TIMEOUT_VALIDATION / portTICK_RATE_MS , pdFALSE, (void *) 1 , TimeToExitCallback);

   // Creacion del timer "heartbeat"
   TimerHeartbeat = xTimerCreate ("Timer Heartbeat", 500 / portTICK_RATE_MS , pdTRUE, (void *) 2 , HeartbeatCallback);
   xTimerStart( TimerHeartbeat , 0 );

   // Creacion de las colas
   xQueueEnvia = xQueueCreate( 100,  sizeof( char ) );

   // Error en la creacion de la Queue
   if( xQueueEnvia == NULL ){
     /* Error fatal */
     while(1){
    	 gpioWrite(LEDR, ON);  // Indica una falla en el sistema
	 }
   }

   xQueueRecibe = xQueueCreate( 100,  sizeof( char ) );

   // Error en la creacion de la Queue
   if( xQueueRecibe == NULL )
   {
	 /* Error fatal */
	 while(1){
		 gpioWrite(LEDR, ON);   // Indica una falla en el sistema
	 }
   }

   /*
    *  Inicializacion de variables del sistema
    */

   fsmUARTRXState = StandBy;    // inicia la maquina de estados con la recepcion en standby

   /*
    *  Inicializacion del scheduler
    */

   vTaskStartScheduler();

   while(true){
      // do nothing and more nothing
   }

   return 0;
}



