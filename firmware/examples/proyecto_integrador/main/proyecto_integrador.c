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
#include "servo_sg90.h"
/*==================[macros and definitions]=================================*/
#define DISTANCE_CLOSE  25	   // distancia en cm para modo de trabajo intenso
#define DISTANCE_MEDIUM  45   // distancia en cm para modo de trabajo moderado
#define DISTANCE_FAR  60	   // distancia en cm para modo de reposo
#define DISTANCE_OFF  100	   // distancia en cm para modo apagado
#define DUTY_CYCLE_MAX  100   // ciclo de trabajo maximo
#define DUTY_CYCLE_MEDIUM  60 // ciclo de trabajo medio medio
#define DUTY_CYCLE_LOW  30	   // ciclo de trabajo bajo
#define DUTY_CYCLE_OFF  0	   // ciclo de trabajo apagado
#define APERTURE_WIDE_OPEN -90   // apertura de iris maxima
#define APERTURE_MEDIUM_OPEN 0 // apertura de iris media
#define APERTURE_NARROW_OPEN 90  // apertura de iris minima

#define CONFIG_OPERATION_CYCLE_MS 500
#define CONFIG_MEASUREMENT_CYCLE_MS 200
/*==================[internal data definition]===============================*/
uint16_t _distancia;
bool _medicionActivada = false;

TaskHandle_t medir_task_handle = NULL;
TaskHandle_t ajustarEInformar_task_handle = NULL;


static void Medir(void *pvParameter)
{
	while (true)
	{
		
		if (_medicionActivada)
			_distancia = HcSr04ReadDistanceInCentimeters(); // Realiza la medición de distancia
		
		vTaskDelay(pdMS_TO_TICKS(CONFIG_MEASUREMENT_CYCLE_MS));
	}
}

static void EnviarDatosUART(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		UartSendString(UART_PC, "Potencia: .\r\n");
		if (_distancia < DISTANCE_CLOSE) 
			UartSendString(UART_PC, (char*)UartItoa(DUTY_CYCLE_MAX, 10));
		else if (_distancia < DISTANCE_MEDIUM)
			UartSendString(UART_PC, (char*)UartItoa(DUTY_CYCLE_MEDIUM, 10));
		else if (_distancia < DISTANCE_FAR)
			UartSendString(UART_PC, (char*)UartItoa(DUTY_CYCLE_LOW, 10));
		else if	(_distancia > DISTANCE_OFF) 
			UartSendString(UART_PC, (char*)UartItoa(DUTY_CYCLE_OFF, 10));

		UartSendString(UART_PC, (char*)UartItoa(_distancia, 10));
		// vTaskDelay(pdMS_TO_TICKS(CONFIG_OPERATION_CYCLE_MS));
	}
}

static void Regular_intensidad_luz(void *pvParameter)
{
	if (_medicionActivada)
	{
		if (_distancia < DISTANCE_CLOSE) 
			PWMSetDutyCycle(PWM_1, DUTY_CYCLE_MAX);
		else if (_distancia < DISTANCE_MEDIUM)
			PWMSetDutyCycle(PWM_1, DUTY_CYCLE_MEDIUM);
		else if (_distancia < DISTANCE_FAR)
			PWMSetDutyCycle(PWM_1, DUTY_CYCLE_LOW);
		else if	(_distancia > DISTANCE_OFF) 
			PWMSetDutyCycle(PWM_1, DUTY_CYCLE_OFF);
	}
	else
	{
		PWMSetDutyCycle(PWM_1, DUTY_CYCLE_OFF);
	}
	// vTaskDelay(pdMS_TO_TICKS(CONFIG_OPERATION_CYCLE_MS));
}
static void Regular_apertura_haz(void *pvParameter)
{
	if (_distancia < DISTANCE_CLOSE) 
		ServoMove(SERVO_1, APERTURE_NARROW_OPEN);
	else if (_distancia < DISTANCE_MEDIUM)
		ServoMove(SERVO_1, APERTURE_MEDIUM_OPEN);
	else if (_distancia < DISTANCE_FAR)
		ServoMove(SERVO_1, APERTURE_WIDE_OPEN);
	else if	(_distancia > DISTANCE_OFF)	 
		ServoMove(SERVO_1, APERTURE_WIDE_OPEN);

	//vTaskDelay(pdMS_TO_TICKS(CONFIG_OPERATION_CYCLE_MS));
}
static void AjustarEInformar(void *pvParameter)
{
	while (true)
	{
		Regular_intensidad_luz(NULL);
		Regular_apertura_haz(NULL);
		EnviarDatosUART(NULL);

		vTaskDelay(pdMS_TO_TICKS(CONFIG_OPERATION_CYCLE_MS));
	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{

	serial_config_t UART_USB;
	UART_USB.baud_rate = 115200;
	UART_USB.port = UART_PC;
	UartInit(&UART_USB);
	ServoInit(SERVO_1, GPIO_19); // Inicializa servo en GPIO19
	PWMInit(PWM_1, GPIO_1, 50); // Inicializa PWM en GPIO5 a 50Hz -> revisar frecuencia adecuada para la luz.


	xTaskCreate(Medir, "Medir", 2048, NULL, 5, &medir_task_handle);
	xTaskCreate(AjustarEInformar, "AjustarEInformar", 8192, NULL, 5, &ajustarEInformar_task_handle);

	// ServoMove(SERVO_1, APERTURE_WIDE_OPEN);    // Coloca servo en posicion inicial (0 grados)
	// vTaskDelay(pdMS_TO_TICKS(1000));
	// ServoMove(SERVO_1, APERTURE_MEDIUM_OPEN); 
	// vTaskDelay(pdMS_TO_TICKS(1000));
	// ServoMove(SERVO_1, APERTURE_NARROW_OPEN); 
	

}
/*==================[end of file]============================================*/


