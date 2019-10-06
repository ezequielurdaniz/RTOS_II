/* -----------------------------------------------------------------
 *
 *  RTOS II : TP 1
 *
 *  Alumnos :
 *
 *  Luciano Perren
 *  Juan Pablo Menditto
 *  Pablo Zizzutti
 *
 ----------------------------------------------------------------- */

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"
#include "crc8.h"
#include "timers.h"

#define TIMEOUT_VALIDATION 500 // 500 ms de timeout

#define LED_ROJO     LED2
#define LED_AMARILLO LED1

#define MEMORIADINAMICA 100

void HeartbeatCallback(TimerHandle_t xTimer);
void TimeoutCallback(TimerHandle_t xTimer);

void uartUsbReceiveCallback( void *unused );
void uartUsbSendCallback( void *unused );
void capa2Task( void* pvParameters );

TimerHandle_t TimerTimeout;
TimerHandle_t TimerHeartbeat;

//char bufferin[100];  // Variable de recepcionde datos por puerto UART
uint8_t indice = 0;
uint8_t crc_temp = 0;

//char bufferPrueba[11] = "1123456789w";

/* Declaraciones variables */

bool_t llegaronDatos;
bool_t finCadenaDatos;

// Definiciones de paquete
typedef enum{
   Malo,
   Bueno,
   EnEspera,
} Paquete;

Paquete EstadoPaquete; // Estado 2 : "EnEspera" : Paquete no analizado

// --------------------------------
//        Declaraciones FSM
// --------------------------------

// Nombres de FSM
typedef enum{
   StandBy,
   Recibiendo,
   CheckCRC,
} fsmUARTRXState_t;

// Variable that hold the current state
fsmUARTRXState_t fsmUARTRXState;

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
 *    Callbacks - Implementaciones
 *
 **********************************************************************************************/

void HeartbeatCallback(TimerHandle_t xTimer){
	gpioToggle(LEDB);
}

void TimeoutCallback(TimerHandle_t xTimer){

  if(xTimer == TimerTimeout){
	 finCadenaDatos = TRUE;

	 crc_temp = crc8_calc(0 , Sector1 , indice-1);                  // realizo el calculo del CRC

	 if(crc_temp == Sector1[indice-1])
	 {
	 	 // Llego un paquete bueno, paso a la siguiente capa
		 EstadoPaquete = Bueno;
	 }
	 else
	 {
	     // Llego un paquete malo, devuelvo error por el puerto
		 EstadoPaquete = Malo;
	 }
  }

}

void uartUsbReceiveCallback( void *unused )
{
   Sector1[indice] = uartRxRead(UART_USB);	    // guarda en el buffer de entrada
   indice++;	  							    // incrementa el indice de recepcion por uart
   llegaronDatos = TRUE;


   // FSM Update Sate Function
   switch( fsmMemoriaState ){
        case DynMemory1:
        /* UPDATE OUTPUTS */

        /* CHECK TRANSITION CONDITIONS */
        if( llegaronDatos  == TRUE ){
        	fsmMemoriaState = DynMemory2;
        }
   	    break;
   }

}

// Envio a la PC desde la UART_USB hasta NULL y deshabilito Callback
void uartUsbSendCallback( void *unused )
{
    if(uartTxReady(UART_USB) && EstadoPaquete==Malo){

    	uartWriteString(UART_USB, "ERROR! Paquete Malo\n");

   	}else if(uartTxReady(UART_USB) && EstadoPaquete==Bueno){

   	    uartWriteString(UART_USB, "Paquete bueno : ");

   		for(int i = 0 ; i < MEMORIADINAMICA ; i++){
   		    uartTxWrite(UART_USB, Sector1[i] );
   		}

   		uartWriteString(UART_USB, "\r\n");
   	}

	/*
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
		uartCallbackClr( UART_USB, UART_TRANSMITER_FREE);
	}
    }
	}*/
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
   while(TRUE) {

	   // FSM Update Sate Function
	   switch( fsmUARTRXState ){

	         case StandBy:
	            /* UPDATE OUTPUTS */
	            gpioWrite( LED_ROJO, OFF );
	        	gpioWrite( LED_AMARILLO, ON );
	            /* CHECK TRANSITION CONDITIONS */
	            if( llegaronDatos  == TRUE ){    // chequea si llego un dato por uart
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

	        	 uartCallbackSet(UART_USB, UART_TRANSMITER_FREE, uartUsbSendCallback, NULL);

	        	 // Vacio buffer
	        	 for(int i = 0 ; i < MEMORIADINAMICA ; i++){
	        		 Sector1[i] = ' ';
	        	 }

	        	 EstadoPaquete = EnEspera;
	        	 fsmUARTRXState = StandBy;
	        	 uartCallbackClr( UART_USB, UART_TRANSMITER_FREE);

	         break;
	   }
      //vTaskDelay(500/portTICK_RATE_MS);
   }
}

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

   uartWriteString(UART_USB, "Iniciando ...\n"); // Mensaje de inicio

   Sector1 = pvPortMalloc(MEMORIADINAMICA);
   Sector2 = pvPortMalloc(MEMORIADINAMICA);
   Sector3 = pvPortMalloc(MEMORIADINAMICA);
   Sector4 = pvPortMalloc(MEMORIADINAMICA);

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
   /*crc_temp = crc8_calc(0 , bufferPrueba , 10);                  // realizo el calculo del CRC

   bufferPrueba[10] = crc_temp;

   crc_temp = 0;

   for(int i = 0 ; i < sizeof(bufferPrueba) ; i++){
	   uartTxWrite(UART_USB, bufferPrueba[i] );
     }*/
   //- Seccion para pruebas del CRC8

   xTaskCreate(
      capa2Task,                  // Funcion de la tarea a ejecutar
      (const char *)"tickTask",   // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+1,         // Prioridad de la tarea
      0                           // Puntero a la tarea creada en el sistema
   );

   // Creacion del timer "Timeout"
   TimerTimeout = xTimerCreate ("Timeout in", TIMEOUT_VALIDATION / portTICK_RATE_MS , pdTRUE, (void *) 0 , TimeoutCallback);

   // Creacion del timer "heartbeat"
   TimerHeartbeat = xTimerCreate ("Timer Heartbeat", 500 / portTICK_RATE_MS , pdTRUE, (void *) 0 , HeartbeatCallback);
   xTimerStart( TimerHeartbeat , 0 );

   // Creacion de las colas


   vTaskStartScheduler();

   fsmUARTRXState = StandBy;  // inicia la maquina de estados con la recepcion en standby

   llegaronDatos = FALSE;
   finCadenaDatos = FALSE;

   EstadoPaquete = EnEspera;

   while(TRUE){

	   vTaskDelay(500/portTICK_RATE_MS);
   }

   return 0;
}



