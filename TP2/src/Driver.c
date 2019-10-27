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
 * Driver.c
 *
 *===========================================================================*/


/*=====[Inclusion of own header]=============================================*/

#include "Driver.h"

/*=====[Inclusions of private function dependencies]=========================*/
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"
#include "TimersControl.h"
#include "timers.h"
#include "queue.h"
#include "stdio.h"
#include "ctype.h"
#include "string.h"
#include "OA.h"

/*=====[Definition macros of private constants]==============================*/


/*=====[Private function-like macros]========================================*/

/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Prototypes (declarations) of private functions]======================*/


/*=====[Implementations of public functions]=================================*/

/*!
 * @brief  Espera que llegue un paquete a la cola
 *
 * @param[in] void
 *
 * @return  void
 */

void esperar_paquete(){

	int TamCola=0;

	TamCola=TamanioCola();

	 // Lee la cantidad de datos que hay en la cola dinamica de datos, para procesarlos o quedarse aqui
	 while(TamCola==0){
	    TamCola=TamanioCola();
	 }

	 // Probar si funciona con
	 //while(front->datos == NULL);

}

/*!
 * @brief  Obtiene el Comando de la cola dinamica
 *
 * @param[in] struct node *
 *
 * @return  uint8_t
 */

uint8_t ObtenerComando(struct node *temp){

	// Variable local para proceso del comando
	uint8_t ComandoDinamic;

	// El primer elemento de la cola dinamica es el comando
	ComandoDinamic=temp->datos[0];

    // Acomoda los datos para seleccionar solo los mismos y pisar el comando en la cola de memoria dinamica
    for(int i = 0 ; i < strlen(temp->datos); i++){
    	temp->datos[i] = temp->datos[i+1];
    }

    // devuelve el comando OA
	return ComandoDinamic;
}

/*!
 * @brief  Obtiene la cantidad de elementos en array de memoria dinamica
 *
 * @param[in] void
 *
 * @return  char
 */

uint8_t ObtenerCantidadCaracteres(){

	// Guardo en indice la cantidad de datos que recibi por el puerto serie
      uint8_t indice;

      indice = strlen(front->datos);
      indice--;

      return indice;
}


/*!
 * @brief  Tarea Demonio del Driver
 *
 * @param[in] pvParameters
 *
 * @return  void
 */

void Demonio( void* pvParameters )
{

  char* lValueToSend1;	 // el valor a enviar por la cola
  char* lValueToSend2;   // el valor a enviar por la cola

  while(1){

     // Verifica si llegan datos por las Queue desde el Objeto Activo, tanto de mayusculizar como de
	 // minusculizar
	 if( Instancia1.xQueueOA != 0 )
	 {
		 if(xQueueReceive(Instancia1.xQueueOA, &lValueToSend1, 0)){

			 // calcula el CRC8 de salida y lo suma al string
			 // Al dato recibido, se le agrega el crc8 y se reenvia por uart
			 lValueToSend1 = CalculaCRC8(lValueToSend1);

			 EnvioMensajeUART(lValueToSend1, strlen(lValueToSend1));

			 memset(&lValueToSend1[0], 0, sizeof(lValueToSend1));                      // clear the array
			 Instancia1.xQueueOA=0;
	     }
	 }

	 if( Instancia2.xQueueOA != 0 )
	 {
	 	if(xQueueReceive(Instancia2.xQueueOA, &lValueToSend2, 0)){

	 		// calcula el CRC8 de salida y lo suma al string
	 		// Al dato recibido, se le agrega el crc8 y se reenvia por uart
	 		lValueToSend2 = CalculaCRC8(lValueToSend2);

	 		EnvioMensajeUART(lValueToSend2, strlen(lValueToSend2));

	 		memset(&lValueToSend2[0], 0, sizeof(lValueToSend2));                      // clear the array
	 		Instancia2.xQueueOA=0;
	 	 }
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

	char lReceivedValue[MEMORIADINAMICA];

	uint8_t indice = 0;

	BaseType_t xStatusRX;  // Variable de status de la queue
	BaseType_t xStatusTX;  // Variable de status de la queue

	const TickType_t xTicksToWait = 0;
	bool_t EstadoPaquete;

	char Error[] = "ERROR "; // Mensaje de error para el envio por la queue

	uint8_t ComandoDin;

	uint8_t crc_temp_rx;

	char caracter_out;

	int TamCola=0;

	bool_t resMayus;
	bool_t resMinus;

	tempInstMayus=NULL;
	tempInstMinus=NULL;

    for( ;; ){

       // Espera que se agregue un paquete en la cola dinamimca de memoria
       while(front==NULL);

       // Guardo en indice la cantidad de datos que recibi por el puerto serie
       indice=ObtenerCantidadCaracteres();

       // Calcular CRC
       // realizo el calculo del CRC para verificar el dato entrante
       crc_temp_rx = crc8_calc(0 , front->datos , indice-1);

   	   // chequeo que el CRC8 calculado sea igual al CRC8 entrante con los datos por el puerto serie
   	   if(crc_temp_rx == front->datos[indice-1]){

   		   // Extraigo el Comando
   		   // El primer elemento de la cola dinamica es el comando

   		   ComandoDin=ObtenerComando(front);

   		   indice--; // acomodo el indice eliminando el CRC de la cantidad de datos de la cola dinamica

   	 	   /* Llego un paquete bueno, paso a la siguiente capa */
   		   // Sobreescribo el caracter del CRC8 por el \0
   		   front->datos[indice -1] = '\0';

   		   // Chequea el paquete entrante para verificar si son letras
   		   EstadoPaquete = CheckLettersFnc(front->datos);

   		   // Aca deberia copiar el puntero de la memoria dinamica a una cola para ser procesada
   		   // por la OA
   		   // Si el paquete esta ok, tiene todas letras, lo convierte a mayusculas y lo envia por la queue
   		   // sino devuelve error por la queue

   		   if(EstadoPaquete == true ){

             // Comprueba si el comando es para mayusculizar
             if(ComandoDin=='1'){

   				  // Verifica que la instancia 1 es NULL para poder crear una nueva
   				 Instancia1.ComandoOA=1;
   				 Instancia1.datos = front->datos;
                  // remover el dato de la cola de memoria
   				 resMayus=ActiveObject_Init(&Instancia1);

   				 if(resMayus==false){
   					// guarda ese dato en la cola de instancias, en el caso que este ocupada esa tarea
   					// CreaElementoColaInstancias();
   					 // eliminalo de la cola
   					 // copiarlo en la nueva cola de punteros

   			     }else if(resMayus==true){
                      // eliminarlo de la cola de memoria dinamica, mueve los punteros de la cola
   			    	  tempInstMayus = front;

   			    	  if (front == NULL)
   			    	  {
   			    	     front = rear = NULL;
   			    	  }
   			    	  else
   			    	  {
   			    	     front = front->link;
   			    	  }
   			     }

   			  // Comprueba si el comando es para minusculizar
              }

             else if(ComandoDin=='2'){

                 // Verifica que la instancia 2 es NULL para poder crear una nueva
                 Instancia2.ComandoOA=2;
                 Instancia2.datos = front->datos;

                 resMinus=ActiveObject_Init(&Instancia2);

                 if(resMinus==false){
                	 // guarda ese dato en la cola de instancias, en el caso que este ocupada esa tarea

                 }else if(resMinus==true){
                	  // eliminarlo de la cola de memoria dinamica, mueve los punteros de la cola
  			    	  tempInstMinus = front;

  			    	  if (front == NULL)
  			    	  {
  			    	     front = rear = NULL;
  			    	  }
  			    	  else
  			    	  {
  			    	     front = front->link;
  			    	  }
                 }
              }

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
    }
}


/*=====[Implementations of interrupt functions]==============================*/


/*=====[Implementations of private functions]================================*/

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
 * @brief  Calcula el tamaño de la cola de memoria dinamica y verifica si hay datos pendientes en la misma
 *
 * @param[in] void
 *
 * @return  uint8_t
 */

int TamanioCola(){

	struct node *temporal;   // estructura temporal para el calculo del tamanio de la cola
	int cnt;                 // Variabe que contiene el tamaño de la cola

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
