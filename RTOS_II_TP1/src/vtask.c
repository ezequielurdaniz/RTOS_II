/*
 * vtask.c
 *
 *  Created on: 25 sep. 2019
 *      Author: jpm
 */


/*=============================================================================
 * Copyright (c) 2019, Eric Pernia <ericpernia@gmail.com>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2019/08/23
 *===========================================================================*/

/*=====[Inclusion of own header]=============================================*/

#include "vTasks.h"

/*=====[Inclusions of private function dependencies]=========================*/
#include "Capa2.h"
#include "sapi.h"       // <= sAPI header

/*=====[Definition macros of private constants]==============================*/
#define UART_PC        UART_USB

/*=====[Private function-like macros]========================================*/

/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

static char buf[100];


/*=====[Prototypes (declarations) of private functions]======================*/

/*=====[Implementations of public functions]=================================*/

// Task implementation


// Task implementation
void vtask( void* taskParmPtr )
{
   // ----- Task setup -----------------------------------


   // Periodic task every 10 ms
   portTickType xPeriodicity =  10 / portTICK_RATE_MS;
   portTickType xLastWakeTime = xTaskGetTickCount();

   // ----- Task repeat for ever -------------------------
   while(TRUE) {
      disk_timerproc();   // Disk timer process

      // Send the task to the locked state during xPeriodicity
      // (periodical delay)
      vTaskDelayUntil( &xLastWakeTime, xPeriodicity );
   }
}



/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/



