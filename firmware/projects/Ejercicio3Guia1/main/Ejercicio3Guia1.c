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
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
/*==================[macros and definitions]=================================*/
#define OFF 0
#define TOGGLE 2
#define ON 1
#define retardo 100
/*==================[internal data definition]===============================*/
struct leds
{
	uint8_t mode;       //ON, OFF, TOGGLE
	uint8_t n_led;        //indica el número de led a controlar
	uint8_t n_ciclos;   //indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;    //indica el tiempo de cada ciclo
} ;
/*==================[internal functions declaration]=========================*/

void ModosLED(struct leds *Led){	
	switch (Led->mode){
		case ON:
		printf("ON\n");		
			switch (Led->n_led){
				case 1:
					LedOn(LED_1);
				break;
				case 2:
					LedOn(LED_2);
				break;
				case 3:
					LedOn(LED_3);
				break;
			}
		break;
		case OFF:		
			switch (Led->n_led){
				case 1:
					LedOff(LED_1);
				break;
				case 2:
					LedOff(LED_2);
				break;
				case 3:
					LedOff(LED_3);
				break;
			}
		break;
		case TOGGLE:
		printf("TOGGLE\n");
			switch (Led->n_led){
				case 1:
					for(int i=0; i<Led->n_ciclos; i++){
						LedToggle(LED_1);
						for(int j=0; j<Led->periodo/retardo; j++){
							vTaskDelay(retardo/ portTICK_PERIOD_MS);
						}
					};
				break;
				case 2:
					for(int i=0; i<Led->n_ciclos; i++){
						LedToggle(LED_2);
						for(int j=0; j<Led->periodo/retardo; j++){
							vTaskDelay(retardo/ portTICK_PERIOD_MS);
						}
					};
				break;
				case 3:
				printf("TOGGle 3\n");
					for(int i=0; i<Led->n_ciclos; i++){
						LedToggle(LED_3);
						for(int j=0; j<Led->periodo/retardo; j++){
							vTaskDelay(retardo/ portTICK_PERIOD_MS);
						}
					};
				break;
			}
		break;
			
	
	default:
		break;
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	printf("Hello world!\n");
	struct leds led1, led2, led3;
	led1.mode = ON;
	led1.n_ciclos = 0;
	led1.n_led = 1;
	led1.periodo = 0;

	led2.mode = ON;
	led2.n_ciclos = 0;
	led2.n_led = 2;
	led2.periodo = 0;

	led3.mode = TOGGLE;
	led3.n_ciclos = 50;
	led3.n_led = 3;
	led3.periodo = 1000;

	ModosLED(&led3);
	ModosLED(&led1);
	printf("Hello world!\n");
}
/*==================[end of file]============================================*/