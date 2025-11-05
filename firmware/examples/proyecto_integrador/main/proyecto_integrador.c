/*! @mainpage Proyecto Integrador
 *
 * \section genDesc General Description
 *
 * Proyecto integrador de la cátedra Electrónica Programable en la que se realiza un prototipado de una
 * luz de dentista que sea ajustable automaticamente en intensidad y apertura de iris de luz dependiendo
 * de la distancia al paciente
 *
 * \section hardConn Hardware Connection
 *
 * |   	MRFC522		|   ESP-EDU		|
 * |:--------------:|:--------------|
 * | 	SDO/MISO 	|	GPIO_22		|
 * | 	3V3		 	| 	3V3			|
 * | 	SCK		 	| 	GPIO_20		|
 * | 	SDI/MOSI 	| 	GPIO_21		|
 * | 	RESET	 	| 	GPIO_18		|
 * | 	CS		 	| 	GPIO_9		|
 * | 	GND		 	| 	GND			|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 22/10/2025 | Document creation		                         |
 *
 * @author Gastón Jair Díaz (diazgastonj@gmail.com)
 * @author Juan Ignacio Oliva (juaaaaanioliiiva@gmail.com)
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "uart_mcu.h"
#include "pwm_mcu.h"
#include "hc_sr04.h"
/*==================[macros and definitions]=================================*/
#define DISTANCE_CLOSE = 25	   // distancia en cm para modo de trabajo intenso
#define DISTANCE_MEDIUM = 45   // distancia en cm para modo de trabajo moderado
#define DISTANCE_FAR = 60	   // distancia en cm para modo de reposo
#define DISTANCE_OFF = 100	   // distancia en cm para modo apagado
#define DUTY_CYCLE_MAX = 100   // ciclo de trabajo maximo
#define DUTY_CYCLE_MEDIUM = 60 // ciclo de trabajo medio medio
#define DUTY_CYCLE_LOW = 30	   // ciclo de trabajo bajo
#define DUTY_CYCLE_OFF = 0	   // ciclo de trabajo apagado
#define CONFIG_WRITE_PERIOD_US 500 * 1000
/*==================[internal data definition]===============================*/
uint16_t _distancia;
bool _medicionActivada = false;

void FuncTimerA(void *param)
{
	// vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);    /* Envía una notificación a la tarea Medir asociada al sensor */
}

static void Medir(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
		if (_medicionActivada)
			_distancia = HcSr04ReadDistanceInCentimeters(); // Realiza la medición de distancia
	}
}

static void EnviarDatosUART(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		UartSendString(UART_PC, "Distanca: .\r\n");
		UartSendString(UART_PC, (char)*UartItoa(_distancia, 10));
	}
}

static void Regular_intensidad_luz(void *pvParameter)
{
	if (_medicionActivada)
	{

		if (_distancia < DISTANCE_CLOSE) // modo maxima intensidad para procedimientos cortos
			PWMSetDutyCycle(PWM_1, DUTY_CYCLE_MAX);
		else if (_distancia < DISTANCE_MEDIUM)
			PWMSetDutyCycle(PWM_1, DUTY_CYCLE_MEDIUM);
		else if (_distancia < DISTANCE_FAR)
			PWMSetDutyCycle(PWM_1, DUTY_CYCLE_LOW);
		else if	(_distancia > DISTANCE_OFF) // se apaga el led
				PWMSetDutyCycle(PWM_1, DUTY_CYCLE_OFF);
	}
	else
	{
		PWMSetDutyCycle(PWM_1, DUTY_CYCLE_OFF);
	}
}
static void Regular_apertura_haz(void *pvParameter)
{
}
/*==================[external functions definition]==========================*/
void app_main(void)
{

	serial_config_t UART_USB;
	UART_USB.baud_rate = 115200;
	UART_USB.port = UART_PC;
	UartInit(&UART_USB);
	// setupRFID(&mfrcInstance);

	/*
	while(true){
		UartSendString(UART_PC,"Reading... \r\n");
		if (PICC_IsNewCardPresent(mfrcInstance)) {
			if (PICC_ReadCardSerial(mfrcInstance)) {
				LedOn(LED_1);
				userTapIn();
				LedOff(LED_1);
			}
		}
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}
		*/
}
/*==================[end of file]============================================*/
