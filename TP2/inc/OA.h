/*============================================================================
 * Autores: Luciano Perren       <lgperren@gmail.com>
 *          Juan Pablo Menditto  <jpmenditto@gmail.com>
 *          Pablo Zizzutti       <pablozizzutti@gmail.com>
 * TP2 RTOS 2
 * Date: 2019/09/10
 * Docentes: Franco Bucafusco y Sergio Renato De Jesús Melean
 * ao.h
 *===========================================================================*/

#ifndef OA_H_
#define OA_H_

/*==================[inlcusiones]============================================*/

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"
#include "Callbacks.h"
#include "timers.h"
#include "queue.h"

/*==================[definiciones y macros]==================================*/

/* Active Object's attributes... */

typedef struct {
	QueueHandle_t xQueueOA;				 // Handler de la cola
	TaskHandle_t xHandleOA;				 // Handler de la tarea
	int ComandoOA;					 // Comando para la creacion del objeto activo
	char * datos;
} Active_Object_t;

Active_Object_t Instancia1;
Active_Object_t Instancia2;



/* Active_Object's operations (Active_Object's interface)... */

bool_t ActiveObject_Init( Active_Object_t* Obj );				/* Initialization */
void AO_Mayus( void* param  );
void AO_Minus( void* param  );

/* Declaracion de las Queue*/
//QueueHandle_t xQueueOA;

#endif /* OA_H_ */
