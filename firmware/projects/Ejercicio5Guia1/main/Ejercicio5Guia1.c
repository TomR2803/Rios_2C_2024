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
#include <gpio_mcu.h>

/*==================[macros and definitions]=================================*/
#define MAX_PIN 4
#define DIGITS 3
/*==================[internal data definition]===============================*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

uint8_t bcd_number [DIGITS]= {0};

/*==================[internal functions declaration]=========================*/
uint8_t  convertToBcdArray (uint32_t data,uint8_t digits , uint8_t * ptr_bcd_number)
{
	printf("convertToBcdArray\n");	
	for (uint8_t i = 0 ; i <digits; i++)
	{
		ptr_bcd_number[digits-i-1]=data%10;
		data=data/10;
	}
 return 0;
}

void escribirBCDEnGPIO(uint8_t bcd, gpioConf_t *pines) {

// Itera sobre el rango de pines GPIO desde MAX_PIN-1 hasta 0
	for(uint8_t i=0;i<MAX_PIN;i++)
		{
			// Verifica si el bit menos significativo de bcd es 1
			if(bcd%2)
				// Si es 1, activa el pin GPIO correspondiente
				GPIOOn(pines[MAX_PIN-1-i].pin);
			else
				// Si es 0, desactiva el pin GPIO correspondiente
				GPIOOff(pines[MAX_PIN-1-i].pin);
			// Desplaza los bits de bcd una posición a la derecha
			bcd=bcd/2;
		}
}
/*==================[external functions definition]==========================*/
void app_main(void){
	uint32_t numero=3;
	convertToBcdArray (numero, DIGITS, bcd_number);

	gpioConf_t pines[MAX_PIN];
	pines[3].pin=GPIO_20;
	pines[2].pin=GPIO_21;
	pines[1].pin=GPIO_22;
	pines[0].pin=GPIO_23;

	for (uint8_t i = 0; i < MAX_PIN; i++) {
		pines[i].dir=1;
        GPIOInit(pines[i].pin, pines[i].dir);
    }
	escribirBCDEnGPIO(bcd_number, pines);
	printf("Hello world!\n");
	
}
/*==================[end of file]============================================*/