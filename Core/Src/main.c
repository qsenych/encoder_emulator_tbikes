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
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "atan_lut.h"
#include "math.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SIN_PERIODS_PER_REV 1
#define HALL_PERIODS_PER_REV 20
#define OFFSET 0
#define LUT_SIZE 1024
#define ADC_MAX 3479.0
#define ADC_MIN 1850.0

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static float fast_atan(uint16_t ADC_C, uint16_t ADC_S);
void HallPositionOutput(int theta_deg);
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
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

  uint16_t sin_adc = 0;
  uint16_t cos_adc = 0;
  float theta_rad;
  int theta_deg; // only for debugging
  int historical_value = 0; // Initialize historical value for decay and merge
  int merged_value = 0; // Initialize merged value



  int sin;
  int cos; 
  int rad; 
  int deg;
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
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    HAL_ADC_Start(&hadc1); // Needs to be called every time
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    cos_adc = decayAndMerge(cos_adc, HAL_ADC_GetValue(&hadc1)); // Decay and merge the new value with the historical value
    
    HAL_ADC_Start(&hadc2); // Needs to be called every time
    HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);
    sin_adc = decayAndMerge(sin_adc, HAL_ADC_GetValue(&hadc2)); // Decay and merge the new value with the historical value

    // cos_adc = 1000; 
    // sin_adc = 3000; // Test values for debugging
    
    // theta_rad = fast_atan(cos_adc, sin_adc);
    const float midpoint = (ADC_MAX + ADC_MIN) / 2.0f;
    float num = cos_adc - midpoint;
    float den = sin_adc - midpoint;
    if (den != 0){
      theta_rad = atan2f(num, den); // Use atan2 for better handling of quadrants
    }
    else {
      theta_rad = 0; // Handle the case where den is zero to avoid division by zero
    }
      

   
    theta_deg = (int)(180.0 / M_PI * theta_rad)+180;

    // if (cos_adc > 180) {
    //   HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_SET);
    // } else {
    //   HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_RESET);
    // }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    rad = (int)(theta_rad * 1000); // Convert to milliradians
    printf("ADC_C: %d ADC_S: %d theta_deg: %d rad: %d (millirad)\r\n", cos_adc, sin_adc, theta_deg, rad);

    HallPositionOutput_10Pair(theta_deg); // Call the function to output the Hall position
    HAL_Delay(10); 
    
  
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

static float fast_atan(uint16_t ADC_C, uint16_t ADC_S) {
  const float midpoint = (ADC_MAX + ADC_MIN) / 2.0f;
  float num = ADC_C - midpoint;
  float den = ADC_S - midpoint;
  if (den == 0)
    return (num >= 0) ? 0 : M_PI;
  float r = fabs(num / den);
  int index = (int)(r * 1023.0f);
  float theta = ATAN_LUT[index];
  float next_theta = ATAN_LUT[index + 1];
  theta += (r - (index / 1023.0f)) * (next_theta - theta);
  if (num > den)
    theta = M_PI_2 - theta;
  if (num < 0 && den > 0)
    return M_PI + theta;
  if (den < 0)
    return M_PI - theta;
  return theta;
}

int decayAndMerge(int historical_value, int new_value){
  if(historical_value == 0){
    historical_value = new_value; // Initialize historical value if it's zero
  }
  float decay_rate = 0.9; 

  return (int)(historical_value * (1.0-decay_rate) + new_value * decay_rate);

  // // Decay the historical value by a factor of 0.9
  // historical_value = (int)(historical_value * 0.9 + new_value * 0.1);
  // // Merge the new value with the decayed historical value
  // int merged_value = (int)(historical_value * 0.5 + new_value * 0.5);
  // return merged_value;
}

/**
 * @brief  This function is used to output the Hall position. For a single pole pair? 
 * @param  theta_deg: The angle in degrees.
 * @retval None
 * @author Alex
 */
void HallPositionOutput(theta_deg){
  if(theta_deg >= 0 && theta_deg < 60) {
    //HALL A = 1, HALL_B = 0, HALL_C = 1
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_SET);

    printf("HALL A = 1, HALL B = 0, HALL C = 1\r\n");
  } else if (theta_deg >= 60 && theta_deg < 120){
    //HALL A = 1, HALL_B = 0, HALL_C = 0
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_RESET);

    printf("HALL A = 1, HALL B = 0, HALL C = 0\r\n");
  } else if (theta_deg >= 120 && theta_deg < 180){
    //HALL A = 1, HALL_B = 1, HALL_C = 0
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_RESET);

    printf("HALL A = 1, HALL B = 1, HALL C = 0\r\n");
  } else if (theta_deg >= 180 && theta_deg < 240){
    //HALL A = 0, HALL_B = 1, HALL_C = 0
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_RESET);

    printf("HALL A = 0, HALL B = 1, HALL C = 0\r\n");
  } else if (theta_deg >= 240 && theta_deg < 300){
    //HALL A = 0, HALL_B = 1, HALL_C = 1
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_SET);

    printf("HALL A = 0, HALL B = 1, HALL C = 1\r\n");
  } else if (theta_deg >= 300 && theta_deg < 360){
    //HALL A = 0, HALL_B = 0, HALL_C = 1
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_SET);

    printf("HALL A = 0, HALL B = 0, HALL C = 1\r\n");
  } else {
    // Invalid angle
    printf("Invalid angle: %d\r\n", theta_deg);
  }

}

/**
 * @brief  This function is used to output the Hall position for a 10-pair motor. 
 * @param  theta_deg: The angle in degrees.
 * @retval None
 * @author Alex
 */
void HallPositionOutput_10Pair(theta_deg){
  // Normalize the angle to a single electrical cycle (0-36 degrees)
  int theta_elec = theta_deg % 36;

  if (theta_elec >= 0 && theta_elec < 6) {
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_SET);
    printf("HALL A = 1, HALL B = 0, HALL C = 1\r\n");
  } else if (theta_elec >= 6 && theta_elec < 12) {
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_RESET);
    printf("HALL A = 1, HALL B = 0, HALL C = 0\r\n");
  } else if (theta_elec >= 12 && theta_elec < 18) {
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_RESET);
    printf("HALL A = 1, HALL B = 1, HALL C = 0\r\n");
  } else if (theta_elec >= 18 && theta_elec < 24) {
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_RESET);
    printf("HALL A = 0, HALL B = 1, HALL C = 0\r\n");
  } else if (theta_elec >= 24 && theta_elec < 30) {
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_SET);
    printf("HALL A = 0, HALL B = 1, HALL C = 1\r\n");
  } else if (theta_elec >= 30 && theta_elec < 36) {
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_SET);
    printf("HALL A = 0, HALL B = 0, HALL C = 1\r\n");
  } else {
    printf("Invalid angle: %d\r\n", theta_deg);
  }

}

/**
 * @brief  This function is used to output the Hall position for a 20-pair motor. 
 * @param  theta_deg: The angle in degrees.
 * @retval None
 * @author Alex
 */
void HallPositionOutput_20Pair(theta_deg){
  // Normalize the angle to a single electrical cycle (0-18 degrees)
  int theta_elec = theta_deg % 18;

  if (theta_elec >= 0 && theta_elec < 3) {
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_SET);
    printf("HALL A = 1, HALL B = 0, HALL C = 1\r\n");
  } else if (theta_elec >= 3 && theta_elec < 6) {
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_RESET);
    printf("HALL A = 1, HALL B = 0, HALL C = 0\r\n");
  } else if (theta_elec >= 6 && theta_elec < 9) {
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_RESET);
    printf("HALL A = 1, HALL B = 1, HALL C = 0\r\n");
  } else if (theta_elec >= 9 && theta_elec < 12) {
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_RESET);
    printf("HALL A = 0, HALL B = 1, HALL C = 0\r\n");
  } else if (theta_elec >= 12 && theta_elec < 15) {
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_SET);
    printf("HALL A = 0, HALL B = 1, HALL C = 1\r\n");
  } else if (theta_elec >= 15 && theta_elec < 18) {
    HAL_GPIO_WritePin(HALL_A_GPIO_Port, HALL_A_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_B_GPIO_Port, HALL_B_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HALL_C_GPIO_Port, HALL_C_Pin, GPIO_PIN_SET);
    printf("HALL A = 0, HALL B = 0, HALL C = 1\r\n");
  } else {
    printf("Invalid angle: %d\r\n", theta_deg);
  }

}

/**
 * @brief  Retargets the C library printf function to the USART.
 * @retval None
 */
PUTCHAR_PROTOTYPE {
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
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
  while (1) {
    printf("Error\r\n");
    HAL_Delay(100);
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
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
