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

/*=====[Avoid multiple inclusion - begin]====================================*/
#ifndef OA_H_
#define OA_H_

/*=====[Inclusions of public function dependencies]==========================*/

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"
#include "crc8.h"
#include "Driver.h"
#include "TimersControl.h"
#include "ProcessLetters.h"
#include "timers.h"
#include "queue.h"
#include "stdio.h"
#include "ctype.h"
#include "string.h"

#include "board.h"

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/


/*=====[Public function-like macros]=========================================*/


/*=====[Definitions of public data types]====================================*/

/* Active Object's attributes... */
typedef struct {
	char buffer_IN[ 75 ];				/* Buffer de entrada */
	QueueHandle_t xQueue;				/* Handler de la cola */
	TaskHandle_t xHandle;				/* Handler de la tarea */
} Active_Object_t;

/* Active_Object's operations (Active_Object's interface)... */

void ActiveObject_App( void* pvParameters );
void ActiveObject_Init( Active_Object_t* obj );					/* Initialization */
void ActiveObject_App( void );									/* Active Object application */
void ActiveObject_Task( void* param);



/*=====[Prototypes (declarations) of public functions]=======================*/


/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/





#endif /* CALLBACKS_H_ */
