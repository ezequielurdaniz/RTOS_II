/*============================================================================
 *
 * Autores: Luciano Perren       <lgperren@gmail.com>
 *          Juan Pablo Menditto  <jpmenditto@gmail.com>
 *          Pablo Zizzutti       <pablozizzutti@gmail.com>
 *
 * RTOS II - TP1
 * Date: 2019/09/10
 * Docentes: Franco Bucafusco y Sergio Renato De Jesús Melean
 *
 * Callbacks.c
 *
 *===========================================================================*/


/*=====[Inclusion of own header]=============================================*/

#include "Callbacks.h"

/*=====[Inclusions of private function dependencies]=========================*/
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"
#include "crc8.h"
#include "Driver.h"
#include "TimersControl.h"
#include "ProcessLetters.h"
#include "OA.h"
#include "timers.h"
#include "queue.h"
#include "stdio.h"
#include "ctype.h"
#include "string.h"

#include "semphr.h"				// Api de sincronización (sem y mutex)

// sAPI header
#include "board.h"

/*=====[Definition macros of private constants]==============================*/


/*=====[Private function-like macros]========================================*/


/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/


/*=====[Definitions of private global variables]=============================*/




/*=====[Prototypes (declarations) of private functions]======================*/

static char* MinusToMayus( char* str);		/* De minúscula a mayúscula */
static char* MayusToMinus( char* str);		/* De mayúscula a minúscula */
static char ReturnCommand( char* str);		/* Obtiene comando */
static Packagereceive( void* ptr );			/* Espera el suceso de un evento */


/*=====[Implementations of public functions]=================================*/
void ActiveObject_App( void* pvParameters )
{
	void ActiveObject_Init( Active_Object_t* obj )
	{
		obj -> xQueue = xQueueCreate( 75, sizeof( char ) );		/* Creo cola */
	}
}




void ActiveObject_App( void* param  )
{ 	/* Active Object application */

	Active_Object_t* obj = ( Active_Object_t* ) param;		/* Casting */

	Active_Object_t* obj1 = NULL;			/* Multiple instances of Active_Object_t */
	Active_Object_t* obj2 = NULL;

	while(1) {

		xQueueReceive( obj -> xQueue, ( void * ) buffer_IN ,portMAX_DELAY );
		WaitforPackage( void* ptr );
		cmd = ReturnCommand( str );


		if( cmd == UPPER ) {

		}


		/* Función que convierte de minúscula a mayúscula */
static char* MinusToMayus( char* str)
		{
			for( int i = 0; str[i] != '\0'; i++ ) {
				str[i] = toupper( ( char ) ( str[i] ) );
			}
			return str;
		}



		/* Función que convierte de mayúscula a minúscula */

static char* MayusToMinus( char* str )
{
			for( int i = 0; str[i] != '\0'; i++ ) {
				str[i] = tolower( ( char ) ( str[i] ) );
				}
				return str;
			}
		}


	/* Función que lee el carácter de comando y lo devuelve */

static char ReturnCommand( char* str)
	{
		if( str[1] == ( 'U' || 'u' ) ) {		/* Convertir a mayúsculas */
			return 'U';
		}

		if( str[1] == ( 'L' || 'l' ) ) {		/* Convertir a minúsculas */
			return 'L';
		}
		else {
			return'E';							/* ERROR */
		}
	}




	static PackageReceive( Active_Object_t* obj, char  )

					{

		xQueueReceive( obj -> xQueue, ( void * ) buffer_IN ,portMAX_DELAY );

	}
}


/*=====[Implementations of interrupt functions]==============================*/



/*==================[definiciones y macros]==================================*/



/*=====[Implementations of private functions]================================*/

