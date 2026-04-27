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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

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

/* USER CODE BEGIN PV */
volatile uint8_t rx_data;        // 串口接收数据
volatile uint8_t action_flag = 0; // 动作标志，指示主循环需要执行的操作
volatile uint8_t is_busy = 0;    // 忙状态标志：0表示空闲，1表示正在执行动作
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
  * @brief  UART接收完成回调函数
  * @param  huart: UART句柄
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2) 
    {
        // 仅在系统空闲时，才处理接收到的有效指令
        if(is_busy == 0) 
        {
            if(rx_data == 'A' || rx_data == 'B')
            {
                action_flag = rx_data; // 设置标志位，通知主循环执行相应动作
            }
        }
        
        // 重新启动接收中断
        HAL_UART_Receive_IT(&huart2, (uint8_t *)&rx_data, 1);
    }
}

/**
  * @brief  UART错误回调函数
  * @param  huart: UART句柄
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
    {
        // 处理任何接收溢出错误（特别是OVR错误标志）
        // 1. 清除错误标志位，防止死锁
        __HAL_UART_CLEAR_OREFLAG(huart);
        
        // 2. 强制重新启动接收中断，确保通信持续
        HAL_UART_Receive_IT(&huart2, (uint8_t *)&rx_data, 1);
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
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  // 启动UART接收中断
  HAL_UART_Receive_IT(&huart2, (uint8_t *)&rx_data, 1);
  
  // ================= 初始化系统所有PWM通道 =================
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); // 0号舵机 (PA6)
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3); // 2号舵机 (PB0)
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); // 4号舵机 (PA15)
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3); // 6号舵机 (PB10, SG90)
  
  // ================= 设置所有舵机到中位 =================
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 1500); 
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 1500); 
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 1500); 
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 1500); 
  
  // 强制给所有舵机1.5秒时间，确保它们都能回到初始状态
  HAL_Delay(1500);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // ================= 动作 A：机械臂抓取并自动复位 =================
    if(action_flag == 'A')
    {
        is_busy = 1;  // 设置忙标志，防止在执行当前动作时响应新的指令
        
        // 1. 机械爪张开，准备抓取 (速度 1800 微秒)
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 1800); 
        HAL_Delay(500); 
        
        // 2. 大臂向上转动到1900位置
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 1900); 
        HAL_Delay(800); 
        
        // 3. 小臂向下转动到1200位置
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 1200); 
        HAL_Delay(800); 
        
        // 4. 手腕向上转动到1800位置，靠近目标
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 1800); 
        HAL_Delay(800); 
        
        // 5. 机械爪闭合，抓取物体(速度 1200 微秒，注意此时不要转动)
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 1200); 
        HAL_Delay(800); // 给足够时间完成抓取
        
        // --- 抓取完成，开始自动复位流程 ---  
        
        // 6. 手腕回到中位
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 1500); 
        HAL_Delay(800); 
        
        // 7. 小臂回到中位
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 1500); 
        HAL_Delay(800); 
        
        // 8. 大臂携带物体回到中位 (1500)
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 1500); 
        HAL_Delay(800); 
        
        action_flag = 0; // 清除动作标志
        is_busy = 0;     // 清除忙标志，恢复空闲状态
    }
    
    // ================= 动作 B：机械臂放置物体并自动复位 =================
    else if(action_flag == 'B')
    {
        is_busy = 1;  // 设置忙标志
        
        // 1. 大臂向左转动到1100位置（模拟放置区）
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 1100); 
        HAL_Delay(800); 
        
        // 2. 小臂向下转动到1200位置
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 1200); 
        HAL_Delay(800); 
        
        // 3. 手腕向上转动到1800位置，靠近放置平台
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 1800); 
        HAL_Delay(800); 
        
        // 4. 机械爪张开，释放物体(速度 1800 微秒)
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 1800); 
        HAL_Delay(800); 
        
        // --- 放置完成，开始自动复位流程 ---  
        
        // 5. 手腕回到中位
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 1500); 
        HAL_Delay(800); 
        
        // 6. 小臂回到中位
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 1500); 
        HAL_Delay(800); 
        
        // 7. 大臂回到中位 (1500)
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 1500); 
        HAL_Delay(800); 
        
        // 8. 机械爪回到安全的中位状态 (1500)
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 1500); 
        HAL_Delay(500); 
        
        action_flag = 0; // 清除动作标志
        is_busy = 0;     // 清除忙标志，恢复空闲状态
    } 
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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