/*=============================================================================
 * Copyright (c) 2019, Eric Pernia <ericpernia@gmail.com>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2019/08/23
 *===========================================================================*/

/*=====[Inclusion of own header]=============================================*/

#include "capa3.h"
 
/*=====[Inclusions of private function dependencies]=========================*/

#include "sapi.h"       // <= sAPI header

#include <stdio.h>
#include <ctype.h>

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
void validar( void* taskParmPtr )
{
   // ----- Task setup -----------------------------------
   // Periodic task every 500 ms
   portTickType xPeriodicity =  500 / portTICK_RATE_MS;
   portTickType xLastWakeTime = xTaskGetTickCount();
   bool_t SonLetras(const char *);
   bool_t mayus (const char *);
   bool_t retorno;
   bool_t con;
   char str[] = "tAASSSADDSE";  // ACA EL PAQUETE QUE ENTRA POR COLA


   // ----- Task repeat for ever -------------------------
   while(TRUE) {

	    retorno = SonLetras(str);
	    if (retorno == 1)
	    {
	 //   printf("verdadero\n");
	     con = mayus(str);
	    }
	    else
	 //   printf("falso\n");// O MENSAJE DE ERROR POR COLA

      vTaskDelayUntil( &xLastWakeTime, xPeriodicity );
   }
}


bool_t mayus (const char *men)
{
   char conv[10];
   while(TRUE)
   {
	for (uint8_t indice = 0; men[indice] != '\0'; ++indice)
	{
		conv[indice] = toupper(men[indice]);
	}
	return 0;
}
}





bool_t SonLetras(const char *str)
	   {
	      char abc[] =
	      {
	         "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	         "abcdefghijklmnopqrstuvwxyz"
	      };

	      size_t num_chars = sizeof abc - 1;
	      size_t i;
	      size_t j;

	      for (i = 0; str[i] != '\0'; i++){
	         abc[num_chars] = str[i];

	         for (j = 0; abc[j] != str[i]; j++)
	            ;
	         if (j == num_chars)

	            return 0;
	      }
	      return 1;
	   }













/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/

