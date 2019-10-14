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

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"
#include "crc8.h"
#include "timers.h"
#include "queue.h"
#include "stdio.h"
#include "ctype.h"
#include "string.h"

// definicion del tiempo de espera antes de procesar un dato entrante o saliente
#define TIMEOUT_VALIDATION 100 // 100 ms de timeout

// defines de los leds para debug
#define LED_ROJO      LED2
#define LED_AMARILLO  LED1
#define LED_VERDE     LED3

// limite de datos entrantes por UART, al excederlos indicará un error
#define MEMORIADINAMICA 75

// Tamanio máximo de la Cola de Memoria dinamica
#define ELEMENTOS_MEMORIA 4

void BorrarBufferIn();
void EnvioErrorUart();
void EnvioErrorUartLim();
void EnvioMensajeUART(char * str, int indice);

bool_t CheckLettersFnc(char *str);
char* MinusToMayus(char *str);
void EliminaBloqueMemoriaDinamica();
bool_t VerificaColaLlena();
void uartDriverInit(uartMap_t uart);

void HeartbeatCallback(TimerHandle_t xTimer);
void TimeoutCallback(TimerHandle_t xTimer);
void TimeToExitCallback(TimerHandle_t xTimer);

void uartUsbReceiveCallback( void *unused );
void uartUsbSendCallback( void *unused );
void Driver( void* pvParameters );

/* Declaracion de los Timers */
TimerHandle_t TimerTimeout;
TimerHandle_t TimerHeartbeat;
TimerHandle_t TimeToExit;

/* Declaracion de las Queue*/
QueueHandle_t xQueueEnvia;
QueueHandle_t xQueueRecibe;
QueueHandle_t xUartReceive;

// Buffer de recepcion de datos por puerto UART
char bufferin[100];

/*-----------------------------------------------------------------------------
 *
 *  Declaraciones FSM
 *
 * ---------------------------------------------------------------------------*/

// Nombres de FSM recepcion UART
typedef enum{
   StandBy,
   Recibiendo,
   ErrorLimite,
} fsmUARTRXState_t;

fsmUARTRXState_t fsmUARTRXState; // Variable para guardar el valor de la maquina de estado

// Declaracion de structura utilizada en la cola de memorias dinamica
struct node
{
    char datos[MEMORIADINAMICA];
    struct node *link;

}*front, *rear;



/*!
 * @brief  Callback del Timer Hearbeat del sistema
 *
 * @param[in] TimerHandle_t
 *
 * @return  void
 */

void HeartbeatCallback(TimerHandle_t xTimer){

	gpioToggle(LEDB);  // ¡¡it's alive!!

}

/*!
 * @brief  Callback del Timer para la salida de datos
 *
 * @param[in] TimerHandle_t
 *
 * @return  void
 */

void TimeToExitCallback(TimerHandle_t xTimer){

	if(xTimer == TimeToExit){

	    if( xTimerStop( TimeToExit, 0) != pdPASS )
	    {
		  		/* Fallo el comando de detencion del timer. Enciende el led de ERROR */
	    }
	    // Limpia la memoria dinamica que no se usará más y habilita el callbac de salida
	    gpioToggle(LED3);
	}
}

/*!
 * @brief  Callback del Timer para la llegada de datos
 *
 * @param[in] TimerHandle_t
 *
 * @return  void
 */

void TimeoutCallback(TimerHandle_t xTimer){

  bool_t ColaLlena;

  struct node *temp;

  if(xTimer == TimerTimeout){

	 if( xTimerStop( TimerTimeout, 0) != pdPASS )
	 {
		/* Fallo el comando de detencion del timer. Enciende el led de ERROR */
		 gpioToggle(LED2);
	 }

	 // Si se produjo un error por exceder el limite de caracteres de recepcion
	 if(fsmUARTRXState==ErrorLimite){

		 EnvioErrorUartLim();
		 // activa el callback de TX UART
		 BorrarBufferIn();
		 fsmUARTRXState = StandBy;
		 return;// Sale del callback del timer
     }

	 // Verifica si la cola esta llena, si es asi, avisa por callback de salida que no puede recibir mas datos
	 // de lo contrario tratara de procesar el paquete entrante
	 fsmUARTRXState = StandBy;

	 // Verifica si la cola esta llena o tiene espacio
	 ColaLlena=VerificaColaLlena();

	 // Si la cola esta llena, avisa al usuario utilizando un mensaje de error
     if(ColaLlena==TRUE){

       EnvioErrorUartLim();
       // activa el callback de TX UART
	   BorrarBufferIn();                                 // clear the array bufferin
       return;// Sale del callback del timer
     }

     //--------------------------------------------------------------------------------------------------------
     // en el caso que haya memoria en el sistema, crea un nuevo bloque de memoria en la cola de memorias dinamicas
     // y acomoda los punteros de la cola dinamica de memorias
     //

     // Creo el bloque de memoria dinamica
	 temp = (struct node*)pvPortMalloc(sizeof(struct node));
	 // Copia el contenido del buffer entrante de datos en el dato que ira al frente
	 for(int i = 0 ; i < strlen(bufferin); i++){
		 temp->datos[i] = bufferin[i]; // copia el contenido del buffer entrante en ultimo elemento de la cola dinamica
	 }

	 BorrarBufferIn();

	 // el bloque creado, sera el ultimo de la cola, entonces apuntara a NULL
	 temp->link = NULL;

	 // Si el primer elemento es NULL, significa que la cola esta vacia, entonces el elemento que creo
	 // sera el primero y el ultimo (FRONT & REAR)
	 // en caso que no sea el primero, lo acomodará al final de la cola dinamica de memoria

	 if(front == NULL){
	  	 front = rear = temp;
	 }
	 else
	 {
	  	 rear->link = temp;
	  	 rear = temp;
	 }

  }
}

/*!
 * @brief  Borra el contenido del bufferin
 *
 * @param[in] void
 *
 * @return  void
 */

void BorrarBufferIn(){
	memset(&bufferin[0], 0, sizeof(bufferin));
}

/*!
 * @brief  Envia por UART el mensaje de ERROR por limite de caracteres o cola llena
 *
 * @param[in] void
 *
 * @return  void
 */

void EnvioErrorUartLim(){

	BaseType_t xStatusTX;    // Variable de status de la queue
	char Error[] = "ERROR "; // Mensaje de error para el envio por la queue
	char caracter_out;

	Error[5] = '\0';
	for(int j = 0 ; j < 6; j++){
	   	  caracter_out = Error[j];
	   	  xStatusTX = xQueueSend( xQueueRecibe, &caracter_out, 0 );
	}
	// Inicia el timer de salida
	xTimerStart( TimeToExit , 0 );
	uartCallbackSet(UART_USB, UART_TRANSMITER_FREE, uartUsbSendCallback, NULL);
}

/*!
 * @brief  Envia por UART el mensaje de ERROR
 *
 * @param[in] void
 *
 * @return  void
 */

void EnvioErrorUart(){

	BaseType_t xStatusTX;    // Variable de status de la queue
	char Error[] = "ERROR "; // Mensaje de error para el envio por la queue
	char caracter_out;

	Error[5] = '\0';
	for(int j = 0 ; j < 6; j++){
	   	  caracter_out = Error[j];
	   	  xStatusTX = xQueueSend( xQueueRecibe, &caracter_out, 0 );
	}
	// Inicia el timer de salida
	xTimerStart( TimeToExit , 0 );
	EliminaBloqueMemoriaDinamica();
}

/*!
 * @brief  Envia por UART el mensaje str
 *
 * @param[in] char * str
 * @param[in] int indice
 *
 * @return  void
 */

void EnvioMensajeUART(char * str, int indice){

	BaseType_t xStatusTX;  // Variable de status de la queue
	char caracter_out;

	for(int i = 0 ; i < indice+1 ; i++){
	   caracter_out = str[i];
	   xStatusTX = xQueueSend( xQueueRecibe, &caracter_out, 0 ); // envio datos por Queue
	}

	// Inicia el timer de salida
	xTimerStart( TimeToExit , 0 );
	EliminaBloqueMemoriaDinamica();

}

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
 * @brief  Elimina el bloque de memoria ya utilizado, limpia los buffers acomoda los punteros de la cola
 * 		   de memoria dinamica y activa el callback de salida de UART para el envio de los datos
 *
 * @param[in] void
 *
 * @return  void
 */

void EliminaBloqueMemoriaDinamica(){

	struct node *temp;

	// borro la memoria dinamica
	// Problema resuelto : se borro el bloque de memoria para no dejar basura en la proxima
	// transaccion
	uartCallbackSet(UART_USB, UART_TRANSMITER_FREE, uartUsbSendCallback, NULL);
	// Borra el sector de memoria dinamica para que en la proxima creacion no modifique el
	// proximo bloque creado
	memset(&front->datos[0], 0, sizeof(front->datos));

	// Mueve los punteros para liberar ese bloque dinamico
	temp = front;
	front = front->link;
	vPortFree(temp);
}

/*!
 * @brief  Verifica si la cola de memorias dinamicas esta llena
 *
 * @param[in] void
 *
 * @return  bool_t
 */

bool_t VerificaColaLlena(){

	 struct node *temporal;
	 uint8_t cnt;

	 // Verificar el tamaño de la cola que no sea mas grande que el maximo permitido : ELEMENTOS_MEMORIA
	 temporal = front; // guarda el primer elemento de la cola en un temporal

	 cnt = 0;  // inicializa la variable para el recorrido

	 // va recorriendo toda la cola desde el primer elemento
	 while (temporal){
		temporal = temporal->link;
		cnt++;
	 }

	 // si la cantidad de elementos de la cola es igual al limite maximo, avisa del error
	 // sino devuelve la cantidad
	 if(cnt == ELEMENTOS_MEMORIA){
         return TRUE;
	  }

	  return FALSE;
}

/*!
 * @brief  Calcula el tamaño de la cola de memoria dinamica y verifica si hay datos pendientes en la misma
 *
 * @param[in] void
 *
 * @return  uint8_t
 */

int TamanioCola(){

	struct node *temporal;   // estructura temporal para el calculo del tamanio de la cola
	int cnt;             // Variabe que contiene el tamaño de la cola

		 // Verificar el tamaño de la cola que no sea mas grande que el maximo permitido : ELEMENTOS_MEMORIA
		 temporal = front; // guarda el primer elemento de la cola en un temporal

		 cnt = 0;  // inicializa la variable para el recorrido

		 // va recorriendo toda la cola desde el primer elemento
		 while (temporal){
			temporal = temporal->link;
			cnt++;
		 }

	return cnt;   // devuelve la cantidad de elementos presentes en la cola dinamica

}

/*!
 * @brief  Callback de la recepcion UART
 *
 * @param[in] unused
 *
 * @return  void
 */

void uartUsbReceiveCallback( void *unused )
{
	static uint8_t indicerx;
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	char Caracter;

	// FSM Recepcion de Uart
	switch( fsmUARTRXState ){

	         case StandBy:

	        	 indicerx=0;
	        	 // Recibimos el primer elemento por UART
	        	 bufferin[indicerx] = uartRxRead(UART_USB);
	        	 //uartTxWrite(UART_USB, bufferin[indicerx] );
	             xTimerStartFromISR( TimerTimeout , &xHigherPriorityTaskWoken );
	             fsmUARTRXState = Recibiendo;

	         break;

	         case Recibiendo:
	        	 // Compruebo que no excede el limite de la memoria dinamica
	        	 if(indicerx > (MEMORIADINAMICA-1)){
	        		fsmUARTRXState = ErrorLimite;
	        	 }
	        	 else
	        	 {
	        		 xTimerResetFromISR( TimerTimeout , &xHigherPriorityTaskWoken );
	        		 indicerx++;
	        		 bufferin[indicerx] = uartRxRead(UART_USB);
	        		 //uartTxWrite(UART_USB, bufferin[indicerx] );
	        	 }
	         break;

	         case ErrorLimite:
	        	 xTimerResetFromISR( TimerTimeout , &xHigherPriorityTaskWoken );
	        	 indicerx=0; // inicializa con el primer caracter
	        	 Caracter = uartRxRead(UART_USB); // el excedende de datos lo guarda aca
	         break;
 	}
}

/*!
 * @brief  Callback de la transmision UART
 *
 * @param[in] unused
 *
 * @return  void
 */

void uartUsbSendCallback( void *unused )
{
	char caracter_in;
	static char lReceivedValue[MEMORIADINAMICA];
	static uint8_t indice = 0;
	BaseType_t xTaskWokenByReceive = pdFALSE;
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;

	 // Recepcion de los datos de la queue
	 /* Se recibe un caracter y se almacena */
	 if(xQueueReceiveFromISR( xQueueRecibe, &caracter_in, &xTaskWokenByReceive ) == pdTRUE){
		 xTimerResetFromISR( TimeToExit , &xHigherPriorityTaskWoken );
		 //gpioToggle(LED1);
		 /* Se recibe un caracter y se almacena */
		 lReceivedValue[indice] = caracter_in;

		  if(lReceivedValue[indice] != '\0'){
		     /* Final del mensaje */
			 uartWriteByte(UART_USB, caracter_in );
			 indice++;
		  }
		  else
		  {
			 indice = 0;
			 memset(&lReceivedValue[0], 0, sizeof(lReceivedValue)); // clear the array
			 uartWriteByte(UART_USB, '\r' );
			 uartWriteByte(UART_USB, '\n' );
			 uartCallbackClr( UART_USB, UART_TRANSMITER_FREE);
		  }


	 }
}

/*!
 * @brief  Tarea principal del Driver
 *
 * @param[in] pvParameters
 *
 * @return  void
 */

void Driver( void* pvParameters )
{
	/* Configuracion inicial de la tarea */
	char* lValueToSend;	 // el valor a enviar por la cola

	char lReceivedValue[MEMORIADINAMICA];

	uint8_t indice = 0;

	BaseType_t xStatusRX;  // Variable de status de la queue

	const TickType_t xTicksToWait = 0;
	bool_t EstadoPaquete;

	char caracter_in;
	int TamCola;

	uint8_t crc_temp_rx;
	uint8_t crc_temp_tx;

	struct node *temp;

    /* loop infinito de la tarea */
    for( ;; ) {

       TamCola=TamanioCola();

       // Lee la cantidad de datos que hay en la cola dinamica de datos, para procesarlos o quedarse aqui
       while(TamCola==0){
    	  TamCola=TamanioCola();
       }

       // Guardo en indice la cantidad de datos que recibi por el puerto serie
       indice = strlen(front->datos);

       // realizo el calculo del CRC para verificar el dato entrante
       crc_temp_rx = crc8_calc(0 , front->datos , indice-1);

   	   // chequeo que el CRC8 calculado sea igual al CRC8 entrante con los datos por el puerto serie
   	   if(crc_temp_rx == front->datos[indice-1]){

   	 	   /* Llego un paquete bueno, paso a la siguiente capa */
   		   // Sobreescribo el caracter del CRC8 por el \0
   		   front->datos[indice -1] = '\0';

   		   // Chequea el paquete entrante para verificar si son letras
   		   EstadoPaquete = CheckLettersFnc(front->datos);

   		   // Si el paquete esta ok, tiene todas letras, lo convierte a mayusculas y lo envia por la queue
   		   // sino devuelve error por la queue
   		   if(EstadoPaquete == true ){
   			    lValueToSend = MinusToMayus(front->datos); // Se envia el mensaje convertido

   				// calcula el CRC8 de salida y lo suma al string
   				// Al dato recibido, se le agrega el crc8 y se reenvia por uart
   				lValueToSend = CalculaCRC8(lValueToSend);

   				EnvioMensajeUART(lValueToSend, indice);

   				indice = 0;
   				memset(&lValueToSend[0], 0, sizeof(lValueToSend));                      // clear the array

   		   }
   		   else
   		   {
   			    // envio un error por transmision de uart, ya que el paquete contiene otros caracteres, no letras
   				EnvioErrorUart();
   		   }

   	    }
   	    else
   	    {
   	       // Llego un paquete con mal CRC8, devuelvo error por el puerto
   	    	EnvioErrorUart();
   	    }

   	//EliminaBloqueMemoriaDinamica();

    }
}


/*!
 * @brief  Inicializacion del puerto UART indicado
 *
 * @param[in] uart
 *
 * @return  NULL
 */

void uartDriverInit(uartMap_t uart) {

	/* Inicializar la UART_USB junto con las interrupciones de Tx y Rx */
	uartConfig(uart, 115200);
	// Seteo un callback al evento de recepcion y habilito su interrupcion
	uartCallbackSet(uart, UART_RECEIVE, uartUsbReceiveCallback, NULL);
	// Seteo un callback al evento de transmisor libre y habilito su interrupcion
	uartCallbackSet(uart, UART_TRANSMITER_FREE, uartUsbSendCallback, NULL);
	// Habilito todas las interrupciones de UART_USB
	uartInterrupt(uart, true);
	// Clear el callback para transmision del UART
	uartCallbackClr(uart, UART_TRANSMITER_FREE);
	// Mensaje de inicio
	uartWriteString(uart, "Iniciando...\r\n");

}

/*!
 * @brief  Funcion main
 *
 * @param[in] void
 *
 * @return  int
 */

int main(void)
{
   /* Inicializar la placa */
   boardConfig();

   uartDriverInit(UART_USB);

   gpioInit( LED_ROJO, GPIO_OUTPUT );
   gpioInit( LED_AMARILLO, GPIO_OUTPUT );
   gpioInit( LED_VERDE, GPIO_OUTPUT );

   xTaskCreate(
	  Driver,                  // Funcion de la tarea a ejecutar
      (const char *)"Driver",    // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+1,         // Prioridad de la tarea
      0                           // Puntero a la tarea creada en el sistema
   );

   // Creacion del timer "Timeout"
   TimerTimeout = xTimerCreate ("Timeout in", TIMEOUT_VALIDATION / portTICK_RATE_MS , pdFALSE, (void *) 0 , TimeoutCallback);

   // Creacion del timer "TimeToExit"
   TimeToExit = xTimerCreate ("Time To Exit", TIMEOUT_VALIDATION / portTICK_RATE_MS , pdFALSE, (void *) 0 , TimeToExitCallback);

   // Creacion del timer "heartbeat"
   TimerHeartbeat = xTimerCreate ("Timer Heartbeat", 500 / portTICK_RATE_MS , pdTRUE, (void *) 0 , HeartbeatCallback);
   xTimerStart( TimerHeartbeat , 0 );

   // Creacion de las colas
   xQueueEnvia = xQueueCreate( 100,  sizeof( char ) );

   // Error en la creacion de la Queue
   if( xQueueEnvia == NULL ){
     /* Error fatal */
     while(1){
    	 gpioWrite(LEDR, ON);  // Indica una falla en el sistema
	 }
   }

   xQueueRecibe = xQueueCreate( 100,  sizeof( char ) );

   // Error en la creacion de la Queue
   if( xQueueRecibe == NULL )
   {
	 /* Error fatal */
	 while(1){
		 gpioWrite(LEDR, ON);   // Indica una falla en el sistema
	 }
   }

   xUartReceive = xQueueCreate( 100,  sizeof( char ) );

   // Error en la creacion de la Queue
   if( xUartReceive == NULL )
   {
   	 /* Error fatal */
   	 while(1){
   		 gpioWrite(LEDR, ON);   // Indica una falla en el sistema
     }
   }

   /*
    *  Inicializacion de variables del sistema
    */

   fsmUARTRXState = StandBy;    // inicia la maquina de estados con la recepcion en standby

   /*
    *  Inicializacion del scheduler
    */

   vTaskStartScheduler();

   while(true){
      // do nothing and more nothing
   }

   return 0;
}



