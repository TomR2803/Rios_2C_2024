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
 * @brief Tiempo de refresco de medición y encendido de LEDs en microsegundos.
 */
#define MEDIR_DELAY 100000 

/**
 * @def PESAR_DELAY
 * @brief Tiempo de refresco de medición de peso, en microsegundos.
 */
#define PESAR_DELAY 5000



/**
 *
 * @brief Valor de las mediciones de peso de la galga 1
 */

uint16_t PesoGalga1;

/**
 *
 * @brief Valor de las mediciones de peso de la galga 2
 */

 uint16_t PesoGalga2;
/**
 *
 * @brief Array con las mediciones de peso de la galga 2
 */

 uint16_t CounterMuestras=0;



/**
 *
 * @brief Distancia medida por el sensor en centímetros.
 */

uint16_t distancia=1100;

/**
 *
 * @brief Distancia medida un ciclo antes por el sensor en centímetros.
 */

uint16_t DistanciaAnterior;

/**
 *
 * @brief velocidad del vehiculo en m/s.
 */

uint16_t velocidad=0;
/**
 *
 * @brief Velocidad maima del vehiculo.
 */

uint16_t VelocidadMaxima=0;

/**
 *
 * @brief Variable booleana que indica si se realiza la medición y muestra de distancia.
 */

bool pesar = false;

/*==================[internal data definition]===============================*/
/**
 * @fn TaskHandle_t MedirTask_task_handle
 * @brief Handle de la función encargada de, medir la distancia con el sensor.
 * @param pvParameter Parámetro de la tarea (no utilizado).
 */

TaskHandle_t MedirTask_task_handle = NULL;

/**
 * @fn TaskHandle_t MostrarTask_task_handle
 * @brief Handle de la función que controla los LEDs y muestra el valor de la velocidad medida en la pantalla LCD.
 * @param pvParameter Parámetro de la tarea (no utilizado).
 */

TaskHandle_t MostrarPorLEDsTask_task_handle = NULL;
/**
 * @fn TaskHandle_t MostrarTask_task_handle
 * @brief Handle de la función que controla los LEDs y muestra el valor de la velocidad medida en la pantalla LCD.
 * @param pvParameter Parámetro de la tarea (no utilizado).
 */

TaskHandle_t PesarTask_task_handle = NULL;
/**
 * @fn TaskHandle_t ControlPorTeclado_task_handle
 * @brief Handle de la función que controla la apertura de la barrera por medio del teclado
 * @param pvParameter Parámetro de la tarea (no utilizado).
 */

TaskHandle_t ControlPorTecladoTask_task_handle = NULL;
/*==================[internal functions declaration]=========================*/

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
    vTaskNotifyGiveFromISR(PesarTask_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_2 */
}

static void MedirTask(void  *pvParameter){

	while (true)
	{	printf("medirTask\n");
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		//mido la distancia con el sensor y le asigno el valor
		//en cm a  la variable distancia
		DistanciaAnterior=distancia;
		distancia = HcSr04ReadDistanceInCentimeters();
		
		//verifico que la distancia sea menor o igual a 10 metros
		//para empezar a medir la velocidad
		if(distancia<=1000){

			velocidad=((distancia-DistanciaAnterior)/100)/0.1;//calculo de la velocidad usando el periodo de muestreo
			//como variacion de tiempo, para que la velocidad sea en m/s
			//y paso la medicion del sensor a metros dividiendo por 100
			
			//si la velocidad medida es mayor a la maxima registrada
			//se cambia la velocidad maxima registrada
			if(velocidad>VelocidadMaxima){
				VelocidadMaxima=velocidad;
			}

			if(velocidad=0){
				//si la velocidad del vehiculo es 0, es decir, si este se detiene
				//cambio pesar a true, para empezar a pesar el vehiculo
				pesar=true;
			}
			else{
				pesar=false;
			}

		}
		//si la distancia medida es mayor a 10 metros
		//cambio pesar a false, para detener la pesa del vehiculo
		//y cambio velocidad y VelocidadMaxima a 0
		else{
			pesar=false;
			velocidad=0;
			VelocidadMaxima=0;
		}
		//Luego de medir, se le envia la notificacion a la tarea mostrarPorLEDsw
		xTaskNotifyGive(MostrarPorLEDsTask_task_handle);
		//vTaskNotifyGiveFromISR(MostrarTask_task_handle, pdFALSE); //isr, solamente desde interrupciones
	}
	
}

static void MostrarPorLEDsTask(void *pvParameter){
    while(true)
	{

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  
		printf("mostrarTask\n");
			// Enciendo LEDs según la velocidad
			//vehiculo detenido
			if(velocidad=0){
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}

			else{ //entre 0 y 8 m/s 
				if (velocidad >0 && velocidad < 8){

				LedOn(LED_2);
				LedOff(LED_1);
				LedOff(LED_3);

				}
			
				else {
					if(velocidad >8 ){
					LedOn(LED_3);
					LedOff(LED_2);
					LedOff(LED_1);
					}
				}
		}
    }
}

static void PesarTask(void *pvParameter){
	uint16_t valor1,valor2,PesoVehiculo;
	while (true){

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(pesar){

		AnalogInputReadSingle(CH0,&valor1);
		AnalogInputReadSingle(CH1,&valor2);
		PesoGalga1=PesoGalga1+(valor1*20000)/3.3;
		PesoGalga2=PesoGalga2+(valor2*20000)/3.3;
		
		CounterMuestras=CounterMuestras+1;
		//Cuando el contador de muestras llega a 50,
		//calculo el peso del vehiculo con los valores de los
		//promedios de las dos galgas y envío esto y la velocidad maxima
		//por la UART 
		if(CounterMuestras>=50){
			PesoVehiculo=(PesoGalga1/50)+(PesoGalga2/50);
			UartSendString(UART_PC,"Peso:");
			UartSendString(UART_PC, (char *)UartItoa(PesoVehiculo, 10));
			UartSendString(UART_PC,"\r");
			UartSendString(UART_PC,"Velocidad máxima:");
			UartSendString(UART_PC, (char *)UartItoa(VelocidadMaxima, 10));
			UartSendString(UART_PC,"\r");
		}

		}


	}
}


void ControlPorTeclado(){
	uint8_t letra;
	UartReadByte(UART_PC, &letra);
	switch (letra)
	{
	case 'c':
		SWITCH_1_Func();
		break;
	case 'o':
		SWITCH_2_Func();
		break;
	case 'C':
		SWITCH_1_Func();
		break;
	case 'O':
		SWITCH_2_Func();
		break;
	
	}
}

void SWITCH_1_Func(){

	GPIOOff(GPIO_20);

}
void SWITCH_2_Func(){

	GPIOOn(GPIO_20);
	
}

void app_main(void){
	printf("Main\n");
	LedsInit();
	HcSr04Init(3, 2);
	GPIOInit(GPIO_20,GPIO_OUTPUT);


		//inicializacion de las entradas analogicas
		analog_input_config_t entrada_analogica_galga1 ={
		.input = CH0,
		.mode = ADC_SINGLE,
	};
	
	AnalogInputInit(&entrada_analogica_galga1);

		analog_input_config_t entrada_analogica_galga2 ={
		.input = CH1,
		.mode = ADC_SINGLE,
	};
	AnalogInputInit(&entrada_analogica_galga2);

	//inicialización de los timers
	
		timer_config_t timer_MedirTask = {
        .timer = TIMER_A,
        .period = MEDIR_DELAY,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
	TimerInit(&timer_MedirTask);


		timer_config_t timer_PesarTask = {
        .timer = TIMER_A,
        .period = PESAR_DELAY,
        .func_p = FuncTimerB,
        .param_p = NULL
    };
	TimerInit(&timer_PesarTask);

	serial_config_t my_uart = {
		.port = UART_PC, 
		.baud_rate = 38400, 
		.func_p = NULL, 
		.param_p = NULL
	};
	UartInit(&my_uart);

	SwitchActivInt(SWITCH_1, SWITCH_1_Func, NULL);
	SwitchActivInt(SWITCH_2, SWITCH_2_Func, NULL);
	
	xTaskCreate(&MedirTask, "MedirTask", 2048, NULL, 4, &MedirTask_task_handle);
	xTaskCreate(&MostrarPorLEDsTask, "MostrarPorLEDsTask", 2048, NULL, 4, &MostrarPorLEDsTask_task_handle);
	xTaskCreate(&PesarTask, "PesarTask", 2048, NULL, 4, &PesarTask_task_handle);
	printf("task create\n");

	//inicialización del conteo de los timers
	printf("timer start\n");
	TimerStart(timer_MedirTask.timer);
}
/*==================[end of file]============================================*/