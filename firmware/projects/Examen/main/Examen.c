/*! @mainpage Examen individual Electronica programable
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 04/11/2024 | Document creation		                         |
 *
 * @author Tomás Rios (tomitotorios@gmail.com)
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
#include "buzzer.h"
/*==================[macros and definitions]=================================*/
/**
 * @def DELAY
 * @brief Tiempo de refresco de medición, encendido de LEDs en microsegundos.
 */
#define MEDIR_DELAY 500000 

/**
 * @def DELAY
 * @brief Tiempo de refresco de medición, encendido de LEDs en microsegundos.
 */
#define BUZZER_DELAY 500 

/**
 * @brief Distancia medida por el sensor en centímetros.
 */
uint16_t distancia=0;

/*==================[internal data definition]===============================*/

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
//CAMBIAR
/**
 * @fn static void	BuzzerControlTask(void *pvParameter)
 * @brief Función que controla los LEDs y muestra el valor de la distancia medida en la pantalla LCD.
 * @param pvParameter Parámetro de la tarea (no utilizado).
 */
TaskHandle_t BuzzerControlTask_task_handle = NULL;
/*==================[internal functions declaration]=========================*/

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

static void BuzzerControlTask(void *pvParameter);
/*==================[external functions definition]==========================*/

static void MedirTask(void  *pvParameter){

	while (true)
	{	printf("medirTask\n");
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		//mido la distancia con el sensor y le asigno el valor
		//en cm a  la variable distancia

		distancia = HcSr04ReadDistanceInCentimeters();

		//Luego de medir, se le envia la notificacion a la tarea mostrar

		vTaskNotifyGiveFromISR(MostrarTask_task_handle, pdFALSE); 
	}
	
}

static void MostrarTask(void *pvParameter){
    while(true){

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  
		printf("mostrarTask\n");
			// Enciendo LEDs según la distancia
			//menor a 3m
			if(distancia < 300){
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
			}
			else //entre 3 y 5 metros 
				if (distancia >= 300 && distancia < 500){
					LedOn(LED_1);
					LedOn(LED_2);
					LedOff(LED_3);
				}
			else //mayor a 5 metros
				if(distancia >= 30){
					LedOn(LED_1);
					LedOff(LED_2);
					LedOff(LED_3);
				}
    }
}

static void BuzzerControlTask(void *pvParameter){
	while(true){

		BuzzerPlayTone(1840, 100);

		//si la distancia es menor a 3 metros, el buzzer suena
		//cada 500 ms (controlado con el delay de la tarea)
		if(distancia<300){
			vTaskDelay(BUZZER_DELAY / portTICK_PERIOD_MS);
		}
		
		//si la distancia está entre 3 y 5 metros, el buzzer suena
		//cada 1000 ms (controlado con el delay de la tarea)
		else if(distancia >= 300 && distancia < 500){
			vTaskDelay(BUZZER_DELAY*2 / portTICK_PERIOD_MS);
		}

	}
}

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

void app_main(void){
	//inicialización del hardware
	LedsInit();
	HcSr04Init(3, 2);
	GPIOInit(GPIO_20,GPIO_OUTPUT);
    BuzzerInit(GPIO_20);

	printf("main\n");
	//inicialización de los timers
	
		timer_config_t timer_MedirTask = {
        .timer = TIMER_A,
        .period = MEDIR_DELAY,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
	TimerInit(&timer_MedirTask);

	printf("timer init\n");
	


	//creación de tareas
	xTaskCreate(&MostrarTask, "MostrarTask", 2048, NULL, 4, &MostrarTask_task_handle);
	xTaskCreate(&MedirTask, "MedirTask", 2048, NULL, 4, &MedirTask_task_handle);
	printf("task create\n");


	//inicialización del conteo de los timers
	printf("timer start\n");
	TimerStart(timer_MedirTask.timer);
	TimerStart(timer_MostrarTask.timer);
/*==================[end of file]============================================*/