/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fdcan.h"
#include "i2c.h"
#include "icache.h"
#include "rng.h"
#include "usart.h"
#include "usb.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mustang.h"
#include "ssd1306.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef union
{
	uint32_t uint32;
	uint8_t uint8[4];
} uint32to8_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAN_MSG_REPEAT_PERIOD 100
#define CAN_MSG_TIME_INTERVAL 5
#define RANDOM_MSG_TIME_INTERVAL 1000
#define ENGINE_SPEED_MAX 8000 // rpm
#define ENGINE_SPEED_INCREMENT 370
#define ENGINE_SPEED_DECREMENT 680
#define VEHICLE_SPEED_MAX 160 // mph
#define VEHICLE_SPEED_INCREMENT 1
#define ENGINE_SPEED_GEAR_SHIFT 3000 // rpm
#define COOLANT_TEMPERATURE 90 // degC
#define OIL_TEMPERATURE 110 // degC
#define TRANS_OIL_TEMP_INIT 10 // degC
#define TRANS_OIL_TEMP_FINAL 140 // degC

#define DELAYED_START_INTERVAL 4000
#define TOGGLE_TIME_INTERVAL 1000

#define ACCELEROMETER_TIME_INTERVAL 20

#define SEND_ALL_POSSIBLE_IDS 0
#define PATTERN_TO_BE_SEND  0x55 // 0b00011000 // 0xAA or 0x55
#define STOP_MSG_SCANNING
#define STOP_RANDOM_MSG

#define BYTE_TO_BINARY_PATTERN "0b%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0')

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
FDCAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];

FDCAN_TxHeaderTypeDef TxHeader;
uint8_t TxData8[8];
uint8_t TxData8Random[8];
uint8_t Random8Bytes[8];
uint8_t TxData7[7];
uint8_t TxData6[6];
uint8_t TxData5[5];
uint32_t TxMailbox;

uint16_t vehicle_speed = 0;
double vehicle_acceleration_y;
double vehicle_accel_x;
double vehicle_accel_y;
uint16_t engine_speed = 0;
uint8_t trans_oil_temp = TRANS_OIL_TEMP_INIT;
uint8_t gear_number;
volatile uint8_t christmas_lights = 1;

uint8_t engine_speed_down;

uint32_t CanMsgSoftTimer;
uint32_t RandomMsgSoftTimer;
uint32_t DelayedStartSoftTimer;
uint32_t ToggleSoftTimer;
uint32_t OdometerSoftTimer;
uint32_t OledSoftTimer;
uint32_t AccelerometerSoftTimer;

volatile uint8_t rng_number_flag = 0;
uint8_t rng_number_count = 0;
uint8_t rng_message_flag = 0;
uint32_t rng_number;

uint8_t toggle_variable = 0;
uint8_t flip_me_variable = 0xAA;  // odometer

volatile uint8_t up_button_flag = 0;
volatile uint8_t down_button_flag = 0;
volatile uint8_t left_button_flag = 0;
volatile uint8_t right_button_flag = 0;
volatile uint8_t mid_button_flag = 0;
volatile uint8_t set_button_flag = 0;
volatile uint8_t reset_button_flag = 0;

uint32to8_t uint32to8_converter;

uint8_t uart_message[128];
uint16_t message_size;

uint16_t msg_id = 0;

char lcd_line[128];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void PrintCanFrame0bToUart(void);
void PrintCanFrame0xToUart(void);
void PrintJustSentCanFrame0xToUart(void);
void CalculateRpmSpeed(void);
void CalculateTransOilTemp(void);
void MarkCanFrame(void);
void ComposeRandomMessage(void);
void EmulateIncreasingMileage(void);
void DisplayUnits(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_ICACHE_Init();
	MX_USART1_UART_Init();
	MX_USART3_UART_Init();
	MX_USB_PCD_Init();
	MX_FDCAN1_Init();
	MX_RNG_Init();
	MX_I2C1_Init();
	/* USER CODE BEGIN 2 */

	ssd1306_Init();
	ssd1306_Fill(Black);
	ssd1306_SetCursor(20, 0);
	ssd1306_WriteString("ufnalski.edu.pl", Font_6x8, White);
	ssd1306_SetCursor(14, 12);
	ssd1306_WriteString("Ford Mustang 2016", Font_6x8, White);
	ssd1306_SetCursor(20, 24);
	ssd1306_WriteString("IPC CAN hacking", Font_6x8, White);
	ssd1306_UpdateScreen();

	HAL_RNG_GenerateRandomNumber_IT(&hrng);

	HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT,
	FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);

	if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
	{
		Error_Handler();
	}

	if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
			0) != HAL_OK)
	{
		/* Notification Error */
		Error_Handler();
	}

	TxHeader.IdType = FDCAN_STANDARD_ID;
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeader.BitRateSwitch = FDCAN_BRS_ON;
	TxHeader.FDFormat = FDCAN_FD_CAN;
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader.MessageMarker = 0;

	CanMsgSoftTimer = HAL_GetTick();
	RandomMsgSoftTimer = HAL_GetTick();
	DelayedStartSoftTimer = HAL_GetTick();
	ToggleSoftTimer = HAL_GetTick();
	OdometerSoftTimer = HAL_GetTick();
	OledSoftTimer = HAL_GetTick();
	AccelerometerSoftTimer = HAL_GetTick();
	memset(RxData, 0xFF, sizeof(RxData));

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{

		if ((HAL_GetTick() - AccelerometerSoftTimer)
				> ACCELEROMETER_TIME_INTERVAL)
		{
			AccelerometerSoftTimer = HAL_GetTick();
			vehicle_accel_x = 75;  // some exemplary hard-coded value
			vehicle_accel_y = 50;  // some exemplary hard-coded value
			sprintf(lcd_line, "A_X=%i,  A_Y=%i         ",
					(int) round(vehicle_accel_x), (int) round(vehicle_accel_y));
			ssd1306_SetCursor(2, 36);
			ssd1306_WriteString(lcd_line, Font_6x8, White);
			ssd1306_UpdateScreen();
		}

		if (HAL_GetTick() - ToggleSoftTimer > TOGGLE_TIME_INTERVAL)
		{
			ToggleSoftTimer = HAL_GetTick();
			toggle_variable ^= 1;
		}

#if SEND_ALL_POSSIBLE_IDS
		if ((HAL_GetTick() - CanMsgSoftTimer > CAN_MSG_REPEAT_PERIOD)
				&& (msg_id <= 0x7FF))
		{
			CanMsgSoftTimer = HAL_GetTick();

			if (HAL_GetTick() - DelayedStartSoftTimer > DELAYED_START_INTERVAL)
			{
				CalculateRpmSpeed();
				if (trans_oil_temp < TRANS_OIL_TEMP_FINAL)
				{
					trans_oil_temp = TRANS_OIL_TEMP_INIT + msg_id / 2;
				}
			}

			activate_cluster(0, 0, CAN_MSG_TIME_INTERVAL);
			send_engine_speed(engine_speed, CAN_MSG_TIME_INTERVAL);
			send_ground_speed(vehicle_speed, CAN_MSG_TIME_INTERVAL);
			send_coolant_and_oil_temp(COOLANT_TEMPERATURE, OIL_TEMPERATURE,
			CAN_MSG_TIME_INTERVAL);
			send_tyre_pressure(220, 225, 230, 235, CAN_MSG_TIME_INTERVAL);
			send_transmission_oil_temp(trans_oil_temp, CAN_MSG_TIME_INTERVAL);
			//send_launch_control(CAN_MSG_TIME_INTERVAL);
			//send_seatbelt(CAN_MSG_TIME_INTERVAL);
			send_inlet_air_temp(100, CAN_MSG_TIME_INTERVAL);
			send_cylinder_head_temp(150, 2 * 750, 15, CAN_MSG_TIME_INTERVAL);
			send_gearbox_mode('S', 5, CAN_MSG_TIME_INTERVAL);
			send_oil_pressure(0, CAN_MSG_TIME_INTERVAL);

			HAL_Delay(CAN_MSG_TIME_INTERVAL);
			TxHeader.DataLength = FDCAN_DLC_BYTES_8;
			TxHeader.Identifier = msg_id;
			memset(TxData8, PATTERN_TO_BE_SEND, sizeof(TxData8));
#ifndef STOP_MSG_SCANNING
			if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData8)
					!= HAL_OK)
			{
				Error_Handler();
			}
#endif
			msg_id++;

			EmulateIncreasingMileage();

			navigate_menu(CAN_MSG_TIME_INTERVAL);
			HAL_GPIO_TogglePin(USER_LED_GPIO_Port,
			USER_LED_Pin);
		}

#else

		ComposeRandomMessage();

		if (HAL_GetTick() - CanMsgSoftTimer > CAN_MSG_TIME_INTERVAL)
		{
			CanMsgSoftTimer = HAL_GetTick();

			if (HAL_GetTick() - DelayedStartSoftTimer > DELAYED_START_INTERVAL)
			{
				CalculateRpmSpeed();
				CalculateTransOilTemp();
			}

			if ((HAL_GetTick() - RandomMsgSoftTimer > RANDOM_MSG_TIME_INTERVAL)
					&& (rng_message_flag == 1))
			{
				RandomMsgSoftTimer = HAL_GetTick();
				rng_message_flag = 0;
				HAL_RNG_GenerateRandomNumber_IT(&hrng);
				memcpy(TxData8Random, Random8Bytes, sizeof(TxData8Random));
				TxData8Random[0] = 0;
				TxData8Random[1] = 0;
				TxData8Random[2] = 0;
				TxData8Random[3] = 0;
				TxData8Random[4] = 0;
				TxData8Random[5] = 0;
				TxData8Random[6] = 0;
				TxData8Random[7] = 0;
				//PrintCanFrame0bToUart();
				PrintCanFrame0xToUart();
			}
			MarkCanFrame();

			HAL_Delay(CAN_MSG_TIME_INTERVAL);
			TxHeader.DataLength = FDCAN_DLC_BYTES_8;
			TxHeader.Identifier = 0x3C3;
			memcpy(TxData8, TxData8Random, sizeof(TxData8));
#ifndef STOP_RANDOM_MSG
			if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData8)
					!= HAL_OK)
			{
				Error_Handler();
			}
#endif

			activate_cluster(toggle_variable, toggle_variable,
			CAN_MSG_TIME_INTERVAL);
			send_engine_speed(engine_speed, CAN_MSG_TIME_INTERVAL);
			send_ground_speed(vehicle_speed, CAN_MSG_TIME_INTERVAL);
			send_coolant_and_oil_temp(COOLANT_TEMPERATURE, OIL_TEMPERATURE,
			CAN_MSG_TIME_INTERVAL);
			send_tyre_pressure(220, 225, 230, 235, CAN_MSG_TIME_INTERVAL);
			send_transmission_oil_temp(trans_oil_temp, CAN_MSG_TIME_INTERVAL);
			//send_launch_control(CAN_MSG_TIME_INTERVAL);
			//send_seatbelt(CAN_MSG_TIME_INTERVAL);
			send_inlet_air_temp(100, CAN_MSG_TIME_INTERVAL);
			send_cylinder_head_temp(150, -500, 15, CAN_MSG_TIME_INTERVAL); // 2 * 750 = 2 bars boost
			send_gearbox_mode('S', 5, CAN_MSG_TIME_INTERVAL);
			send_oil_pressure(0, CAN_MSG_TIME_INTERVAL);

			EmulateIncreasingMileage();
			send_accelerometer(-vehicle_accel_x / 20.0, -vehicle_accel_y / 20.0,
			CAN_MSG_TIME_INTERVAL);

			navigate_menu(10);
			//		PrintJustSentCanFrame0xToUart();
			DisplayUnits();
			HAL_GPIO_TogglePin(USER_LED_GPIO_Port,
			USER_LED_Pin);
		}

#endif
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct =
	{ 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct =
	{ 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

	while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
	{
	}

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48
			| RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLL1_SOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 12;
	RCC_OscInitStruct.PLL.PLLN = 250;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1_VCIRANGE_1;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1_VCORANGE_WIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK3;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure the programming delay
	 */
	__HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_2);
}

/* USER CODE BEGIN 4 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
	{
		/* Retreive Rx messages from RX FIFO0 */
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData)
				!= HAL_OK)
		{
			/* Reception Error */
			Error_Handler();
		}

		if (HAL_FDCAN_ActivateNotification(hfdcan,
		FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
		{
			/* Notification Error */
			Error_Handler();
		}
	}
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == B1_BLUE_USER_BUTTON_Pin)
	{
		christmas_lights ^= 1; // toggle
	}
}

//void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == UP_BUTTON_Pin)
	{
		up_button_flag = 1;
	}

	if (GPIO_Pin == DOWN_BUTTON_Pin)
	{
		down_button_flag = 1;
	}

	if (GPIO_Pin == LEFT_BUTTON_Pin)
	{
		left_button_flag = 1;
	}

	if (GPIO_Pin == RIGHT_BUTTON_Pin)
	{
		right_button_flag = 1;
	}

	if (GPIO_Pin == MID_BUTTON_Pin)
	{
		mid_button_flag = 1;
	}

	if (GPIO_Pin == SET_BUTTON_Pin)
	{
		set_button_flag = 1;
	}

	if (GPIO_Pin == RESET_BUTTON_Pin)
	{
		reset_button_flag = 1;
	}

}

void HAL_RNG_ReadyDataCallback(RNG_HandleTypeDef *hrng, uint32_t random32bit)
{
	rng_number_flag = 1;
}

void PrintCanFrame0bToUart(void)
{
	message_size =
			sprintf((char*) uart_message,
					"[0x%02X] "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN"\r\n",
					(unsigned int) TxHeader.Identifier,
					BYTE_TO_BINARY(TxData8Random[0]),
					BYTE_TO_BINARY(TxData8Random[1]),
					BYTE_TO_BINARY(TxData8Random[2]),
					BYTE_TO_BINARY(TxData8Random[3]),
					BYTE_TO_BINARY(TxData8Random[4]),
					BYTE_TO_BINARY(TxData8Random[5]),
					BYTE_TO_BINARY(TxData8Random[6]),
					BYTE_TO_BINARY(TxData8Random[7]));
	HAL_UART_Transmit(&huart3, (uint8_t*) uart_message, message_size, 200);
}

void PrintCanFrame0xToUart(void)
{
	message_size =
			sprintf((char*) uart_message,
					"[0x%02X] 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n",
					(unsigned int) TxHeader.Identifier, TxData8Random[0],
					TxData8Random[1], TxData8Random[2], TxData8Random[3],
					TxData8Random[4], TxData8Random[5], TxData8Random[6],
					TxData8Random[7]);
	HAL_UART_Transmit(&huart3, (uint8_t*) uart_message, message_size, 200);
}

void PrintJustSentCanFrame0xToUart(void)
{
	message_size =
			sprintf((char*) uart_message,
					"[0x%02X] 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n",
					(unsigned int) TxHeader.Identifier, TxData8[0], TxData8[1],
					TxData8[2], TxData8[3], TxData8[4], TxData8[5], TxData8[6],
					TxData8[7]);
	HAL_UART_Transmit(&huart3, (uint8_t*) uart_message, message_size, 200);
}

void MarkCanFrame(void)
{
	if (set_button_flag == 1)
	{
		set_button_flag = 0;
		message_size = sprintf((char*) uart_message, "--> red check mark\r\n");
		HAL_UART_Transmit(&huart3, (uint8_t*) uart_message, message_size, 200);
	}

	if (reset_button_flag == 1)
	{
		reset_button_flag = 0;
		message_size = sprintf((char*) uart_message,
				"--> white check mark\r\n");
		HAL_UART_Transmit(&huart3, (uint8_t*) uart_message, message_size, 200);
	}

	if (mid_button_flag == 1)
	{
		mid_button_flag = 0;
		message_size = sprintf((char*) uart_message, "--> mid check mark\r\n");
		HAL_UART_Transmit(&huart3, (uint8_t*) uart_message, message_size, 200);
	}
}

void CalculateRpmSpeed(void)
{
	if (!engine_speed_down)
	{
		if (engine_speed <= ENGINE_SPEED_MAX - ENGINE_SPEED_INCREMENT)
		{
			engine_speed = engine_speed + ENGINE_SPEED_INCREMENT;
			vehicle_acceleration_y = 1.4;
		}
		else if (gear_number < 5)
		{
			engine_speed_down = 1;
			gear_number++;
		}
		else
		{
			vehicle_acceleration_y = 0.0;
		}
	}
	else
	{
		if (engine_speed > ENGINE_SPEED_GEAR_SHIFT)
		{
			engine_speed = engine_speed - ENGINE_SPEED_DECREMENT;
			vehicle_acceleration_y = 0.0;
		}
		else
		{
			engine_speed_down = 0;
		}
	}

	if (vehicle_speed <= VEHICLE_SPEED_MAX - VEHICLE_SPEED_INCREMENT)
	{
		vehicle_speed = vehicle_speed + VEHICLE_SPEED_INCREMENT;
	}
}

void CalculateTransOilTemp(void)
{
	if (trans_oil_temp < TRANS_OIL_TEMP_FINAL)
	{
		trans_oil_temp++;
	}
}

void ComposeRandomMessage(void)
{
	if ((rng_number_flag == 1) && (rng_message_flag == 0)
			&& (rng_number_count < 2))
	{
		uint32to8_converter.uint32 = HAL_RNG_ReadLastRandomNumber(&hrng);
		rng_number_flag = 0;
		for (uint8_t i = 0; i < 4; i++)
		{
			Random8Bytes[i + 4 * rng_number_count] =
					uint32to8_converter.uint8[i];
		}
		rng_number_count++;
		HAL_RNG_GenerateRandomNumber_IT(&hrng);
		if (rng_number_count == 2)
		{
			rng_message_flag = 1;
			rng_number_count = 0;
		}
	}
}

void EmulateIncreasingMileage(void)
{
	if (HAL_GetTick() - OdometerSoftTimer > 2000) // 50 km/h
	{
		OdometerSoftTimer = HAL_GetTick();
		flip_me_variable ^= 0xFF;
		send_add_25m_to_odometer(flip_me_variable,
		CAN_MSG_TIME_INTERVAL);
	}
}

void DisplayUnits(void)
{
	if (HAL_GetTick() - OledSoftTimer > 1000)
	{
		OledSoftTimer = HAL_GetTick();

		if ((RxData[2] & 0xF0) == 0x00)
		{
			sprintf(lcd_line, "Unt: meters + deg. C  ");
		}
		else if ((RxData[2] & 0xF0) == 0x40)
		{
			sprintf(lcd_line, "Unt: meters + deg. F  ");
		}
		else if ((RxData[2] & 0xF0) == 0x80)
		{
			sprintf(lcd_line, "Unt: miles + deg. C   ");
		}
		else if ((RxData[2] & 0xF0) == 0xC0)
		{
			sprintf(lcd_line, "Unt: miles + deg. F   ");
		}
		else
		{
			sprintf(lcd_line, "Sth went wrong!   ");
		}
		ssd1306_SetCursor(2, 48);
		ssd1306_WriteString(lcd_line, Font_6x8, White);
		ssd1306_UpdateScreen();
	}
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
