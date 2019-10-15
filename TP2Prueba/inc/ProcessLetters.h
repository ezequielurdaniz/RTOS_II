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
 * ProcessLetters.h
 *
 *===========================================================================*/


/*=====[Avoid multiple inclusion - begin]====================================*/
#ifndef PROCESSLETTERS_H_
#define PROCESSLETTERS_H_

/*=====[Inclusions of public function dependencies]==========================*/


#include "Driver.h"
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


/*=====[Prototypes (declarations) of public functions]=======================*/


char* MinusToMayus(char *str);
bool_t CheckLettersFnc(char *str);
char* MayusToMinus( char* str );

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/





#endif /* PROCESSLETTERS_H_ */
