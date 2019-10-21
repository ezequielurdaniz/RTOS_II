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
#ifndef DRIVER_H_
#define DRIVER_H_

/*=====[Inclusions of public function dependencies]==========================*/

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"
#include "crc8.h"
#include "TimersControl.h"
#include "Callbacks.h"
#include "timers.h"
#include "queue.h"
#include "stdio.h"
#include "ctype.h"
#include "string.h"
#include "OA.h"

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/

#define UART_PC        UART_USB
#define UART_232       UART_232

#define TIMEOUT_VALIDATION 50 // 50 ms de timeout

#define LED_ROJO      LED2
#define LED_AMARILLO  LED1
#define LED_VERDE     LED3

#define MEMORIADINAMICA 75

#define ELEMENTOS_MEMORIA 4

/*=====[Public function-like macros]=========================================*/

struct node
{
    char datos[MEMORIADINAMICA];
    struct node *link;

}*front, *rear;


struct node *tempInstMayus;
struct node *tempInstMinus;

/*=====[Definitions of public data types]====================================*/


/* Declaracion de las Queue*/
QueueHandle_t xQueueEnvia;
QueueHandle_t xQueueRecibe;

char memDinIn[MEMORIADINAMICA];  // Variable de recepcion de datos por puerto UART       // <-- File object needed for each open file

/*=====[Prototypes (declarations) of public functions]=======================*/


void Driver( void* pvParameters );
void Demonio( void* pvParameters );
void uartDriverInit(uartMap_t uart);
int TamanioCola();
void EnvioMensajeUART(char * str, int indice);
uint8_t ObtenerComando();
uint8_t ObtenerCantidadCaracteres();

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/





#endif /* DRIVER_H_*/
