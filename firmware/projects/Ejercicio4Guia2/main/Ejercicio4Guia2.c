/*! @mainpage Template
 *
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
 * | Potenciometro	| 	GPIO_1		|
 * | 	GND			| 	GND		    |
 * | 	3,3v		|	 3,3v	  	|

 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 03/10/2024 | Document creation		                         |
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
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
/**
 * @def TASK_PERIOD
 * @brief Periodo de delay de las tareas
 * 
 */
#define TASK_PERIOD 2000

/**
 * @def BUFFER_SIZE
 * @brief Tamaño del array ecg
 * 
 */
#define BUFFER_SIZE 231
/*==================[internal data definition]===============================*/
/**
 * @var counter
 * @brief Contador para el correcto recorrido del array ecg
 * 
 */
uint16_t counter=0;

/**
 * 
 * @brief Array con valores de un ecg digital
 * 
 */
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};

/**
 * @fn static void MedirYEnviarTask(void *pvParameter)
 * @brief Función encargada de medir valores por el CH1 y enviarlos por UART
 * convertidos separados por el carácter de fin de línea.

 * @param pvParameter Parámetro de la tarea (no utilizado).
 */
TaskHandle_t MedirYEnviarTask_task_handle = NULL;

/**
 * @fn static void ConversionDATask(void *pvParameter)
 * @brief Función encargada de la conversion DA de los valores en el array ecg
 * y su envío por el CH1
 * @param pvParameter Parámetro de la tarea (no utilizado).
 */
TaskHandle_t ConversionDATask_task_handle = NULL;
/*==================[internal functions declaration]=========================*/

static void ConversionDATask(void *pvParameter){

	while(1){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
			AnalogOutputWrite(ecg[counter]);
			if (counter<=BUFFER_SIZE){
				counter=counter+1;
			}
			else{
				counter=0;
			}
	}	
}
static void MedirYEnviarTask(void  *pvParameter){
	uint16_t valor_medido;
	while (true)
	{	
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  
		AnalogInputReadSingle(CH1, &valor_medido); // se lée una entrada analogica por el canal 1 y se asigna 
		//el valor medido y digitalizado a valor_medido
		UartSendString(UART_PC, (const char*)UartItoa(valor_medido, 10));
		UartSendString(UART_PC,"\r");
		}
	}
	

/**
 * @fn FuncTimerA
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(MedirYEnviarTask_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al LED_1 */
}

/**
 * @fn FuncTimerB
 * @brief Función invocada en la interrupción del timer B
 */
void FuncTimerB(void* param){
    vTaskNotifyGiveFromISR(ConversionDATask_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al LED_1 */
}

/*==================[external functions definition]==========================*/
void app_main(void){

	analog_input_config_t entrada_analogica ={
		.input = CH1,
		.mode = ADC_SINGLE,
	};
	
	AnalogInputInit(&entrada_analogica);
	AnalogOutputInit();

	//Configuración del puerto serie
	serial_config_t my_uart = {
		.port = UART_PC, 
		.baud_rate = 38400, //  (1/38400)*50 es menor a loss 20ms, periodo de muestreo necesario para el programa
		.func_p = NULL, 
		.param_p = NULL
	};
	UartInit(&my_uart);
	//inicialización de los timers
		timer_config_t timer_MedirYEnviarTask = {
        .timer = TIMER_A,
        .period = TASK_PERIOD,
        .func_p = FuncTimerA,
        .param_p = NULL
		};
	TimerInit(&timer_MedirYEnviarTask);

		timer_config_t timer_ConversionDATask = {
        .timer = TIMER_B,
        .period = TASK_PERIOD*2,
        .func_p = FuncTimerB,
        .param_p = NULL
		};
	TimerInit(&timer_ConversionDATask);

	//creación de tareas
	xTaskCreate(&MedirYEnviarTask,"MedirYEnviarTask",2048,NULL,4,&MedirYEnviarTask_task_handle);
	xTaskCreate(&ConversionDATask,"ConversionDATask",2048,NULL,4,&ConversionDATask_task_handle);


	//inicialización del conteo de los timers
	TimerStart(timer_MedirYEnviarTask.timer);
	TimerStart(timer_ConversionDATask.timer);
}
/*==================[end of file]============================================*/