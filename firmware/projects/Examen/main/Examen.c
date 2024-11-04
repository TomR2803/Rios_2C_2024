/*! @mainpage Examen individual Electronica programable
 * @section genDesc General Description
 *
 * Este programa utiliza un acelerometro triaxial y un sensor de ultrasonido que mide distancia
 * para permitir la detección de caídas y alertar al usuario cuándo un auto
 * se encuentra a una distancia cercana a la bicicleta de este
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | Acelerometro canal 1| 	GPIO_0	|
 * |Acelerometro canal 2| 	GPIO_1		|
 * |Acelerometro canal 3| 	GPIO_2		|
 * | 	ECHO	 	| 	GPIO_12		|
 * | 	TRIGGER	 	| 	GPIO_13		|
 * | 	BUZZER	 	| 	GPIO_20		|
 * | 	GND	 	| 	GND		|
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
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
/**
 * @def MEDIR_DELAY
 * @brief Tiempo de refresco de medición, encendido de LEDs y envío de mensajes por UART en microsegundos.
 */
#define MEDIR_DELAY 500000 

/**
 * @def CAIDA_DELAY
 * @brief Tiempo de refresco de medición de la gravedad, en microsegundos.
 */
#define CAIDA_DELAY 10000

/**
 * @def DELAY
 * @brief Periodo base de sonido del buzzer, en milisegundos.
 */
#define BUZZER_DELAY 500 

/**
 * @brief Distancia medida por el sensor en centímetros.
 */
uint16_t distancia=0;

/*==================[internal data definition]===============================*/

/**
 * @fn static void MedirTask(void *pvParameter)
 * @brief Tarea encargada de medir la distancia con el sensor HC-SR04 y actualizar la distancia medida.

 * @param pvParameter Parámetro de la tarea (no utilizado).
 */
TaskHandle_t MedirTask_task_handle = NULL;

/**
 * @fn static void MostrarTask(void *pvParameter)
 * @brief Tarea que controla los LEDs y envía  mensajes por UART según la distancia medida.

 * @param pvParameter Parámetro de la tarea (no utilizado).
 */
TaskHandle_t MostrarTask_task_handle = NULL;

/**
 * @fn static void	BuzzerControlTask(void *pvParameter)
 * @brief Tarea que controla el buzzer y la frecuencia con la que suena
 * @param pvParameter Parámetro de la tarea (no utilizado).
 */
TaskHandle_t BuzzerControlTask_task_handle = NULL;
/**
 * @fn static void ControlCaidaTask(void *pvParameter)
 * @brief Tarea que detecta cuando hay un caída y envía un mensaje por  UART en dicho caso

 * @param pvParameter Parámetro de la tarea (no utilizado).
 */
TaskHandle_t ControlCaidaTask_task_handle = NULL;


/*==================[internal functions declaration]=========================*/

/**
 * @fn static void MedirTask(void *pvParameter)
 * @brief Tarea encargada de medir la distancia por medio del sensor.
 * 
 * @param pvParameter Parámetro de la tarea (no utilizado).
 * @return
 */
static void MedirTask(void *pvParameter);

/**
 * @fn static void MostrarTask(void *pvParameter)
 * @brief Tarea encargada de controlar los LEDs y el envío de advertencias según la distancia medida.
 * 
 * Según la distancia medida en la tarea  MedirTask, se encienden los LED correspondientes:
 * 
 * Led verde para distancias mayores a 5 metros
 * Led verde y amarillo para distancias entre 5 y 3 metros(precaucion)
 * Led verde, amariilo y rojo para distancias menores a 3 metros(peligro).
 * 
 * Ademas de esto, se envían advertencias por UART para el segundo y tercer caso
 * 
 * @param pvParameter Parámetro de la tarea (no utilizado).
 * @return
 */
static void MostrarTask(void *pvParameter);
/**
 * @fn static void	BuzzerControlTask(void *pvParameter)
 * @brief Tarea que controla el buzzer y la frecuencia con la que suena
 * La alarma sonará con una frecuencia de 1 segundo en el caso
 * 	de precaución y cada 0.5 segundos en el caso de peligro.
 * 
 * @param pvParameter Parámetro de la tarea (no utilizado).
 * @return
 */

static void BuzzerControlTask(void *pvParameter);
/**
 * @fn static void ControlCaidaTask(void *pvParameter)
 * @brief Tarea que detecta cuando hay un caída
 * 	Si la sumatoria (escalar) de la aceleración en los tres ejes supera los 4G se deberá
 *	enviar el siguiente mensaje a la aplicación: “Caída detectada”
 * @param pvParameter Parámetro de la tarea (no utilizado).
 * @return
 */
static void ControlCaidaTask(void *pvParameter);
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
					//envío el mensaje correspondiente a través de la UART
					UartSendString(UART_CONNECTOR,"Precaución, vehículo cerca");
					UartSendString(UART_CONNECTOR,"\r");

				}
			else //mayor a 5 metros
				if(distancia >= 30){

					LedOn(LED_1);
					LedOff(LED_2);
					LedOff(LED_3);
					//envío el mensaje correspondiente a través de la UART
					UartSendString(UART_CONNECTOR,"Peligro, vehículo cerca");
					UartSendString(UART_CONNECTOR,"\r");
				}
    }
}

static void BuzzerControlTask(void *pvParameter){
	while(true){

//		BuzzerPlayTone(1840, 100);

		//si la distancia es menor a 3 metros, el buzzer suena
		//cada 500 ms (controlado con el delay de la tarea)
		if(distancia<300){
			GPIOOn(GPIO_20);
			vTaskDelay(BUZZER_DELAY / portTICK_PERIOD_MS);
			GPIOOff(GPIO_20);
		}
		
		//si la distancia está entre 3 y 5 metros, el buzzer suena
		//cada 1000 ms (controlado con el delay de la tarea)
		else if(distancia >= 300 && distancia < 500){
			GPIOOn(GPIO_20);
			vTaskDelay(BUZZER_DELAY*2 / portTICK_PERIOD_MS);
			GPIOOff(GPIO_20);
		}

	}
}

static void ControlCaidaTask(void *pvParameter){
	uint16_t valor1,valor2,valor3;
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		// se lée una entrada analogica por los canales y se asigna 
		//el valor medido y digitalizado a valor1,valor2 y valor3, en voltios
		AnalogInputReadSingle(CH0,&valor1);
		AnalogInputReadSingle(CH1,&valor2);
		AnalogInputReadSingle(CH2,&valor3);

		//paso los valores de mV a G
		valor1=(valor1/(0.3*1000))-5.5;
		valor2=(valor2/(0.3*1000))-5.5;
		valor3=(valor3/(0.3*1000))-5.5;
		
		if(valor1+valor2+valor3>4){
			//si la suma de los valores es mayor a 4,se envía el mensaje por la UART
			UartSendString(UART_CONNECTOR,"Caida detectada");
			UartSendString(UART_CONNECTOR,"\r");
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
    vTaskNotifyGiveFromISR(ControlCaidaTask_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_2 */
}

void app_main(void){
	printf("main\n");
	//inicialización del hardware
	LedsInit();
	HcSr04Init(12, 13);

	GPIOInit(GPIO_20,GPIO_OUTPUT);
	BuzzerInit(GPIO_20);

	
		//Configuración del puerto serie
	serial_config_t my_uart = {
		.port = UART_CONNECTOR, 
		.baud_rate = 38400, 
		.func_p = NULL, 
		.param_p = NULL
	};
	UartInit(&my_uart);

	//inicializacion de las entradas analogicas
		analog_input_config_t entrada_analogica_1 ={
		.input = CH0,
		.mode = ADC_SINGLE,
	};
	
	AnalogInputInit(&entrada_analogica_1);

		analog_input_config_t entrada_analogica_2 ={
		.input = CH1,
		.mode = ADC_SINGLE,
	};
	
	AnalogInputInit(&entrada_analogica_2);

		analog_input_config_t entrada_analogica_3 ={
		.input = CH2,
		.mode = ADC_SINGLE,
	};
	
	AnalogInputInit(&entrada_analogica_3);

	//inicialización de los timers
	
		timer_config_t timer_MedirTask = {
        .timer = TIMER_A,
        .period = MEDIR_DELAY,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
	TimerInit(&timer_MedirTask);

			timer_config_t timer_ControlCaidaTask = {
        .timer = TIMER_B,
        .period = CAIDA_DELAY,
        .func_p = FuncTimerB,
        .param_p = NULL
    };
	TimerInit(&timer_ControlCaidaTask);

	printf("timer init\n");
	


	//creación de tareas
	xTaskCreate(&MostrarTask, "MostrarTask", 2048, NULL, 4, &MostrarTask_task_handle);
	xTaskCreate(&MedirTask, "MedirTask", 2048, NULL, 4, &MedirTask_task_handle);
	xTaskCreate(&BuzzerControlTask, "BuzzerControlTask", 2048, NULL, 4, &BuzzerControlTask_task_handle);
	xTaskCreate(&ControlCaidaTask, "ControlCaidaTask", 2048, NULL, 4, &ControlCaidaTask_task_handle);
	printf("task create\n");


	//inicialización del conteo de los timers
	printf("timer start\n");
	TimerStart(timer_MedirTask.timer);
	TimerStart(timer_ControlCaidaTask.timer);
/*==================[end of file]============================================*/