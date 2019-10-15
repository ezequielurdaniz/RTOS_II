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
 * main.c
 *
 *===========================================================================*/


/*=====[Inclusions of function dependencies]=================================*/

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"
#include "crc8.h"
#include "Driver.h"
#include "TimersControl.h"
#include "Callbacks.h"
#include "ProcessLetters.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"		//Api de sincronización (sem y mutex)

/*=====[Definition macros of private constants]==============================*/


/*=====[Definitions of extern global variables]==============================*/


/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

/*!
 * @brief  Funcion main
 *
 * @param[in] void
 *
 * @return  int
 */

int main(void)
{
	   // ---------- CONFIGURACIONES ------------------------------
	   // Inicializar y configurar la plataforma
	   boardConfig();

	   // Inicializar UART_USB para conectar a la PC
	   uartDriverInit(UART_USB);

       // Inicializacion de LEDS de Debug
	   gpioInit( LED_ROJO, GPIO_OUTPUT );
	   gpioInit( LED_AMARILLO, GPIO_OUTPUT );
	   gpioInit( LED_VERDE, GPIO_OUTPUT );


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

   // Create a task in freeRTOS with dynamic memory

   xTaskCreate(
	  Driver,                  // Funcion de la tarea a ejecutar
      (const char *)"Driver",    // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+1,         // Prioridad de la tarea
      0                           // Puntero a la tarea creada en el sistema
   );

   /*
    *  Inicializacion del scheduler
    */

   vTaskStartScheduler();

   while(true){
      // do nothing and more nothing
   }

   return 0;
}



