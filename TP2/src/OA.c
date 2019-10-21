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
bool_t CorriendoMinus = false;
bool_t CorriendoMayus = false;
/*====================[definición de datos internos]=========================*/

/*==================[definiciones y macros]==================================*/

/*==================[declaraciones de funciones externas]====================*/

/*==================[Active Object Initialization]======================================*/

bool_t ActiveObject_Init( Active_Object_t* Obj ) {

	BaseType_t xReturned;

	// Si se solicita crear un objeto activo para mayusculizar
	 if(Obj->ComandoOA==1){

		 if(CorriendoMayus == false){

	  /* Crear tarea en freeRTOS */
	  xReturned = xTaskCreate(
		  AO_Mayus,
	      (const char *)"AO_Mayusculizar",
	      configMINIMAL_STACK_SIZE*2,
		  Obj,
	      tskIDLE_PRIORITY+1,
	      &Obj->xHandleOA
	      );

		  // Si la tarea se creo correctamente
		  if(Obj->xHandleOA!=NULL){

		  }

		  /* Inicializo elementos de la estructura */
		  Obj->xQueueOA = xQueueCreate(1, sizeof( char  *));	/* Creo cola */

		  // Error en la creacion de la Queue
		  if( Obj->xQueueOA == NULL ){
		  	 /* Error fatal */
		  	 // Indica una falla en el sistema
		  }

		  CorriendoMayus=true;

		  return true;
	  }
		 else
		 {
			 return false;
		 }
    }
	 // Si se solicita crear un objeto activo para minusculizar
	if(Obj->ComandoOA==2){

	 	  if(CorriendoMinus == false){

	 	  /* Crear tarea en freeRTOS */
	 	  xReturned = xTaskCreate(
	 		  AO_Minus,
	 	      (const char *)"AO_Mayusculizar",
	 	      configMINIMAL_STACK_SIZE,
	 		  Obj,
	 	      tskIDLE_PRIORITY+1,
	 	      &Obj->xHandleOA
	 	      );

	 		  // Si la tarea se creo correctamente
	 		  if(Obj->xHandleOA!=NULL){

	 		  }

	 		  /* Inicializo elementos de la estructura */
	 		  Obj->xQueueOA = xQueueCreate(1, sizeof( char  *));	/* Creo cola */

	 		  // Error en la creacion de la Queue
	 		  if( Obj->xQueueOA == NULL ){
	 		  	 /* Error fatal */
	 		  	 // Indica una falla en el sistema
	 		  }

	 		 CorriendoMinus=true;
	 		  return true;
	 	  }
	 	  else
	 	  {
	 		  return false;
	 	  }
	 }
}

/*
 *
 *  Task para mayusculizar
 *
 */

void AO_Mayus( void* param  ) { 	                        /* Active Object application */

	Active_Object_t* obj = ( Active_Object_t* ) param;		/* Casting */
	BaseType_t xStatusTX;    // Variable de status de la queue

	while(1) {

		 for (int i = 0; obj->datos[i] != '\0'; i++){
		      obj->datos[i] = toupper((char)(obj->datos[i]));
		 }

		 xStatusTX=xQueueSend(obj->xQueueOA, &obj->datos, 0);
		 CorriendoMayus=false;
         // llamar a destructor del obj
         vTaskDelete(obj->xHandleOA);

	}

}

/*
 *
 *  Task para mayusculizar
 *
 */

void AO_Minus( void* param  ) { 	                        /* Active Object application */

	Active_Object_t* obj = ( Active_Object_t* ) param;		/* Casting */
	BaseType_t xStatusTX;    // Variable de status de la queue

	while(1) {

		 for (int i = 0; obj->datos[i] != '\0'; i++){
		      obj->datos[i] = tolower((char)(obj->datos[i]));
		 }

		 xStatusTX=xQueueSend(obj->xQueueOA, &obj->datos, 0);
		 CorriendoMinus=false;
         // llamar a destructor del obj
         vTaskDelete(obj->xHandleOA);

	}

}



/*==================[internal functions definition]==========================*/


