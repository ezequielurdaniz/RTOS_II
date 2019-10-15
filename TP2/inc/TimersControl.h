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
#ifndef TIMERSCONTROL_H_
#define TIMERSCONTROL_H_

/*=====[Inclusions of public function dependencies]==========================*/

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"
#include "crc8.h"
#include "Driver.h"
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



/*=====[Definitions of public data types]====================================*/

/* Declaracion de los Timers */
TimerHandle_t TimerTimeout;
TimerHandle_t TimerHeartbeat;
TimerHandle_t TimeToExit;


/*=====[Prototypes (declarations) of public functions]=======================*/

//void myTask( void* taskParmPtr );  // Task declaration


void HeartbeatCallback(TimerHandle_t xTimer);
void TimeToExitCallback(TimerHandle_t xTimer);
void EliminaBloqueMemoriaDinamica();
void TimeoutCallback(TimerHandle_t xTimer);
bool_t VerificaColaLlena();
void BorrarBufferIn();
void EnvioErrorUartLim();
void EnvioErrorUart();



/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/





#endif /* TIMERSCONTROL_H_ */
