/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LEDROW(x) HAL_GPIO_WritePin(LED##x##_GPIO_Port, LED##x##_Pin, GPIO_PIN_RESET)
#define LEDROW_WRAP(x) LEDROW(x)
#define LEDROWSTOP(x) HAL_GPIO_WritePin(LED##x##_GPIO_Port, LED##x##_Pin, GPIO_PIN_SET)
#define LEDROWSTOP_WRAP(x) LEDROWSTOP(x)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/***** Ultrasonic variables *****/
uint8_t left_detect = 0;
uint8_t right_detect = 0;

float left_sonic_threshold = 50;
float right_sonic_threshold = 50;

const float speedOfSound = 0.0343 / 2;
float distance_left;
float distance_right;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void usDelay(uint32_t uSec);
float ultrasonic_measure_left(void);
float ultrasonic_measure_right(void);
void ledrowoutput(float distance);
void wiggle(void);
/* USER CODE BEGIN PFP */

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
    MX_TIM4_Init();
    /* USER CODE BEGIN 2 */

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */
        ledrowoutput(ultrasonic_measure_left());
        HAL_Delay(10);
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

    /** Configure the main internal regulator output voltage
  */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 6;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
  */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

void usDelay(uint32_t uSec)
{
    // usDelay(2) actually delay 2.8us as measured by oscilloscope
    if (uSec < 2)
        uSec = 2;
    TIM4->ARR = uSec - 1; /*sets the value in the auto-reload register*/
    TIM4->EGR = 1;        /*Re-initialises the timer*/
    TIM4->SR &= ~1;       //Resets the flag
    TIM4->CR1 |= 1;       //Enables the counter
    while ((TIM4->SR & 0x0001) != 1)
        ;
    TIM4->SR &= ~(0x0001);
    /*
	Alternative method: need to change type of uSec to uint16_t and change ARR to 0xFFFF-1
   https://controllerstech.com/create-1-microsecond-delay-stm32/
	Test code:
	while (1)
  {
	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
	  delay_us(10);
  }
*/
    //	__HAL_TIM_SET_COUNTER(&htim4,0);  // set the counter value a 0
    //	while (__HAL_TIM_GET_COUNTER(&htim4) < uSec);  // wait for the counter to reach the us input in the parameter
}

float ultrasonic_measure_left(void)
{
    HAL_GPIO_WritePin(TRIG1_GPIO_Port, TRIG1_Pin, GPIO_PIN_RESET);
    usDelay(3);
    //*** START Ultrasonic measure routine ***//
    //1. Output 10 usec TRIG (actually 14us)
    HAL_GPIO_WritePin(TRIG1_GPIO_Port, TRIG1_Pin, GPIO_PIN_SET);
    usDelay(10);
    HAL_GPIO_WritePin(TRIG1_GPIO_Port, TRIG1_Pin, GPIO_PIN_RESET);

    //2. Wait for ECHO pin rising edge
    while (HAL_GPIO_ReadPin(ECHO1_GPIO_Port, ECHO1_Pin) == GPIO_PIN_RESET)
        ;
    //3. Start measuring ECHO pulse width in usec
    uint32_t numTicks = 0;
    while (HAL_GPIO_ReadPin(ECHO1_GPIO_Port, ECHO1_Pin) == GPIO_PIN_SET)
    {
        numTicks++;
        usDelay(2);
    };
    return (numTicks + 0.0f) * 2.8f * speedOfSound; // centimeter
    //    numTicks = 0;
}
float ultrasonic_measure_right(void)
{
    HAL_GPIO_WritePin(TRIG2_GPIO_Port, TRIG2_Pin, GPIO_PIN_RESET);
    usDelay(3);
    //*** START Ultrasonic measure routine ***//
    //1. Output 10 usec TRIG (actually 14us)
    HAL_GPIO_WritePin(TRIG2_GPIO_Port, TRIG2_Pin, GPIO_PIN_SET);
    usDelay(10);
    HAL_GPIO_WritePin(TRIG2_GPIO_Port, TRIG2_Pin, GPIO_PIN_RESET);

    //2. Wait for ECHO pin rising edge
    while (HAL_GPIO_ReadPin(ECHO2_GPIO_Port, ECHO2_Pin) == GPIO_PIN_RESET)
        ;
    //3. Start measuring ECHO pulse width in usec
    uint32_t numTicks = 0;
    while (HAL_GPIO_ReadPin(ECHO2_GPIO_Port, ECHO2_Pin) == GPIO_PIN_SET && numTicks < 2100)
    {
        // within 1 meter range
        numTicks++;
        usDelay(2);
    };
    return (numTicks + 0.0f) * 2.8f * speedOfSound; // centimeter
    //    numTicks = 0;
}
void wiggle(void)
{
    ;
}

// led output, test function
void ledrowoutput(float distance)
{
    if (distance < 12.5f)
    {
        LEDROW_WRAP(1);
        LEDROWSTOP_WRAP(2);
        LEDROWSTOP_WRAP(3);
        LEDROWSTOP_WRAP(4);
        LEDROWSTOP_WRAP(5);
        LEDROWSTOP_WRAP(6);
        LEDROWSTOP_WRAP(7);
        LEDROWSTOP_WRAP(8);
    }
    else if (distance < 25.0f)
    {
        LEDROW_WRAP(1);
        LEDROW_WRAP(2);
        LEDROWSTOP_WRAP(3);
        LEDROWSTOP_WRAP(4);
        LEDROWSTOP_WRAP(5);
        LEDROWSTOP_WRAP(6);
        LEDROWSTOP_WRAP(7);
        LEDROWSTOP_WRAP(8);
    }
    else if (distance < 37.5f)
    {
        LEDROW_WRAP(1);
        LEDROW_WRAP(2);
        LEDROW_WRAP(3);
        LEDROWSTOP_WRAP(4);
        LEDROWSTOP_WRAP(5);
        LEDROWSTOP_WRAP(6);
        LEDROWSTOP_WRAP(7);
        LEDROWSTOP_WRAP(8);
    }
    else if (distance < 50.0f)
    {
        LEDROW_WRAP(1);
        LEDROW_WRAP(2);
        LEDROW_WRAP(3);
        LEDROW_WRAP(4);
        LEDROWSTOP_WRAP(5);
        LEDROWSTOP_WRAP(6);
        LEDROWSTOP_WRAP(7);
        LEDROWSTOP_WRAP(8);
    }
    else if (distance < 62.5f)
    {
        LEDROW_WRAP(1);
        LEDROW_WRAP(2);
        LEDROW_WRAP(3);
        LEDROW_WRAP(4);
        LEDROW_WRAP(5);
        LEDROWSTOP_WRAP(6);
        LEDROWSTOP_WRAP(7);
        LEDROWSTOP_WRAP(8);
    }
    else if (distance < 75.0f)
    {
        LEDROW_WRAP(1);
        LEDROW_WRAP(2);
        LEDROW_WRAP(3);
        LEDROW_WRAP(4);
        LEDROW_WRAP(5);
        LEDROW_WRAP(6);
        LEDROWSTOP_WRAP(7);
        LEDROWSTOP_WRAP(8);
    }
    else if (distance < 87.5f)
    {
        LEDROW_WRAP(1);
        LEDROW_WRAP(2);
        LEDROW_WRAP(3);
        LEDROW_WRAP(4);
        LEDROW_WRAP(5);
        LEDROW_WRAP(6);
        LEDROW_WRAP(7);
        LEDROWSTOP_WRAP(8);
    }
    else
    {
        LEDROW_WRAP(1);
        LEDROW_WRAP(2);
        LEDROW_WRAP(3);
        LEDROW_WRAP(4);
        LEDROW_WRAP(5);
        LEDROW_WRAP(6);
        LEDROW_WRAP(7);
        LEDROW_WRAP(8);
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
