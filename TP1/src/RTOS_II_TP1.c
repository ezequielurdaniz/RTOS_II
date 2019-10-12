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

char bufferin[100];  // Variable de recepcionde datos por puerto UART

/*-----------------------------------------------------------------------------
 *
 *  Declaraciones FSM
 *
 * ---------------------------------------------------------------------------*/

// Nombres de FSM recepcion UART
typedef enum{
   StandBy,
   Recibiendo,
} fsmUARTRXState_t;

fsmUARTRXState_t fsmUARTRXState; // Variable para guardar el valor de la maquina de estado


// Memoria dinamica
#define ELEMENTOS_MEMORIA 4

struct node
{
    char datos[MEMORIADINAMICA];
    struct node *link;

}*front, *rear;

struct node *temp;
//struct node *temp;



/**********************************************************************************************
 *
 *   Funciones comunes a todo
 *
 **********************************************************************************************/



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

  char car_recibido;

  char MensajeError[] = "ERROR\0"; // Mensaje de error para el envio por la queue

  if(xTimer == TimerTimeout){

	  if( xTimerStop( TimerTimeout, 0) != pdPASS )
	  {
		  /* The stop command was not executed successfully.  Take appropriate
		  action here. */
	  }

	  temp->link = NULL;

	  if (rear  ==  NULL)
	  {
	  	 front = rear = temp;
	  }
	  else
	  {
	  	 rear->link = temp;
	  	 rear = temp;
	  }

	 fsmUARTRXState = StandBy;

	 indice = strlen(temp->datos);
	 //uartWriteByte(UART_USB, indice );
	 //bufferin[indice] = '\0';

	 crc_temp_rx = crc8_calc(0 , temp->datos , indice-1);                  // realizo el calculo del CRC

	 if(crc_temp_rx == temp->datos[indice-1])
	 {
	 	 // Llego un paquete bueno, paso a la siguiente capa
		 temp->datos[indice -1] = '\0'; // Sobreescribo el caracter del CRC8 por el \0

		 for(int i = 0 ; i < indice; i++){
			 car_recibido = temp->datos[i];
		 	 xStatusTX = xQueueSend( xQueueEnvia, &car_recibido, 0 ); // envio datos por Queue
		 }

		 if( xStatusTX == pdPASS ){
		 	// OK EN EL ENVIO DE LA COLA
			 gpioToggle(LED3);
		 }
		 else
		 {
		 	// ERROR ENVIO EN LA COLA
			gpioToggle(LED1);
		 }
	 }
	 else
	 {
	     // Llego un paquete malo, devuelvo error por el puerto
		 gpioToggle(LED2);

		 for(int j = 0 ; j < 6; j++){
			car_recibido = MensajeError[j];
			xStatusTX = xQueueSend( xQueueRecibe, &car_recibido, 0 );
		 }

		 uartCallbackSet(UART_USB, UART_TRANSMITER_FREE, uartUsbSendCallback, NULL);
	 }

	 memset(&temp->datos[0], 0, sizeof(temp->datos)); // clear the array
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

	int cnt;


	// FSM Recepcion de Uart
	   switch( fsmUARTRXState ){

	         case StandBy:
	        	 indicerx=0;
	        	 // Recibimos el primer elemento por UART
	        	 // Verificar el tamaño de la cola que no sea mas grande que el maximo permitido : ELEMENTOS_MEMORIA

	        	 temp = front;

	        	 cnt = 0;

	        	 if (front == NULL)
	        	 {
	        	   // Envia por callback de tansmision de UART el error, utilizando queue
	        		 uartWriteString(UART_USB, "Cola Vacia\r\n" );
	        	 }
	        	 while (temp)
	        	 {
	        	    temp = temp->link;
	        	    cnt++;
	        	 }
	        	 uartWriteString(UART_USB, "Tamanio de la cola : " );
	        	 uartWriteByte(UART_USB, (char) cnt );
	        	 uartWriteString(UART_USB, "\r\n" );

	        	 gpioToggle(LED1);

	        	 if(cnt == ELEMENTOS_MEMORIA){
	        		 // Envia por callback de tansmision de UART el error, utilizando queue
	        		 uartWriteString(UART_USB, "ERROR MEMORIA LLENA\r\n" );
	        		 // SALIR!!!!
	        		 break;
	        	 }

	        	 gpioToggle(LED2);

	        	 // crea el nuevo elemento en la queue de memoria dinamica
	        	 temp = (struct node*)pvPortMalloc(sizeof(struct node));

	        	 gpioToggle(LED3);

	        	 temp->datos[indicerx] = uartRxRead(UART_USB);
	        	 uartWriteByte(UART_USB, temp->datos[indicerx] );
	        	 //uartWriteByte(UART_USB, bufferin[indicerx] );
	             xTimerStartFromISR( TimerTimeout , &xHigherPriorityTaskWoken );
	             fsmUARTRXState = Recibiendo;
	         break;

	         case Recibiendo:
	        	 xTimerResetFromISR( TimerTimeout , &xHigherPriorityTaskWoken );
	        	 indicerx++;
	        	 temp->datos[indicerx] = uartRxRead(UART_USB);

	        	// uartWriteByte(UART_USB, temp->datos[indicerx] );
	        	 // Compruebo que no excede el limite de la memoria dinamica
	        	 if(indicerx > (MEMORIADINAMICA-1)){
	        	   // ERROR : se supero el limite de memoria para un paquete
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
	uint8_t crc_temp_tx;

	char caracter_in;

	char lReceivedValue[MEMORIADINAMICA];
	uint8_t indice = 0;

	 // Recepcion de los datos de la queue

	 /* Se recibe un caracter y se almacena */
	 if(xQueueReceiveFromISR( xQueueRecibe, &caracter_in, NULL ) == pdTRUE){
		 /* Se recibe un caracter y se almacena */
		 lReceivedValue[indice] = caracter_in;

		 if(lReceivedValue[indice] != '\0'){
		     /* Final del mensaje*/
			 uartWriteByte(UART_USB, caracter_in );
			 indice++;

		  }
		  else
		  {
			 indice = 0;
			 memset(&lReceivedValue[0], 0, sizeof(lReceivedValue)); // clear the array
			 uartWriteByte(UART_USB, '\r' );
			 uartWriteByte(UART_USB, '\n' );
			 uartCallbackClr( UART_USB, UART_TRANSMITER_FREE);
			 vPortFree(temp);
		 }

	 }


	 /*

	    	 if( xStatusRX == pdPASS ){
	    		 //gpioToggle(LED1);
	    	   	 pDataToSend = lReceivedValue;       // copio el contenido de la queue a la variable para envio por uart

	    	 // Al dato recibido, se le agrega el crc8 y se reenvia por uart
	    	 crc_temp_tx = crc8_calc(0 , pDataToSend , strlen(pDataToSend) ); // realizo el calculo del CRC8

	   	     pDataToSend[strlen(pDataToSend)] = (char) crc_temp_tx;  		  // en la ultima posicion coloco el CRC8
	   	     pDataToSend[strlen(pDataToSend) + 1] = '\0';            		  // agrego el caracter nulo que fue sobreescrito
	   	    		    	 		 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	  // en la operacion anterior
	   	     crc_temp_tx = 0;										 	      // borro la variable temporal de calculo CRC8 de TX

	  		   for(int i = 0 ; pDataToSend[i] != '\0' ; i++){
	  		        uartTxWrite(UART_USB, pDataToSend[i] );
	  		   }
	  		   uartCallbackClr( UART_USB, UART_TRANSMITER_FREE);

	    	 }
	    	 else
	    	 {
	    		 // ERROR ENVIO EN LA COLA
	    	 }
*/
	    	   //xTimerStart( TimeToExit , 0 );
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

	const char MensajeError[] = "ERROR"; // Mensaje de error para el envio por la queue

	char caracter_in;
	char caracter_out;

    /* loop infinito de la tarea */
    for( ;; ) {

    	if(xQueueReceive( xQueueEnvia, &caracter_in, xTicksToWait ) == pdTRUE){
    		/* Se recibe un caracter y se almacena */
    		lReceivedValue[indice] = caracter_in;

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
				   for(int j = 0 ; j < 6; j++){
					 caracter_out = MensajeError[j];
				     xStatusTX = xQueueSend( xQueueRecibe, &caracter_out, 0 );
				   }
			   }

               indice = 0;
               memset(&lValueToSend[0], 0, sizeof(lValueToSend)); // clear the array
               memset(&lReceivedValue[0], 0, sizeof(lReceivedValue)); // clear the array
               uartCallbackSet(UART_USB, UART_TRANSMITER_FREE, uartUsbSendCallback, NULL);

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

void uartDriverInit(void) {

	/* Inicializar la UART_USB junto con las interrupciones de Tx y Rx */
	uartConfig(UART_USB, 115200);
	// Seteo un callback al evento de recepcion y habilito su interrupcion
	uartCallbackSet(UART_USB, UART_RECEIVE, uartUsbReceiveCallback, NULL);
	// Seteo un callback al evento de transmisor libre y habilito su interrupcion
	uartCallbackSet(UART_USB, UART_TRANSMITER_FREE, uartUsbSendCallback, NULL);
	// Habilito todas las interrupciones de UART_USB
	uartInterrupt(UART_USB, true);
	// Clear el callback para transmision del UART
	uartCallbackClr(UART_USB, UART_TRANSMITER_FREE);
	// Mensaje de inicio
	uartWriteString(UART_USB, "Iniciando...\r\n");

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

   uartDriverInit();

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

   // Inicializacion de variables del sistema

   fsmUARTRXState = StandBy;    // inicia la maquina de estados con la recepcion en standby

   /*
   for(int i = 0 ; i < 100 ; i++){
	  startDynamicMem->datos[i] = 'u';
   }

   startDynamicMem->link = NULL;

         if (rear  ==  NULL)
         {
             front = rear = startDynamicMem;
         }
         else
         {
             rear->link = startDynamicMem;
             rear = startDynamicMem;
         }


   if (front == NULL)
	  uartWriteString(UART_USB, "cOLA vACIA\r\n");
   else
      uartWriteString(UART_USB, "HAY ELEMENTOS EN LA COLA\r\n");

   */




/*
       for(int i = 0 ; i < 100 ; i++){
    	   temp->datos[i] = 'u';
       }

       for(int i = 0 ; i < 100 ; i++){
          (temp+1)->datos[i] = 'f';
       }

       for(int i = 0 ; i < 100 ; i++){
    	   uartWriteByte(UART_USB, temp->datos[i]);
       }

       for(int i = 0 ; i < 100 ; i++){
    	   uartWriteByte(UART_USB, (temp+1)->datos[i]);
         }

       temp->link = NULL;

       if (rear  ==  NULL)
       {
           front = rear = temp;
       }
       else
       {
           rear->link = temp;
           rear = temp;
       }
*/

   vTaskStartScheduler();

   while(true){
      // do nothing and more nothing
   }

   return 0;
}



