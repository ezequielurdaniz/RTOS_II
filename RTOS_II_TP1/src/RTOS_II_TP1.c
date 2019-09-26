/*  -----------------------------------------------------------------

    RTOS II : TP 1
	
	Alumnos :
	
	Luciano Perren
	Juan Pablo Menditto
	Pablo Zizzutti
	
	
// -----------------------------------------------------------------

You need to add

DEFINES+=TICK_OVER_RTOS
DEFINES+=USE_FREERTOS

on config.mk to tell SAPI to use FreeRTOS Systick

*/

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "Capa2.h"
#include "vtask.h"
#include "sapi.h"
#include "crc8.h"



int main(void)
{
   /* Inicializar la placa */
   boardConfig();

   /* Inicializar la UART_USB junto con las interrupciones de Tx y Rx */
   uartConfig(UART_USB, 115200);

   // Seteo un callback al evento de recepcion y habilito su interrupcion
   uartCallbackSet(UART_USB, UART_RECEIVE, uartUsbReceiveCallback, NULL);

   // Seteo un callback al evento de transmisor libre y habilito su interrupcion
   uartCallbackSet(UART_USB, UART_TRANSMITER_FREE, uartUsbSendCallback, NULL);

   // Habilito todas las interrupciones de UART_USB
   uartInterrupt(UART_USB, true);
   


   xTaskCreate(
      vtask,                     // Funcion de la tarea a ejecutar
      (const char *)"vtask",     // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+1,         // Prioridad de la tarea
      0                           // Puntero a la tarea creada en el sistema
   );

   xTaskCreate(
      Capa2,                     // Funcion de la tarea a ejecutar
      (const char *)"Capa2",     // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+1,         // Prioridad de la tarea
      0                           // Puntero a la tarea creada en el sistema
   );

/*
   xTaskCreate(
      Capa3,                     // Funcion de la tarea a ejecutar
      (const char *)"Capa3",     // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+1,         // Prioridad de la tarea
      0                           // Puntero a la tarea creada en el sistema
   );
*/

   vTaskStartScheduler();

   return 0;
}
