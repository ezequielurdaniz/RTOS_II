/* Copyright 2016, Eric Pernia.
 * All rights reserved.
 *
 * This file is part sAPI library for microcontrollers.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Date: 2018-26-09
 */
#include "vtask.h"
#include "sapi.h"         /* <= sAPI header */
#include "Capa2.h"

/* Callbacks - Declaraciones */



/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */
int Capa2 (void){
   /* ------------- INICIALIZACIONES ------------- */

   portTickType xPeriodicity =  10 / portTICK_RATE_MS;
   portTickType xLastWakeTime = xTaskGetTickCount();

   // Envío para identificar cual es cual y para arrancar la secuencia (si no
   // hasta que no se envie un byte desde cada terminal no funciona)
   //uartWriteByte(UART_USB, 'u');

   /* ------------- REPETIR POR SIEMPRE ------------- */

   while(1) {
      /* Sleep until next Interrupt happens */
      sleepUntilNextInterrupt();
   }

   /* NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa no es llamado
      por ningun S.O. */
   return 0 ;
}

/* Callbacks - Implementaciones */

uint8_t Recibido = 0;
bool_t dataToSendToUart232Pending = FALSE;

// Recibo de la PC en la UART_USB
void uartUsbReceiveCallback( void *unused )
{
   Recibido = uartRxRead(UART_USB);
   flag1 = TRUE;
}


uint8_t dataToSendToUartUsb = 0;
bool_t dataToSendToUartUsbPending = FALSE;


// Envio a la PC desde la UART_USB
void uartUsbSendCallback( void *unused )
{
   if(dataToSendToUartUsbPending){
      uartTxWrite(UART_USB, dataToSendToUartUsb);
      dataToSendToUartUsb = 0;
      dataToSendToUartUsbPending = FALSE;
   }
}
