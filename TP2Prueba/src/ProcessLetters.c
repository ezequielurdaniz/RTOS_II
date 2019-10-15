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
 * ProcessLetters.c
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

/*!
 * @brief  Verifica que el string sea solo letras
 *
 * @param[in] char *
 *
 * @return  bool_t
 */

bool_t CheckLettersFnc(char *str){

	 for (int i = 0; str[i] != '\0'; i++){
		 if(!isalpha(str[i])){ // verifica si el caracter es alfabetico
			 return false;  // devuelve error si encuentra un caracter no alfabetico
		 }
	 }
	 return true; // Si llega hasta aca, recorrio toda la cadena y encontro caracteres alfabeticos
}

/*!
 * @brief  Convierte el string de minusculas a mayusculas
 *
 * @param[in] char *
 *
 * @return  char *
 */

char* MinusToMayus(char *str){

	 for (int i = 0; str[i] != '\0'; i++){
		 str[i] = toupper((char)(str[i]));
	 }
	return str;
}

/*!
 * @brief  Convierte el string de mayusculas a minusculas
 *
 * @param[in] char *
 *
 * @return  char *
 */

char* MayusToMinus( char* str ) {

	for( int i = 0; str[i] != '\0'; i++ ) {
		str[i] = tolower( ( char ) ( str[i] ) );
		}
	return str;

}



/*=====[Implementations of interrupt functions]==============================*/




/*=====[Implementations of private functions]================================*/


