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
/*==================[macros and definitions]=================================*/
#define DIGITS 3
/*==================[internal data definition]===============================*/
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

/*==================[external functions definition]==========================*/
void app_main(void){

	uint32_t data=123;

	convertToBcdArray (data, DIGITS,bcd_number);

	printf("%hhn\n",bcd_number);

	printf("Hello world!\n");
}
/*==================[end of file]============================================*/