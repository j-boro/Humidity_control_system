/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "eth.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c-lcd.h"
#include "humidifier_control.h"
#include <stdio.h>
#include <string.h>
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

/* USER CODE BEGIN PV */
float humidity = 34.56f, set = 70.0;
int state = 0;
char buff[16];
_Bool tryingFor = 0, fanState = 0, uncert = 0, humid_state;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
float readHumidity(void) {
  uint8_t command[2] = {0xFD}; // Measurement command for high precision
  uint8_t data[6];
  HAL_StatusTypeDef ret;

  // Send measurement command
  ret = HAL_I2C_Master_Transmit(&hi2c1, 0x44 << 1, command, 1, 1000);
  if (ret != HAL_OK) {
    // Handle error
    return -1;
  }

  // Wait for measurement to complete (refer to datasheet for timing)
  HAL_Delay(10);

  // Read measurement data
  ret = HAL_I2C_Master_Receive(&hi2c1, 0x44 << 1, data, 6, 1000);
  if (ret != HAL_OK) {
    // Handle error
    return -1;
  }

  // Convert raw data to humidity value
  uint16_t rawHumidity = (data[3] << 8) | data[4];
  float humidity = -6.0 + 125.0 * (float)rawHumidity / 65535.0;

  return humidity;
}

void uncertain(float humidity, float lowerThresh, float upperThresh){
	if(humidity < lowerThresh || humidity > upperThresh){
		uncert = 0;
	}
}

void main_control_loop(float humidity, float lowerThresh, float upperThresh){

	if(uncert == 0){
		if(humidity < lowerThresh){
			tryingFor = 1;
		} else if(humidity > upperThresh){
			tryingFor = 0;
		} else if(humidity >= lowerThresh && humidity <= upperThresh && tryingFor == 1){
			uncert = 1;
			tryingFor = 0;
		} else if(humidity >= lowerThresh && humidity <= upperThresh && tryingFor == 0){
			uncert = 1;
			tryingFor = 1;
		}
	} else if(uncert == 1)
		uncertain(humidity, lowerThresh, upperThresh);

}

void fan_control(float humidity, float lowerThresh, float upperThresh){
	if(humidity > upperThresh){
		fanState = 1;
	} else if(humidity < lowerThresh){
		fanState = 0;
	}
}

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
  MX_ETH_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_1);
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_2);
  __HAL_TIM_SET_COUNTER(&htim4, 480);

  lcd_init(&hi2c2);
  lcd_clear(&hi2c2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  humidity = readHumidity();

	  if(__HAL_TIM_GET_COUNTER(&htim4) >= 600)
		  __HAL_TIM_SET_COUNTER(&htim4, 600);
	  else if(__HAL_TIM_GET_COUNTER(&htim4) <= 200)
		  __HAL_TIM_SET_COUNTER(&htim4, 200);

	  set = (__HAL_TIM_GET_COUNTER(&htim4) - 200) / 4.0;

	  //Humidifier control
	  main_control_loop(humidity, set * 0.942, set * 1.028);

	  if(tryingFor == 1){
		  humidifier_on();
	  } else if(tryingFor == 0){
		  humidifier_off();
	  }

      char buffer[50];
      sprintf(buffer, "%.2f,%.2f,%i,%i\n", humidity, set, fanState, humid_state); // Format data as CSV
      HAL_UART_Transmit(&huart3, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY); // Use your UART handle here (e.g., &huart2)

      HAL_Delay(500);

	  //Fan control
	  fan_control(humidity, set * 1.028, set * 1.03);
	  HAL_GPIO_WritePin(FAN_GPIO_Port, FAN_Pin, fanState);

	  lcd_clear(&hi2c2);
	  lcd_display_two_floats(&hi2c2, "Set: %.2f%%", "Read: %.2f%%", set, humidity);
	  HAL_Delay(500);
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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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
