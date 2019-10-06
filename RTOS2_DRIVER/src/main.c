/*=============================================================================
 * Copyright (c) 2019, Eric Pernia <ericpernia@gmail.com>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2019/08/23
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"		//Api de sincronización (sem y mutex)


#include "sapi.h"
#include "capa3.h"



/*=====[Definition macros of private constants]==============================*/
#define UART_PC        UART_USB

/*=====[Definitions of extern global variables]==============================*/


/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/
SemaphoreHandle_t Evento_pulsado, Mutex_t_pulsado, Mutex_UART; //Para el uso de semaforos
portTickType TiempoPulsado;  //Para la periodicidad
/*=====[Main function, program entry point after power on or reset]==========*/

DEBUG_PRINT_ENABLE;  //Para configurar los mensajes por monitor

int main( void )
{
	uint8_t Error_state = 0;

	   // ---------- CONFIGURACIONES ------------------------------
	   // Inicializar y configurar la plataforma
	   boardConfig();

	   // Inicializar UART_USB para conectar a la PC
	   uartConfig( UART_PC, 9600 );
	   uartWriteString( UART_PC, "UART_PC configurada.\r\n" );

	   /* Attempt to create a semaphore. */
	   if (NULL == (Evento_pulsado = xSemaphoreCreateBinary()))   // la crea y comprueba al mismo tiempo
	   {
		   Error_state =1;
	   }

	   if (NULL == (Mutex_t_pulsado = xSemaphoreCreateMutex()))
	   {
	   	   Error_state =1;
	   }

	   if (NULL == (Mutex_UART = xSemaphoreCreateMutex()))
	   {
		   Error_state =1;
	   }


   // Create a task in freeRTOS with dynamic memory


   xTaskCreate(
	   validar,                     // Function that implements the task.
      (const char *)"validar",     // Text name for the task.
      configMINIMAL_STACK_SIZE*2, // Stack size in words, not bytes.
      0,                          // Parameter passed into the task.
      tskIDLE_PRIORITY+1,         // Priority at which the task is created.
      0                           // Pointer to the task created in the system
   );






   vTaskStartScheduler(); // Initialize scheduler

   while( true ); // If reach heare it means that the scheduler could not start

   // YOU NEVER REACH HERE, because this program runs directly or on a
   // microcontroller and is not called by any Operating System, as in the 
   // case of a PC program.
   return 0;
}
