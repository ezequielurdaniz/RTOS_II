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
#ifndef MIS_PROGRAMAS_TP1_5_INC_DRIVER_H_
#define MIS_PROGRAMAS_TP1_5_INC_DRIVER_H_

/*=====[Inclusions of public function dependencies]==========================*/

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"
#include "crc8.h"
#include "TimersControl.h"
#include "ProcessLetters.h"
#include "Callbacks.h"
#include "timers.h"
#include "queue.h"
#include "stdio.h"
#include "ctype.h"
#include "string.h"

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/

#define TIMEOUT_VALIDATION 100 // 100 ms de timeout

#define LED_ROJO      LED2
#define LED_AMARILLO  LED1
#define LED_VERDE     LED3

#define MEMORIADINAMICA 25
#define ELEMENTOS_MEMORIA 4

/*=====[Public function-like macros]=========================================*/

struct node
{
    char datos[MEMORIADINAMICA];
    struct node *link;

}*front, *rear;


/*=====[Definitions of public data types]====================================*/


/* Declaracion de las Queue*/
QueueHandle_t xQueueEnvia;
QueueHandle_t xQueueRecibe;

char bufferin[100];  // Variable de recepcion de datos por puerto UART       // <-- File object needed for each open file

/*=====[Prototypes (declarations) of public functions]=======================*/


//void myTask( void* taskParmPtr );  // Task declaration
void Driver( void* pvParameters );

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/





#endif /* MIS_PROGRAMAS_TP1_5_INC_DRIVER_H_ */
