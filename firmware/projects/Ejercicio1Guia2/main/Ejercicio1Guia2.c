/*! @mainpage Proyecto 2 Ejercicio 1
 *
 * @section genDesc General Description
 *
 * Este progrma, usado con la placa ESP32 y el  sensor HC-SR04, mide distancia y controla el encendido de los LEDS de la placa según la medición.
 * Asimismo, se muestra la distancia medida en cm en una pantalla LCD, pudiendo controlar el encendido o apagado del dispositivo con la Tecla1 y 
 * controlar si se mantiene el último valor medido en la pantalla usando la Tecla2
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * | 	 +3,3V	 	| 	 +3,3V		|
 * | 	 GND	 	| 	 GND		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2024 | Document creation		                         |
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

/* ==================[macros and definitions]=================================*/

/**
 * @def TASK_DELAY
 * @brief Tiempo de refresco de medición, encendido de LEDs y muestra en pantalla LCD en milisegundos.
 */

#define TASK_DELAY 1000

/**
 * @def SWITCHES_DELAY
 * @brief Tiempo de delay para el control de lectura de los switches en milisegundos.
 */

#define SWITCHES_DELAY 100

/**
 *
 * @brief Distancia medida por el sensor en centímetros.
 */

uint16_t distancia=0;

/**
 *
 * @brief Variable booleana que indica si se realiza la medición y muestra de distancia.
 */

bool global_on = false;

/**
 * @def global_hold
 * @brief Variable booleana que indica si se mantiene el último valor medido.
 */

bool global_hold = false;

/*==================[internal data definition]===============================*/

/**
 * @fn TaskHandle_t TaskTeclas_task_handle
 * @brief Handle de la función encargada de leer el estado de los switches y actualizar las variables on y hold.
 * @param pvParameter Parámetro de la tarea (no utilizado).
 */

TaskHandle_t TaskTeclas_task_handle = NULL;

/**
 * @fn TaskHandle_t MedirTask_task_handle
 * @brief Handle de la función encargada de, en caso de estar encendido el dispositivo, medir la distancia con el sensor.
 * @param pvParameter Parámetro de la tarea (no utilizado).
 */

TaskHandle_t MedirTask_task_handle = NULL;


/**
 * @fn TaskHandle_t MostrarTask_task_handle
 * @brief Handle de la función que controla los LEDs y muestra el valor de la distancia medida en la pantalla LCD.
 * @param pvParameter Parámetro de la tarea (no utilizado).
 */

TaskHandle_t MostrarTask_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

/**
 * @fn static void TaskTeclas(void *pvParameter)
 * @brief Tarea encargada de leer el estado de los switches y cambiar las variables global_on y global_hold.
 * Las teclas están asignadas de la siguiente manera:
 * - TEC1: ON/OFF del dispositivo de medición.
 * - TEC2: Activa o desactiva el modo "hold", que dejá mostrando el ultimo valor medido en la pantalla.
 * 
 * @param pvParameter Parámetro de la tarea (no utilizado).
 * @return
 */

static void TaskTeclas(void *pvParameter);

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
 *
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

/*==================[external functions definition]==========================*/
static void MedirTask(void  *pvParameter){
 printf("Medir\n");
	while (true)
	{
		if(global_on)
		distancia = HcSr04ReadDistanceInCentimeters();
		vTaskDelay(TASK_DELAY / portTICK_PERIOD_MS);
	}
	
}

static void MostrarTask(void *pvParameter){
	 printf("Mostrar\n");
    while(true){
		//Checkeo si la  tarea de medición está activa
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
		vTaskDelay(TASK_DELAY / portTICK_PERIOD_MS);
    }
}

static void TaskTeclas(void *pvParameter){
	 printf("teclas\n");
    while(true){

		uint8_t switches = SwitchesRead();
		switch(switches)
		{
			case SWITCH_1:
				global_on = !global_on;
			break;
			
			case SWITCH_2:
				global_hold = !global_hold;
			break;

			case SWITCH_1 | SWITCH_2:
				global_on =! global_on;
				global_hold =! global_hold;
			break;
		}
		vTaskDelay(SWITCHES_DELAY/ portTICK_PERIOD_MS);
    }
}
/*==================[external functions definition]==========================*/
/**
 * @brief Función principal de la aplicación. Inicializa el hardware y crea las tareas para medir distancia,
 *  mostrarla por medio de la pantalla y los LEDs y leer las teclas.
 */
void app_main(void){
	// Inicializar el hardware
	printf("Main\n");
	LedsInit();
	HcSr04Init(3, 2);
	LcdItsE0803Init();
	SwitchesInit();
 	//Crear las tareas
	xTaskCreate(&MostrarTask, "MostrarTask", 2048, NULL, 4, &MostrarTask_task_handle);
	xTaskCreate(&MedirTask, "MedirTask", 2048, NULL, 4, &MedirTask_task_handle);
    xTaskCreate(&TaskTeclas, "TaskTeclas",2048, NULL, 5, &TaskTeclas_task_handle);
	
	}

/*=================[end of file]============================================*/
