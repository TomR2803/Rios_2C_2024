/*! @mainpage Proyecto 2 Ejercicio 3
 *
 * @section genDesc General Description
 *
* Este progrma, usado con la placa ESP32 y el  sensor HC-SR04, mide distancia y controla el encendido de los LEDS de la placa según la medición.
 * Asimismo, se muestra la distancia medida en cm en una pantalla LCD y envía las mediciones por puerto serie para poder observarlos con
 * la extensión "Serial monitor". Se puede también controlar el encendido o apagado del dispositivo con la Tecla1 en la placa
 * o la tecla/letra "O" en el Serial Monitor; y controlar si se mantiene el último valor medido en la pantalla usando la Tecla2 o la tecla/letra "H" en
 * el Serial Monitor. Este funcionamiento es implementado con el uso de interrupciones y timers
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * | 	 +5V	 	| 	 +5V		|
 * | 	 GND	 	| 	 GND		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Tomás Rios(tomr2803@outlook.com)
 *
 */


/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"

/* ==================[macros and definitions]=================================*/
/**
 * @def DELAY
 * @brief Tiempo de refresco de medición, encendido de LEDs y muestra en pantalla LCD en milisegundos.
 */
#define TASK_DELAY 1000000

/**
 * @def SWITCHES_DELAY
 * @brief Tiempo de delay para el control de lectura de los switches en milisegundos.
 */
#define SWITCHES_DELAY 100

/**
 * @brief Distancia medida por el sensor en centímetros.
 */
uint16_t distancia=0;

/**
 * @brief Variable booleana que indica si se realiza la medición y muestra de distancia.
 */
bool global_on = true;

/**
 * @brief Variable booleana que indica si se mantiene el último valor medido.
 */
bool global_hold = false;
/*==================[internal data definition]===============================*/
/**
 * @fn static void TaskTeclas(void *pvParameter)
 * @brief Función encargada de leer el estado de los switches y actualizar las variables on y hold.
 * @param pvParameter Parámetro de la tarea (no utilizado).
 */
TaskHandle_t TaskTeclas_task_handle = NULL;

/**
 * @fn static void MedirTask(void *pvParameter)
 * @brief Función encargada de, en caso de estar encendido el dispositivo, medir la distancia con el sensor.
 * @param pvParameter Parámetro de la tarea (no utilizado).
 */
TaskHandle_t MedirTask_task_handle = NULL;


/**
 * @fn static void MostrarTask(void *pvParameter)
 * @brief Función que controla los LEDs y muestra el valor de la distancia medida en la pantalla LCD.
 * @param pvParameter Parámetro de la tarea (no utilizado).
 */
TaskHandle_t MostrarTask_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
/**
 * @fn void ControlPorTeclado()
 * @brief Función que lée la letra  introducida por el usuario y cambia las variables globales "global_on"(si se ingresa la O)
 *  ó  "global_hold" (si se ingresa la H).
 * 
 * @param NULL
 * @return
 */
void ControlPorTeclado();
/**
 * @fn void LeerDistanciaEnTerminal()
 * @brief Función que envía la  distancia medida por el sensor al puerto serie para que pueda ser leida por
 * medio del serial monitor.
 * 
 * @param NULL
 * @return
 */
void LeerDistanciaEnTerminal();

/**
 * @fn static void MedirTask(void *pvParameter)
 * @brief Tarea encargada de medir la distancia por medio del sensor.
 * 
 * Si la  variable global_on es verdadera, es decir, si el dispositivo está encendido
 * se realiza la medición.
 * 
 * @param pvParameter Parámetro de la tarea (no utilizado).
 * @return
 */
static void MedirTask(void *pvParameter);

/**
 * @fn static void MostrarTask(void *pvParameter)
 * @brief Tarea encargada de controlar los LEDs y la pantalla LCD según la distancia medida.
 * 
 * Según la distancia medida en la tarea  MedirTask, se encienden los LED correspondientes:
 * 
 * - Menos de 10 cm: Apaga todos los LEDs.
 * - Entre 10 y 20 cm: Enciende el LED_1.
 * - Entre 20 y 30 cm: Enciende el LED_1 y el LED_2.
 * - Más de 30 cm: Enciende el LED_1, LED_2 y LED_3.
 * 
 * Junto con esto, si no se está en modo HOLD, muestra la distancia medida en la pantalla LCD.
 * 
 * @param pvParameter Parámetro de la tarea (no utilizado).
 * @return
 */
static void MostrarTask(void *pvParameter);

/**
 * @fn void SWITCH_1_Func()
 * @brief Función que se  ejecuta al presionar la tecla 1. Interrumpe la tarea que esté corriendo para ejecutarse
 * y cambia la  variable global_on a su valor contrario.


 * 
 * @param NULL
 * @return
 */
void SWITCH_1_Func();

/**
 * @fn void SWITCH_2_Func()
 * @brief Función que se  ejecuta al presionar la tecla 2. Interrumpe la tarea que esté corriendo para ejecutarse
 * y cambia la  variable global_hold a su valor contrario.


 * 
 * @param NULL
 * @return
 */
void SWITCH_2_Func();
/*==================[external functions definition]==========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(MedirTask_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al LED_1 */
}

/**
 * @brief Función invocada en la interrupción del timer B
 */
void FuncTimerB(void* param){
    vTaskNotifyGiveFromISR(MostrarTask_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_2 */
}

static void MedirTask(void  *pvParameter){

	while (true)
	{	printf("medirTask\n");
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  
		if(global_on==true){
		distancia = HcSr04ReadDistanceInCentimeters();
		}
	}
	
}

static void MostrarTask(void *pvParameter){
    while(true){
		//Checkeo si la  tarea de medición está activa
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  
		printf("mostrarTask\n");
		if(global_on == true)
		{
			// Enciendo LEDs según la distancia
			if(distancia < 10){
				LedsOffAll();
			}
			else if (distancia >= 10 && distancia < 20){
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
			else if(distancia >= 20 && distancia < 30){
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
			}
			else if(distancia >= 30){
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
			}

			// Si el modo hold no está activado, muestro distancia en la pantalla LCD
			if(global_hold == false){
				LcdItsE0803Write(distancia);
			}
		}
		else
		{
			// Apagar LEDs y pantalla LCD si on está apagado
			LedOff(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
			LcdItsE0803Off();
		}
    }
}

void LeerDistanciaEnTerminal(){
	UartSendString(UART_PC, (const char*)UartItoa(distancia, 10));
	UartSendString(UART_PC, " cm\r\n");
}

void ControlPorTeclado(){
	uint8_t letra;
	UartReadByte(UART_PC, &letra);
	switch (letra)
	{
	case 'O':
		SWITCH_1_Func();
		break;
	case 'H':
		SWITCH_2_Func();
		break;
	
	}
}

void SWITCH_1_Func(){

	global_on=!global_on;

}
void SWITCH_2_Func(){

	global_hold=!global_hold;
	
}

void LeerDistanciaEnTerminal(){
	UartSendString(UART_PC, (const char*)UartItoa(distancia, 10));
	UartSendString(UART_PC, " cm\r\n");
}
/*==================[external functions definition]==========================*/
/**
 * @brief Función principal de la aplicación. Inicializa el hardware y crea las tareas para medir distancia,
 *  mostrarla por medio de la pantalla y los LEDs y leer las teclas.
 */
void app_main(void){
	//inicialización del hardware
	LedsInit();
	HcSr04Init(3, 2);
	LcdItsE0803Init();

	printf("main\n");
	SwitchesInit();
	SwitchActivInt(SWITCH_1, SWITCH_1_Func, NULL);
	SwitchActivInt(SWITCH_2, SWITCH_2_Func, NULL);


	//inicialización de los timers
	
		timer_config_t timer_MedirTask = {
        .timer = TIMER_A,
        .period = TASK_DELAY,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
	TimerInit(&timer_MedirTask);

		timer_config_t timer_MostrarTask = {
        .timer = TIMER_B,
        .period = TASK_DELAY,
        .func_p = FuncTimerB,
        .param_p = NULL
		};
	TimerInit(&timer_MostrarTask);
	printf("timer init\n");
	
	//inicialización de la Uart
	serial_config_t ConfigUart = {			
	.port = UART_PC,	
	.baud_rate = 9600,		
	.func_p = ControlPorTeclado,
    .param_p = NULL
	};
	UartInit(&ConfigUart);


	//creación de tareas
	xTaskCreate(&MostrarTask, "MostrarTask", 2048, NULL, 4, &MostrarTask_task_handle);
	xTaskCreate(&MedirTask, "MedirTask", 2048, NULL, 4, &MedirTask_task_handle);
	printf("task create\n");


	//inicialización del conteo de los timers
	printf("timer start\n");
	TimerStart(timer_MedirTask.timer);
	TimerStart(timer_MostrarTask.timer);

	}

/*=================[end of file]============================================*/