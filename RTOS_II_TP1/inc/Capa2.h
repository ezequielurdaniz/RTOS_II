/*
 * Capa2.h
 *
 *  Created on: 25 sep. 2019
 *      Author: jpm
 */

#ifndef MIS_PROGRAMAS_RTOS_II_TP1_INC_CAPA2_H_
#define MIS_PROGRAMAS_RTOS_II_TP1_INC_CAPA2_H_

/*=====[Inclusions of public function dependencies]==========================*/

#include "FreeRTOS.h"
#include "vtask.h"
#include "sapi.h"

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/

/*=====[Prototypes (declarations) of public functions]=======================*/

//void myTask( void* taskParmPtr );  // Task declaration

void Capa2( void* taskParmPtr );

void uartUsbReceiveCallback( void *unused );
void uartUsbSendCallback( void *unused );

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* MIS_PROGRAMAS_RTOS_II_TP1_INC_CAPA2_H_ */
