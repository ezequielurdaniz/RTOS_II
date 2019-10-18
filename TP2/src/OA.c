/*============================================================================
 * Autores: Luciano Perren       <lgperren@gmail.com>
 *          Juan Pablo Menditto  <jpmenditto@gmail.com>
 *          Pablo Zizzutti       <pablozizzutti@gmail.com>
 * TP2 RTOS 2
 * Date: 2019/09/10
 * Docentes: Franco Bucafusco y Sergio Renato De Jesús Melean
 * OA.c
 *===========================================================================*/

/*==================[inlcusiones]============================================*/

//Includes de FreeRTOS
#include "FreeRTOS.h"    		// Motor del SO
#include "FreeRTOSConfig.h"
#include "task.h"				// Api de control de tareas y temporización
#include "semphr.h"				// Api de sincronización (sem y mutex)
#include "queue.h"				// Api de colas

#include "ctype.h"
#include "string.h"

// sAPI header
#include "sapi.h"
#include "board.h"

#include "OA.h"	/* Active Object interface */

/*================[declaración de funciones internas]========================*/


/*====================[definición de datos internos]=========================*/

// Inicializacion de variables para verificacion de OA activos

bool_t CorriendoMayus=false;
bool_t CorriendoMinus=false;

/*==================[definiciones y macros]==================================*/

/*==================[declaraciones de funciones externas]====================*/

/*==================[Active Object Initialization]======================================*/

void ActiveObject_Init( Active_Object_t* obj ) {

	if(obj->TipoOperacion==1){

		if(obj->CorriendoMinus==false){

		}
    /* Crear tarea en freeRTOS */
	xTaskCreate(
	   ActiveObject_App,					/* Funcion de la tarea a ejecutar (en OA.h) */
       (const char *)"AO_application",    	/* Nombre de la tarea como String amigable para el usuario */
       configMINIMAL_STACK_SIZE*2,      	/* Cantidad de stack de la tarea */
       obj,                               	/* El parametro define la función UPPER o LOWER */
       tskIDLE_PRIORITY+1,              	/* Prioridad de la tarea */
       &xHandle		                		/* Puntero a la tarea creada en el sistema */
    );
	   // Si la tarea se creo correctamente
	   if(xHandle!=NULL){
		   CorriendoMinus=true;
	   }


	}

	/* Inicializo elementos de la estructura */
	xQueueOA= xQueueCreate( MEMORIADINAMICA,  sizeof( char ) );	/* Creo cola */

	// Error en la creacion de la Queue
	if( xQueueOA == NULL )
	{   /* Error fatal */
	    gpioWrite(LEDR, ON);   // Indica una falla en el sistema
	}

}

void ActiveObject_App( void* param  ) { 	/* Active Object application */

	Active_Object_t* obj = ( Active_Object_t* ) param;		/* Casting */

	while(1) {



         if(xQueueReceive(obj->xQueue, obj->buffer_IN ,0 ) == pdTRUE){
             //
         }
         else
         {


        	 // llamar a destructor del obj
			 CorriendoMinus=false;
			 vTaskDelete(NULL);
         }


	}
}

/*==================[internal functions definition]==========================*/


