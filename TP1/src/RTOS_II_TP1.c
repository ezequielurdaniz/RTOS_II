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

#define TIMEOUT_VALIDATION 3 // 100 ms de timeout

#define LED_ROJO     LED2
#define LED_AMARILLO LED1
#define LED_VERDE LED3

#define MEMORIADINAMICA 100

void SetCallBackUartTX();
bool_t CheckLettersFnc(char *str);
char* MinusToMayus(char *str);

void HeartbeatCallback(TimerHandle_t xTimer);
void TimeoutCallback(TimerHandle_t xTimer);

void uartUsbReceiveCallback( void *unused );
void uartUsbSendCallback( void *unused );
void capa2Task( void* pvParameters );
void capa3Task( void* pvParameters );

/* Declaracion de los Timers */
TimerHandle_t TimerTimeout;
TimerHandle_t TimerHeartbeat;
TimerHandle_t TimeToExit;

/* Declaracion de las Queue*/
QueueHandle_t xQueueEnvia;
QueueHandle_t xQueueRecibe;

//char bufferin[100];  // Variable de recepcionde datos por puerto UART
uint8_t indice = 0;
uint8_t crc_temp_rx = 0;
uint8_t crc_temp_tx = 0;

char bufferPrueba[11] = "ABCDEFGHIJ";

/* Declaraciones variables */

bool_t llegaronDatos;
bool_t finCadenaDatos;

bool_t datosListosTx;
bool_t finalizoenvioTX;
bool_t uartTxLibre;

char *pDataToSend; // Variable para el envio de datos por UART

// Definiciones de paquete
typedef enum{
   Malo,
   Bueno,
   EnEspera,
} Paquete;

Paquete EstadoPaquete; // Estado 2 : "EnEspera" : Paquete no analizado

/*-----------------------------------------------------------------------------
 *
 *  Declaraciones FSM
 *
 * ---------------------------------------------------------------------------*/

// Nombres de FSM recepcion UART
typedef enum{
   StandBy,
   Recibiendo,
   CheckCRC,
} fsmUARTRXState_t;

fsmUARTRXState_t fsmUARTRXState; // Variable para guardar el valor de la maquina de estado

// Nombres de FSM envio UART
typedef enum{
   StandByTx,
   Enviando,
   EsperaTM,
} fsmUARTXState_t;

fsmUARTXState_t fsmUARTTXState; // Variable para guardar el valor de la maquina de estado

// Nombres de FSM
typedef enum{
   DynMemory1,
   DynMemory2,
   DynMemory3,
   DynMemory4,
} fsmMemoriaState_t;

fsmMemoriaState_t fsmMemoriaState; // Guarda el sector actual disponible de memoria para usar

// Bloques de memoria dinamica
char* Sector1;
char* Sector2;
char* Sector3;
char* Sector4;

/**********************************************************************************************
 *
 *   Funciones memoria dinamica
 *
 **********************************************************************************************/
/*
void* malloc(size_t size)
{
    void* ptr = NULL;

    if(size > 0)
    {
        // We simply wrap the FreeRTOS call into a standard form
        ptr = pvPortMalloc(size);
    }   // else NULL if there was an error

    return ptr;
}

void free(void* ptr)
{
    if(ptr)
    {
        // We simply wrap the FreeRTOS call into a standard form
        vPortFree(ptr);
    }
}
*/
/**********************************************************************************************
 *
 *   Funciones comunes a todo
 *
 **********************************************************************************************/

void SetCallBackUartTX(){
	uartCallbackSet(UART_USB, UART_TRANSMITER_FREE, uartUsbSendCallback, NULL);
}


/**********************************************************************************************
 *
 *   Funciones capa 3
 *
 **********************************************************************************************/

bool_t CheckLettersFnc(char *str){

	 for (uint8_t i = 0; str[i] != '\0'; i++){

		 if(!isalpha(str[i])){ // verifica si el caracter es alfabetico
			 return false;  // devuelve error si encuentra un caracter no alfabetico
		 }

	 }

	 return true; // Si llega hasta aca, recorrio toda la cadena y encontro caracteres alfabeticos

}

char* MinusToMayus(char *str){

	 for (uint8_t i = 0; str[i] != '\0'; i++)
	 {
		 str[i] = toupper((char)(str[i]));
	 }

	return str;

}

/**********************************************************************************************
 *
 *    Callbacks - Implementaciones
 *
 **********************************************************************************************/

void HeartbeatCallback(TimerHandle_t xTimer){
	gpioToggle(LEDB);
}

void TimeToExitCallback(TimerHandle_t xTimer){

   if(xTimer == TimeToExit){
	   xTimerStop( TimeToExit , 0 ); // detiene el timer
	   xTimerReset( TimeToExit , 0 );
	   uartTxLibre = TRUE; 		   // avisa que se puede enviar un proximo paquete
   }

}

void TimeoutCallback(TimerHandle_t xTimer){

  if(xTimer == TimerTimeout){

	 crc_temp_rx = crc8_calc(0 , Sector1 , indice-1);                  // realizo el calculo del CRC

	 if(crc_temp_rx == Sector1[indice-1])
	 {
	 	 // Llego un paquete bueno, paso a la siguiente capa
		 EstadoPaquete = Bueno;
	 }
	 else
	 {
	     // Llego un paquete malo, devuelvo error por el puerto
		 EstadoPaquete = Malo;
	 }
	 finCadenaDatos = TRUE;
  }

}

void uartUsbReceiveCallback( void *unused )
{
   Sector1[indice] = uartRxRead(UART_USB);	    // guarda en el buffer de entrada
   indice++;	  							    // incrementa el indice de recepcion por uart
   llegaronDatos = TRUE;

/*
   // FSM Update Sate Function
   switch( fsmMemoriaState ){
        case DynMemory1:
        /* UPDATE OUTPUTS */

        /* CHECK TRANSITION CONDITIONS */
 //       if( llegaronDatos  == TRUE ){
  //      	fsmMemoriaState = DynMemory2;
 //       }
 ////  	    break;
  // }

}

// Envio a la PC desde la UART_USB hasta NULL y deshabilito Callback
void uartUsbSendCallback( void *unused )
{
	while (*pDataToSend != '\0') {
		if (uartTxReady(UART_USB)) {
			uartTxWrite(UART_USB, *pDataToSend++);
		} else
			break;
	}
	if (*pDataToSend == '\0') {
		uartTxWrite(UART_USB, '\r');
		uartTxWrite(UART_USB, '\n');
		uartCallbackClr( UART_USB, UART_TRANSMITER_FREE);
		finalizoenvioTX = TRUE;
	}
}


/*****************************************************************************************
 *
 *
 *   Tarea Capa 2
 *
 *
 *****************************************************************************************/

void capa2Task( void* pvParameters )
{
	/* Configuracion inicial de la tarea */
	char *lValueToSend;	                   // el valor a enviar por la queue
	char *lReceivedValue;			       // el valor que recibe en la tare por la queue
	BaseType_t xStatusTX;                  // Variable de status de la queue
	BaseType_t xStatusRX;                  // Variable de status de la queue
	const TickType_t xTicksToWait = 0;     // Ticks to wait

	const char MensajeError[] = "ERROR\0"; // Mensaje de error para el envio por la queue

	/* loop infinito de la tarea */
	for( ;; ) {

	   // FSM Recepcion de Uart
	   switch( fsmUARTRXState ){

	         case StandBy:
	            /* UPDATE OUTPUTS */
	            gpioWrite( LED_ROJO, OFF );
	        	gpioWrite( LED_AMARILLO, ON );
	            /* CHECK TRANSITION CONDITIONS */
	            if( llegaronDatos  == TRUE ){              // Chequea si llego un dato por uart
	            	xTimerStart( TimerTimeout , 0 );
	            	fsmUARTRXState = Recibiendo;
	            }
	         break;

	         case Recibiendo:
	            /* UPDATE OUTPUTS */
	        	gpioWrite( LED_ROJO, ON );
	        	gpioWrite( LED_AMARILLO, OFF );

	        	 if( llegaronDatos  == TRUE ){
	        		 llegaronDatos  = FALSE;
	        		 xTimerReset( TimerTimeout , 0 );
	        	 }

	            /* CHECK TRANSITION CONDITIONS */
	        	if( finCadenaDatos  == TRUE ){
	        		xTimerStop( TimerTimeout , 0 );
	        		llegaronDatos = FALSE;
	        	    finCadenaDatos = FALSE;
	        	    indice=0;
	        		fsmUARTRXState = CheckCRC;
	        	}
	         break;

	         case CheckCRC:
	        	 /* UPDATE OUTPUTS */
	        	 gpioWrite( LED_ROJO, ON );
	        	 gpioWrite( LED_AMARILLO, ON );

	        	 if(EstadoPaquete == Bueno){

	        		// Cargar el valor en la queue y enviarlo a la capa3
	        		lValueToSend = Sector1;
	        		lValueToSend[strlen(Sector1) - 1] = '\0';   // Para el envio a la capa 3, elimino el ultimo caracter, que es
	        		 	        				        		// el CRC8

	        		xStatusTX = xQueueSend( xQueueEnvia, &lValueToSend, xTicksToWait ); // envio datos a la capa 3

	        		if( xStatusTX == pdPASS ) {
	        			// Ok
	        		}
	        		else
	        		{
	        			pDataToSend = (char*) MensajeError;
	        			datosListosTx = TRUE;
	        		}

	        	 }else if(EstadoPaquete == Malo){
	        		 pDataToSend = (char*) MensajeError;
	        		 datosListosTx = TRUE;
	        	 }

	        	 EstadoPaquete = EnEspera;
	        	 fsmUARTRXState = StandBy;

	         break;
	   }

	   // FSM Envio por Uart
	   switch( fsmUARTTXState ){

	       case StandByTx:
	   	      /* UPDATE OUTPUTS */
	   	      gpioWrite( LED_VERDE, OFF );
	   	      /* CHECK TRANSITION CONDITIONS */
	   	      if( datosListosTx  == TRUE ){             // Espera a que se avise que los datos estan listos para el envio por TX
	   	          datosListosTx = FALSE;
	   	    	  fsmUARTTXState = Enviando;		    // indica un cambio de la FSM

	   	    	  crc_temp_tx = crc8_calc(0 , pDataToSend , strlen(pDataToSend) ); // realizo el calculo del CRC8

	   	    	  pDataToSend[strlen(pDataToSend)] = (char) crc_temp_tx;  		  // en la ultima posicion coloco el CRC8
	   	    	  pDataToSend[strlen(pDataToSend) + 1] = '\0';            		  // agrego el caracter nulo que fue sobreescrito
	   	    		    	 		 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	  // en la operacion anterior
	   	    	  crc_temp_tx = 0;										 	      // borro la variable temporal de calculo CRC8 de TX

	   	    	  SetCallBackUartTX();											  // Habilito el Callback de TX Uart
	   	      }
	   	   break;

	       case Enviando:
	       	   /* UPDATE OUTPUTS */
	    	   gpioWrite( LED_VERDE, ON );

	       	   /* CHECK TRANSITION CONDITIONS */
	       	   if( finalizoenvioTX  == TRUE ){
	       	     finalizoenvioTX = FALSE;
	        	 xTimerStart( TimeToExit , 0 ); // inicia el timer para esperar el proximo paquete
	       	     fsmUARTTXState = EsperaTM;     // indica un cambio de la FSM
	       	   }
	       	break;

	        case EsperaTM:
	             gpioWrite( LED_VERDE, OFF );

	        	 /* CHECK TRANSITION CONDITIONS */
	        	 if( uartTxLibre  == TRUE ){
	        		 uartTxLibre = FALSE;
	        		 fsmUARTTXState = StandByTx;  // indica un cambio de la FSM
	        	 }
	    	break;
	   }

	   // Recepcion de los datos de la queue
	   xStatusRX = xQueueReceive( xQueueRecibe, &lReceivedValue, xTicksToWait );

	   if( xStatusRX == pdPASS ) {

	     /* Al dato recibido, se le agrega el crc8 y se reenvia por uart */
		 pDataToSend = lReceivedValue;                                    // copio el contenido de la queue a la variable
		 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	  // para envio por uart
		 datosListosTx = TRUE;											  // avisa que esta listo para enviar datos

	   }
	   else {

	   }

   } // bucle infinito for

}
/*****************************************************************************************
 *
 *
 *   Tarea Capa 3
 *
 *
 *****************************************************************************************/

void capa3Task( void* pvParameters )
{
	/* Configuracion inicial de la tarea */
	char *lValueToSend;	 // el valor a enviar por la cola
	char *lReceivedValue;

	BaseType_t xStatusRX;  // Variable de status de la queue
	BaseType_t xStatusTX;  // Variable de status de la queue

	const TickType_t xTicksToWait = 0;
	bool_t EstadoPaquete;

	const char MensajeError[] = "ERROR\0"; // Mensaje de error para el envio por la queue

    /* loop infinito de la tarea */
    for( ;; ) {

    	xStatusRX = xQueueReceive( xQueueEnvia, &lReceivedValue, xTicksToWait );

    	if( xStatusRX == pdPASS ) {
    	   /* Data was successfully received from the queue, print out the received
    	    value. */

    	   // Carga el char* con un valor inicial que es el de la cola de recepcion
    	   lValueToSend = lReceivedValue;

    	   // Chequea el paquete entrante para verificar si son letras
    	   EstadoPaquete = CheckLettersFnc(lReceivedValue);

    	   // Si el paquete esta ok, tiene todas letras, lo convierte a mayusculas y lo envia por la queue
    	   // sino devuelve error por la queue

    	   if(EstadoPaquete == true ){
    		   lValueToSend = MinusToMayus(lReceivedValue); // Se envia el mensaje convertido
    	   }
    	   else
    	   {
    		   lValueToSend = (char*) MensajeError; 		// Se envia el mensaje de ERROR
    	   }

    	   xStatusTX = xQueueSend( xQueueRecibe, &lValueToSend, xTicksToWait );

    	} else {
    	  /* We did not receive anything from the queue even after waiting for 100ms.
    	  This must be an error as the sending tasks are free running and will be
    	  continuously writing to the queue. */
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

   Sector1 = pvPortMalloc(MEMORIADINAMICA);
   Sector2 = pvPortMalloc(MEMORIADINAMICA);
   Sector3 = pvPortMalloc(MEMORIADINAMICA);
   Sector4 = pvPortMalloc(MEMORIADINAMICA);
   /*
   vPortFree(Sector1);
   vPortFree(Sector2);
   vPortFree(Sector3);
   vPortFree(Sector4);
   *

/*
   char* Sector2 =  pvPortMalloc(100);

   for(int i = 0 ; i < 100 ; i++){
   	     Sector2[i] = 1;
   }

   for(int j = 0 ; j < 100 ; j++){
   	     uartTxWrite(UART_USB, Sector2[j] );
   }
*/

   // Seccion para pruebas del CRC8
   //-
   /*crc_temp_rx = crc8_calc(0 , bufferPrueba , 10);                  // realizo el calculo del CRC

   bufferPrueba[10] = crc_temp_rx;

   crc_temp_rx = 0;

   for(int i = 0 ; i < sizeof(bufferPrueba) ; i++){
	   uartTxWrite(UART_USB, bufferPrueba[i] );
   }*/
   //- Seccion para pruebas del CRC8

   xTaskCreate(
      capa2Task,                  // Funcion de la tarea a ejecutar
      (const char *)"capa2Task",   // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+1,         // Prioridad de la tarea
      0                           // Puntero a la tarea creada en el sistema
   );

   xTaskCreate(
	  capa3Task,                  // Funcion de la tarea a ejecutar
      (const char *)"capa3Task",    // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+1,         // Prioridad de la tarea
      0                           // Puntero a la tarea creada en el sistema
   );

   // Creacion del timer "Timeout"
   TimerTimeout = xTimerCreate ("Timeout in", TIMEOUT_VALIDATION / portTICK_RATE_MS , pdTRUE, (void *) 0 , TimeoutCallback);

   // Creacion del timer "TimeToExit"
   TimeToExit = xTimerCreate ("Time To Exit", TIMEOUT_VALIDATION / portTICK_RATE_MS , pdTRUE, (void *) 0 , TimeToExitCallback);

   // Creacion del timer "heartbeat"
   TimerHeartbeat = xTimerCreate ("Timer Heartbeat", 500 / portTICK_RATE_MS , pdTRUE, (void *) 0 , HeartbeatCallback);
   xTimerStart( TimerHeartbeat , 0 );

   // Creacion de las colas
   xQueueEnvia = xQueueCreate( 1, sizeof( Sector1 ));

   // Error en la creacion de la Queue
   if( xQueueEnvia == 0 )
   {
	  while(1){
	    gpioToggle(LEDR);  // Indica una falla en el sistema
	  }
   }

   xQueueRecibe = xQueueCreate( 1, sizeof( Sector1 ));

   // Error en la creacion de la Queue
   if( xQueueRecibe == 0 )
   {
	 while(1){
	    gpioToggle(LEDR);   // Indica una falla en el sistema
	 }
   }

   vTaskStartScheduler();

   // Inicializacion de variables del sistema

   fsmUARTRXState = StandBy;    // inicia la maquina de estados con la recepcion en standby
   fsmUARTTXState = StandByTx;  // inicia la maquina de estado con la transmision en standby

   llegaronDatos = FALSE;
   finCadenaDatos = FALSE;

   datosListosTx = FALSE;
   finalizoenvioTX = FALSE;
   uartTxLibre = TRUE;

   EstadoPaquete = EnEspera;


   while(true){
      // do nothing and more nothing
   }

   return 0;
}



