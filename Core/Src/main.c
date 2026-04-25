/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
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
#include "logger.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;

I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemPower_Config(void);
static void MX_GPIO_Init(void);
static void MX_ICACHE_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
bool detect_fall(float, float, float);
void Accelerometer_Init();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the System Power */
	SystemPower_Config();

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_ICACHE_Init();
	MX_I2C1_Init();
	/* USER CODE BEGIN 2 */
	Accelerometer_Init();
	/* USER CODE END 2 */

	/* Initialize leds */
	BSP_LED_Init(LED_GREEN);
	BSP_LED_Init(LED_BLUE);
	BSP_LED_Init(LED_RED);

	/* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
	BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

	/* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
	BspCOMInit.BaudRate = 115200;
	BspCOMInit.WordLength = COM_WORDLENGTH_8B;
	BspCOMInit.StopBits = COM_STOPBITS_1;
	BspCOMInit.Parity = COM_PARITY_NONE;
	BspCOMInit.HwFlowCtl = COM_HWCONTROL_NONE;
	if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE) {
		Error_Handler();
	}

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	uint32_t last = 0;
	while (1) {

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

		uint8_t raw[6];

		HAL_I2C_Mem_Read(&hi2c1, IMU_ADDR, 0x28, I2C_MEMADD_SIZE_8BIT, raw, 6,
		HAL_MAX_DELAY);
		float ax_g = ((int16_t) (raw[1] << 8 | raw[0])) * 0.000061f;
		float ay_g = ((int16_t) (raw[3] << 8 | raw[2])) * 0.000061f;
		float az_g = ((int16_t) (raw[5] << 8 | raw[4])) * 0.000061f;

		log_info( "AX:%.2f AY:%.2f AZ:%.2f", ax_g, ay_g, az_g);

		HAL_Delay(200);

		//Adding the lowPass Filter
		static float ax_f = 0, ay_f = 0, az_f = 0;

		ax_f = FILTER_ALPHA * ax_g + (1 - FILTER_ALPHA) * ax_f;
		ay_f = FILTER_ALPHA * ay_g + (1 - FILTER_ALPHA) * ay_f;
		az_f = FILTER_ALPHA * az_g + (1 - FILTER_ALPHA) * az_f;

		if (detect_fall(ax_f, ay_f, az_f)) 
    {
		  log_info( "FALL DETECTED!");
		}

		if (BSP_PB_GetState(BUTTON_USER) == BUTTON_PRESSED) {
			if (HAL_GetTick() - last > 500) {
				HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_7);
				HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_2);
				last = HAL_GetTick();
			}
			BSP_LED_Off(LED_BLUE);
		} else {
			BSP_LED_Off(LED_GREEN);
			BSP_LED_Off(LED_RED);
			BSP_LED_On(LED_BLUE);
		}

	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE4)
			!= HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_4;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK3;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief Power Configuration
 * @retval None
 */
static void SystemPower_Config(void) {

	/*
	 * Disable the internal Pull-Up in Dead Battery pins of UCPD peripheral
	 */
	HAL_PWREx_DisableUCPDDeadBattery();

	/*
	 * Switch to SMPS regulator instead of LDO
	 */
	if (HAL_PWREx_ConfigSupply(PWR_SMPS_SUPPLY) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN PWR */
	/* USER CODE END PWR */
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x00400D10;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE)
			!= HAL_OK) {
		Error_Handler();
	}

	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief ICACHE Initialization Function
 * @param None
 * @retval None
 */
static void MX_ICACHE_Init(void) {

	/* USER CODE BEGIN ICACHE_Init 0 */

	/* USER CODE END ICACHE_Init 0 */

	/* USER CODE BEGIN ICACHE_Init 1 */

	/* USER CODE END ICACHE_Init 1 */

	/** Enable instruction cache in 1-way (direct mapped cache)
	 */
	if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_ICACHE_Enable() != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ICACHE_Init 2 */

	/* USER CODE END ICACHE_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void Accelerometer_Init()
{
  //Write to register CTRL1_XL:
	/*ODR_104HZ | FS_2G = 0x60 decoded:
	 *
	 * 0110 0000
	 * ^^^^ ---- → ODR(Output Data Rate) = 0110 = 416 Hz
	 * ---- ^^-- → FS(Full Scale) = 00 = ±2g
	 * ------ ^^ → BW(BandWidth) = default
	 */
	uint8_t config = ODR_416HZ | FS_2G;

	HAL_I2C_Mem_Write(&hi2c1, IMU_ADDR, CTRL1_XL,
	I2C_MEMADD_SIZE_8BIT, &config, 1, HAL_MAX_DELAY);

	uint8_t ctrl3 = 0x44;
	// 0x40 → BDU = 1
	// 0x04 → IF_INC = 1

	HAL_I2C_Mem_Write(&hi2c1, IMU_ADDR, 0x12,
	I2C_MEMADD_SIZE_8BIT, &ctrl3, 1, HAL_MAX_DELAY);
	HAL_Delay(100);
}


bool detect_fall(float ax, float ay, float az) {
	static int state = 0;
	static uint32_t t0 = 0;

	float mag = sqrtf(ax * ax + ay * ay + az * az);

	switch (state) {
	case 0: // Normal
		if (mag < FALL_FREEFALL_THRESHOLD)   // free fall
		{
			state = 1;
			t0 = HAL_GetTick();
		}
		break;

	case 1: // Falling
		if (mag > FALL_IMPACT_THRESHOLD)   // impact
		{
			state = 2;
			t0 = HAL_GetTick();
		} else if (HAL_GetTick() - t0 > FALL_FREEFALL_TIMEOUT_MS) {
			state = 0; // timeout
		}
		break;

	case 2: // Post-impact (check stillness)
		if (HAL_GetTick() - t0 > FALL_STILLNESS_TIME_MS) {
			state = 0;
			return true; // confirmed fall
		}
		break;
	}

	return false;
}
/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM17 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM17) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
