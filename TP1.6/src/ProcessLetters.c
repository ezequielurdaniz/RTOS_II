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

/*=====[Inclusion of own header]=============================================*/

#include "ProcessLetters.h"

/*=====[Inclusions of private function dependencies]=========================*/

#include "Driver.h"
#include "stdio.h"
#include "ctype.h"
#include "string.h"

/*=====[Definition macros of private constants]==============================*/


/*=====[Private function-like macros]========================================*/


/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/




/*=====[Prototypes (declarations) of private functions]======================*/




/*=====[Implementations of public functions]=================================*/

bool_t CheckLettersFnc(char *str){

	 for (int i = 0; str[i] != '\0'; i++){
		 if(!isalpha(str[i])){ // verifica si el caracter es alfabetico
			 return false;  // devuelve error si encuentra un caracter no alfabetico
		 }
	 }
	 return true; // Si llega hasta aca, recorrio toda la cadena y encontro caracteres alfabeticos
}



char* MinusToMayus(char *str){

	 for (int i = 0; str[i] != '\0'; i++){
		 str[i] = toupper((char)(str[i]));
	 }
	return str;
}



/*=====[Implementations of interrupt functions]==============================*/




/*=====[Implementations of private functions]================================*/


