# RTOS_II
Repositorio de la materia de RTOS II

## Descripción:
- trabajos practicos de RTOS 2 N°1 y N°2.
- Link [Condiciones de aprobación]( https://github.com/pablozizzutti/RTOS_II/blob/master/RTOS%202%20-%20Condici%C3%B3n%20de%20aprobaci%C3%B3n.pdf ).

## Alumnos:
 *  Luciano Perren
 *  Juan Pablo Menditto
 *  Pablo Zizzutti

## Aclaraciones:

- Estructura del proyecto:
  - ejercicio1: Carpeta con el código para TP1
  - ejercicio2: Carpeta con el código para TP2.
  - README.md: Este documento.

## Consideraciones de diseño
- Comandos de operación:

  - 0.Convertir los datos recibidos a mayúsculas.
  - 1.Convertir los datos recibidos a minúsculas.

# Documentación

## Estados

- ```StandBy```: Es el estado inicial y donde va el primer byte cuando se empiezan a recibir los datos.

- ```Recibiendo```: En este estado entra a partir de recibir el segundo byte hasta finalizar.

- ```ErrorLimite```: Se activa luego de los 3ms de recibido el ultimo byte.



# TP1
## Colas :
- Recepción del byte.
- Envio del byte
- Mensaje de error

## Funciones:
```
void uartDriverInit(uartMap_t uart): Inicializacion de la UART
```
```
void Driver( void* pvParameters ): Tarea Principal
```
```
void uartUsbReceiveCallback( void *unused ): Recibe los datos e inicia la maquina de estados segun el dato
```
```
uint8_t crc8_init(void): Inicialización del codigo de comprobación
```
```
uint8_t crc8_calc(uint8_t val, void *buf, int cnt): calcula el CRC de entrada
```
```
char* CalculaCRC8(char * str): Calcula el CRC de salida
```
```
bool_t VerificaColaLlena(): Indica cuando la cola supera la cantidad de memoria estipulada
```
```
void EnvioErrorUartLim(): Indica error por limite de tiempo
```
```
bool_t CheckLettersFnc(char *str): Indica si todos los caractéres de entrada són letras.
```
```
void EnvioMensajeUART(char * str, int indice): Envia mensajes por cola
```
```
int TamanioCola(): Cantidad de elementos en la cola
```
```
char* MinusToMayus(char *str): Conversión de minúscula a mayúscula
```
```
void EliminaBloqueMemoriaDinamica(): Limpia la memoria dinámica
```
```
void BorrarBufferIn(): Limpia el buffer de recepción
```



```
void uartUsbReceiveCallback( void *unused ):
Estados de la maquina:
            STANDBY
            RECIBIENDO
            ERROR LIMITE

```



```
Se utilizo una cola doblemente enlazada para armar la cola, con la siguiente estructura

struct node
{
    char datos[MEMORIADINAMICA];
    struct node *link;

}*front, *rear;
```



