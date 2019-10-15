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
#ifndef CALLBACKS_H_
#define CALLBACKS_H_

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

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/


/*=====[Public function-like macros]=========================================*/


/*=====[Definitions of public data types]====================================*/

typedef enum{
   StandBy,
   Recibiendo,
   ErrorLimite,
} fsmUARTRXState_t;

fsmUARTRXState_t fsmUARTRXState; // Variable para guardar el valor de la maquina de estado


/*=====[Prototypes (declarations) of public functions]=======================*/

void uartUsbReceiveCallback( void *unused );
void uartUsbSendCallback( void *unused );


/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/





#endif /* CALLBACKS_H_ */
