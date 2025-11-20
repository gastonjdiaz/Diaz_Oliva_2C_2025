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
 * | 19/11/2025 | Presentation			                         |
 * | 20/11/2025 | Documentation			                         |
 *
 * @author Gastón Jair Díaz (diazgastonj@gmail.com)
 * @author Juan Ignacio Oliva (juaaaaanioliiiva@gmail.com)
 * 
 * @note Led y motor conectado a PWM con salidas de 5v de la placa
 * @note PWM de led puesto a 100Hz, con 50 se veía el titileo
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
/**
 * @brief Lista constantes de configuración del proyecto
 */
#define DISTANCE_CLOSE  8	   ///<  distancia en cm para modo de trabajo intenso 
#define DISTANCE_MEDIUM  16   ///<  distancia en cm para modo de trabajo moderado 
#define DISTANCE_FAR  24	   ///<  distancia en cm para modo de reposo 
#define DISTANCE_OFF  30	   ///<  distancia en cm para modo apagado 
#define DUTY_CYCLE_MAX  100   ///<  ciclo de trabajo maximo  
#define DUTY_CYCLE_MEDIUM  60 ///<  ciclo de trabajo medio medio 
#define DUTY_CYCLE_LOW  30	   ///<  ciclo de trabajo bajo 
#define DUTY_CYCLE_OFF  0	   ///<  ciclo de trabajo apagado 
#define APERTURE_WIDE_OPEN -90   ///<  apertura de iris maxima 
#define APERTURE_MEDIUM_OPEN 0 ///<  apertura de iris media 
#define APERTURE_NARROW_OPEN 90  ///<  apertura de iris minima 

#define CONFIG_OPERATION_CYCLE_MS 500 ///<  Tiempo de período de operación en milisegundos 
#define CONFIG_MEASUREMENT_CYCLE_MS 200 ///<  Tiempo de perído de medición en milisegundos 
/*==================[internal data definition]===============================*/
uint16_t _distancia;
/**
 * \def _medicionActivada
 * \todo Variable para activar o desactivar modo manual de medición
 */
bool _medicionActivada = true;

TaskHandle_t medir_task_handle = NULL;
TaskHandle_t PotenciaLuz_task_handle = NULL;
TaskHandle_t AperturaHaz_task_handle = NULL;
TaskHandle_t Uart_task_handle = NULL;


/**
 * @brief Mide distancia con HC-SR04 y actualiza variable global 
 */
static void Medir(void *pvParameter)
{
	while (true)
	{
		
	
		_distancia = HcSr04ReadDistanceInCentimeters(); // Realiza la medición de distancia
		
		vTaskDelay(pdMS_TO_TICKS(CONFIG_MEASUREMENT_CYCLE_MS));
	}
}
/**
 * @brief Envía datos de potencia y distancia por UART
 */
void EnviarDatosUART()
{
	while (true)
	{
		UartSendString(UART_PC, "Potencia: ");
		if (_distancia < DISTANCE_CLOSE) 
			UartSendString(UART_PC, (char*)UartItoa(DUTY_CYCLE_MAX, 10));
		else if (_distancia < DISTANCE_MEDIUM)
			UartSendString(UART_PC, (char*)UartItoa(DUTY_CYCLE_MEDIUM, 10));
		else if (_distancia < DISTANCE_FAR)
			UartSendString(UART_PC, (char*)UartItoa(DUTY_CYCLE_LOW, 10));
		else if	(_distancia > DISTANCE_OFF) 
			UartSendString(UART_PC, (char*)UartItoa(DUTY_CYCLE_OFF, 10));

		UartSendString(UART_PC, "\r\n Distancia: ");
		UartSendString(UART_PC, (char*)UartItoa(_distancia, 10));
		UartSendString(UART_PC, "cm\r\n");

		vTaskDelay(pdMS_TO_TICKS(CONFIG_OPERATION_CYCLE_MS));
	}
}

/**
 * @brief Regula la intensidad de la luz con PWM dependiendo de la distancia medida
 */
void Regular_intensidad_luz()
{
 while (true)
 {
		if (_distancia < DISTANCE_CLOSE) 
			PWMSetDutyCycle(PWM_2, DUTY_CYCLE_MAX);
		else if (_distancia < DISTANCE_MEDIUM)
			PWMSetDutyCycle(PWM_2, DUTY_CYCLE_MEDIUM);
		else if (_distancia < DISTANCE_FAR)
			PWMSetDutyCycle(PWM_2, DUTY_CYCLE_LOW);
		else if	(_distancia > DISTANCE_OFF) 
			PWMSetDutyCycle(PWM_2, DUTY_CYCLE_OFF);
	
		//PWMSetDutyCycle(PWM_2, DUTY_CYCLE_OFF);
	
		vTaskDelay(pdMS_TO_TICKS(CONFIG_OPERATION_CYCLE_MS));
 }
}
/**
 * @brief Regula apartura del iris dependiendo de la distancia medida
 */
void Regular_apertura_haz()
{
	while (true)
	{
		if (_distancia < DISTANCE_CLOSE) 
			ServoMove(SERVO_1, APERTURE_NARROW_OPEN);
		else if (_distancia < DISTANCE_MEDIUM)
			ServoMove(SERVO_1, APERTURE_MEDIUM_OPEN);
		else if (_distancia < DISTANCE_FAR)
			ServoMove(SERVO_1, APERTURE_WIDE_OPEN);
		else if	(_distancia > DISTANCE_OFF)	 
			ServoMove(SERVO_1, APERTURE_WIDE_OPEN);

		vTaskDelay(pdMS_TO_TICKS(CONFIG_OPERATION_CYCLE_MS));
	}
	
}
/**
 * @brief Función que acumula las otras pero sacada de uso
 * 
 * @deprecated Deprecado por no funionamiento (creemos que por los while sin el vTaskDelay)
 */
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
	PWMInit(PWM_2, GPIO_20, 100); // Inicializa PWM en GPIO_20 a 100Hz 
	HcSr04Init(GPIO_3, GPIO_2);
	
	xTaskCreate(Medir, "Medir", 2048, NULL, 5, &medir_task_handle);
	xTaskCreate(Regular_apertura_haz, "AperturaHaz", 4096, NULL, 5, &AperturaHaz_task_handle);
	xTaskCreate(Regular_intensidad_luz, "RegularLuz", 4096, NULL, 5, &PotenciaLuz_task_handle);
	xTaskCreate(EnviarDatosUART, "EnviarDatos", 4096, NULL, 5, &Uart_task_handle);	

}
/*==================[end of file]============================================*/


