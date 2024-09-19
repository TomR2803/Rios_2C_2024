/*! @mainpage Proyecto 2 Ejercicio 2
 *
 * @section genDesc General Description
 *
 * Este progrma, usado con la placa ESP32 y el  sensor HC-SR04, mide distancia y controla el encendido de los LEDS de la placa según la medición.
 * Asimismo, se muestra la distancia medida en cm en una pantalla LCD, pudiendo controlar el encendido o apagado del dispositivo con la Tecla1 y 
 * controlar si se mantiene el último valor medido en la pantalla usando la Tecla2. Este funcionamiento es implementado con el uso de interrupciones
 * y timers
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
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");
}
/*==================[end of file]============================================*/