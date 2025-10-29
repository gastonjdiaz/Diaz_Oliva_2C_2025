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
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 1000
/*==================[internal data definition]===============================*/
uint16_t _distancia;
bool _medicionActivada = false;

TaskHandle_t medir_task_handle;
/*==================[internal functions declaration]=========================*/
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);    /* Envía una notificación a la tarea Medir asociada al sensor */
	
}

static void Medir(void *pvParameter)
{
    while (true)
    {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
        if (_medicionActivada)
            _distancia = HcSr04ReadDistanceInCentimeters(); // funcion del ultrasonido que mide

    }
}

static void Regular_intensidad_luz(void *pvParameter){



}

static void Regular_apertura_haz(void *pvParameter){



}
/*==================[external functions definition]==========================*/
void app_main(void){
	
/* 	LedsInit();
	serial_config_t UART_USB;
	UART_USB.baud_rate = 115200;
	UART_USB.port = UART_PC;
	UartInit(&UART_USB);
	setupRFID(&mfrcInstance);

	UartSendString(UART_PC,"Init MRFC522 test.\r\n"); */
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
