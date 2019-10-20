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

/*==================[definiciones y macros]==================================*/

/*==================[declaraciones de funciones externas]====================*/

/*==================[Active Object Initialization]======================================*/

void ActiveObject_Init( Active_Object_t* obj ) {

	BaseType_t xReturned;

	// Si se solicita crear un objeto activo para mayusculizar
	if(obj->ComandoOA==1){

	  /* Crear tarea en freeRTOS */
	  xReturned = xTaskCreate(
		  AO_Mayus,
	      (const char *)"AO_Mayusculizar",
	      configMINIMAL_STACK_SIZE*2,
	      obj,
	      tskIDLE_PRIORITY+1,
	      &obj->xHandleOA
	      );

		  // Si la tarea se creo correctamente
		  if(obj->xHandleOA!=NULL){

		  }

		  /* Inicializo elementos de la estructura */
		  obj->xQueueOA = xQueueCreate( 1, sizeof( char * ));	/* Creo cola */

		  // Error en la creacion de la Queue
		  if( obj->xQueueOA == NULL ){
		  	 /* Error fatal */
		  	 // Indica una falla en el sistema
		  }
	}
	// Si se solicita crear un objeto activo para minusculizar

}




/*
 *
 *  Task para mayusculizar
 *
 */

void AO_Mayus( void* param  ) { 	                        /* Active Object application */

	Active_Object_t* obj = ( Active_Object_t* ) param;		/* Casting */

	while(1) {

		 for (int i = 0; obj->datos[i] != '\0'; i++){
		      obj->datos[i] = toupper((char)(obj->datos[i]));
		  }

		  // envia la respuesta
		  xQueueSend(obj->xQueueOA, &obj->datos, 0);


          // llamar a destructor del obj
          vTaskDelete(obj->xHandleOA);

	}

}



/*==================[internal functions definition]==========================*/


