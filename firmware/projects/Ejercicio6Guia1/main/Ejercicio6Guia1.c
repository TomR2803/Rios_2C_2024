/*! @mainpage Ejercicio 6 Guia 1
 *
 * @section genDesc General Description
 *
 * Este archivo contiene las funciones para interactuar con la pantalla LCD y los pines GPIO del microcontrolador ESP32
 * para permitirnos escribir un numero en las LCD.
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
 * @author Tomás Rios (tomitotorios@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>

/*==================[macros and definitions]=================================*/
/** @def MAX_PIN
 *  @brief Numero maximo de pines
 */
#define MAX_PIN 4
/*==================[internal data definition]===============================*/
/**
 * @struct gpioConf_t
 * @brief Estructura de configuración GPIO
 *
 * Esta estructura almacena la información de configuración de un pin GPIO.
 */
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;


/*==================[internal functions declaration]=========================*/
/** @fn uint8_t  convertToBcdArray (uint32_t data,uint8_t digits , uint8_t * ptr_bcd_number)
 * @brief Función para pasar un numero binario a binario BCD
 *
 * @param[in] data Número en binario que se desea transformar
 * @param[in] digits Cantidad de digitos del número que se desea transformar
 * @param[in] ptr_bcd_number Arreglo donde se almacena el número transformado
 *
 * @return 0
 */
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
/** @fn int8_t  escribirBCDEnGPIO(uint8_t numero, gpioConf_t *pines)
 * @brief Escribe un número en binario BCD en los pines GPIO configurados
 *
 * @param[in] bcd Número en binario BCD que se desea escribir
 * @param[in] pines arreglo de estructuras GPIO con la informacación del pin que se desea modificar
 *
 * @return 0
 */
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

/** @fn int8_t escribirNumeroEnLCD(uint32_t numero, uint8_t cantidadDigitos, gpioConf_t *pinesBCD, gpioConf_t *digitos)
 * @brief Función para escribir un numero en la pantalla LCD
 *
 * @param[in] numero Número que se desea escribir
 * @param[in] cantidadDigitos Cantidad de digitos del número que se desea escribir
 * @param[in] pinesBCD puntero a una estructura de configuración GPIO con la información de los pines de salida
 * @param[in] digitos puntero a una estructura de configuración GPIO con el estado de los pines que habilitan cada digito del LCD
 *
 * @return 0
 */
int8_t escribirNumeroEnLCD(uint32_t numero, uint8_t cantidadDigitos, gpioConf_t *pinesBCD, gpioConf_t *digitos)
{
	uint8_t bcd_number[cantidadDigitos];
	convertToBcdArray (numero,cantidadDigitos,&bcd_number);

	for(uint8_t j=0;j<cantidadDigitos;j++)
		GPIOOff(digitos[j].pin);

	for(uint8_t i=0;i<cantidadDigitos;i++)
	{
		escribirBCDEnGPIO(bcd_number[i],pinesBCD);
		GPIOOn(digitos[i].pin);
		GPIOOff(digitos[i].pin);
	}

	return 0;
}

/*==================[external functions definition]==========================*/
/** @fn void app_main(void)
 * @brief Función principal del programa
 *
 * Esta función inicializa los pines GPIO y la pantalla LCD, y luego entra en un bucle infinito donde se escribe el número en la pantalla LCD.
 */
void app_main(void){
	uint32_t numeroAMostrar=356;
	uint32_t cantidadDigitos = 3;

	// Define un arreglo de estructuras gpioConf_t para almacenar las configuraciones de pines para los segmentos
    gpioConf_t pines[MAX_PIN];
    
    // Define un arreglo de estructuras gpioConf_t para almacenar las configuraciones de pines para los dígitos
    gpioConf_t digitos[cantidadDigitos];
    
    // Configura los pines para los segmentos
    pines[3].pin = GPIO_20;
    pines[2].pin = GPIO_21;
    pines[1].pin = GPIO_22;
    pines[0].pin = GPIO_23;
    
    // Configura los pines para los dígitos
    digitos[0].pin = GPIO_19;
    digitos[1].pin = GPIO_18;
    digitos[2].pin = GPIO_9;
    
    // Inicializa los pines para los segmentos
    for (uint8_t i = 0; i < MAX_PIN; i++) {
        // Establece la dirección del pin como salida (1)
        pines[i].dir = 1;
        // Inicializa el pin GPIO con la dirección especificada
        GPIOInit(pines[i].pin, pines[i].dir);
    }
    
    // Inicializa los pines para los dígitos
    for (uint8_t i = 0; i < cantidadDigitos; i++) {
        // Establece la dirección del pin como salida (1)
        digitos[i].dir = 1;
        // Inicializa el pin GPIO con la dirección especificada
        GPIOInit(digitos[i].pin, digitos[i].dir);
    }
    
    // Bucle infinito para mostrar continuamente el número
    while(1){
        // Llama a la función para escribir el número en la pantalla LCD
        escribirNumeroEnLCD(numeroAMostrar, cantidadDigitos, &pines, &digitos);
    }
    
	printf("Hello world!\n");
}
/*==================[end of file]============================================*/