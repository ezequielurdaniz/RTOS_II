/*============================================================================
 * Autores: Luciano Perren       <lgperren@gmail.com>
 *          Juan Pablo Menditto  <jpmenditto@gmail.com>
 *          Pablo Zizzutti       <pablozizzutti@gmail.com>
 * TP2 RTOS 2
 * Date: 2019/09/10
 * Docentes: Franco Bucafusco y Sergio Renato De Jesús Melean
 * ao.h
 *===========================================================================*/

#ifndef RTOSII_RTOS_II_TP2_INC_OA_H_
#define RTOSII_RTOS_II_TP2_INC_OA_H_

/*==================[inlcusiones]============================================*/

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

/*==================[definiciones y macros]==================================*/

/* Active Object's attributes... */

typedef struct {
	QueueHandle_t xQueueOA;				 // Handler de la cola
	TaskHandle_t xHandleOA;				 // Handler de la tarea
	uint8_t ComandoOA;					 // Comando para la creacion del objeto activo
	char datos;
} Active_Object_t;

/* Active_Object's operations (Active_Object's interface)... */

void ActiveObject_Init( Active_Object_t* obj );					/* Initialization */
void AO_Mayus( void* param  );

/* Declaracion de las Queue*/
QueueHandle_t xQueueOA;

#endif /* RTOSII_RTOS_II_TP2_INC_OA_H_ */
