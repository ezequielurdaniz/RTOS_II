# RTOS_II
Repositorio de la materia de RTOS II

## Descripción:
- Trabajos practicos de RTOS 2 N°1 y N°2.
- Link [Condiciones de aprobación]( https://github.com/pablozizzutti/RTOS_II/blob/master/RTOS%202%20-%20Condici%C3%B3n%20de%20aprobaci%C3%B3n.pdf )
- TP 1 https://drive.google.com/drive/folders/14kG0asygjdLQRMK713hIFkTuAyrfpYos
- TP 2 https://drive.google.com/drive/folders/14kG0asygjdLQRMK713hIFkTuAyrfpYos

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
- Comandos de operación: )PARA EL TP2

  - 0.Convertir los datos recibidos a mayúsculas.
  - 1.Convertir los datos recibidos a minúsculas.
  - Incluir los simbolos de corchete de apertura y fin para el FRAME

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



# TP2
## Colas :
- Recepción del byte.
- Envio del byte
- Mensaje de error

## Obejeto activo:
- Cada operación se delegó a un objeto activo diferente al cual se envia el buffer a procesar
- 0.Convertir los datos recibidos a mayúsculas.
- 1.Convertir los datos recibidos a minúsculas.
- Se instancia desde la capa 3 en Driver.c y se le pasan los datos desde un struct creado




## Estructura del programa

```
* Inicializaciones -----> de placa, 
      main.c       -----> timers, 
                   -----> colas 
                   -----> creación de tareas DRIVER y DEMONIO

* TimersControl.c ------> Aquí estan las funciones de los timers y sus condiciones 

* Callbacks.c -----> Máquina de estados para la recepción de datos
              -----> Envio y Recepsión del FRAME
                


* Driver.c ----> Creación de la memoria dinámica
           ----> Recepción de los datos de la capa 2 por cola
           ----> Validación de CRC
           ----> Validación de tipo de char (letras)
           ----> Análisis del comando para conversión
           ----> Llamado del Objeto Activo
           ----> devolución de datos a la capa 2
           ----> Liberación de memoria dinámica

* OA.c  -----> Estructura para el pasaje de datos 
        -----> Función Mayusculizar
        -----> Función Minusculizar
        -----> devolución de dato convertido



```
## Funciones:


## Funciones del programa anterior

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

## Funciones del TP2



```
uint8_t ObtenerCantidadCaracteres(): total de caracteres
```
```
uint8_t ObtenerComando():devuelve el valor 1 ó 2 que va a determinar si mayusc o minusc
```
```
void Demonio( void* pvParameters ): Esta tarea sirve para estar constantemente viendo si hay nuevos datos en la cola para crear el ojbeto activo  (si no es que ya esta creado) y mediante el mismo hacer la conversion de las letras, Son dos Objetos Activos uno para cada comando, creados a partir del mismo que se describe aq continuación.
```
```
bool_t ActiveObject_Init( Active_Object_t* Obj ): crea el objeto activo teniendo en cuenta el parametro "obj" que es el que indica que tipo de objeto activo es para mayusculizar o minusculizar.
```
```
void AO_Mayus( void* param  ): función de mayusculizar.
```
```
void AO_Minus( void* param  ): función de minusculizar.
```





