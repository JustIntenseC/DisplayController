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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LCD_ACTIVE_WIDTH  1280
#define  LCD_ACTIVE_HEIGHT 2400


#define H_SYNC_WIDTH  50
#define H_BACK_PORCH  50
#define H_FRONT_PORCH 50

#define V_SYNC_WIDTH  50
#define V_BACK_PORCH  50
#define V_FRONT_PORCH 50

#define H_TOTAL     (H_SYNC_WIDTH + H_FRONT_PORCH + LCD_ACTIVE_WIDTH + H_BACK_PORCH)
#define V_TOTAL     (V_SYNC_WIDTH + V_FRONT_PORCH + LCD_ACTIVE_HEIGHT + V_BACK_PORCH)
#define TOTAL_STRIPS  6

#define BYTES_PER_PIXEL 1
#define STRIP_HEIGHT 200
#define STRIP_SIZE_BYTES (LCD_ACTIVE_WIDTH * STRIP_HEIGHT)
#define TOTAL_STRIPS (LCD_ACTIVE_HEIGHT / STRIP_HEIGHT)
#define FRAME_RATE 1 /*Hz*/
#define ROW_TIME_US (1000000 / (FRAME_RATE * LCD_ACTIVE_HEIGHT))




/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

LTDC_HandleTypeDef hltdc;

/* USER CODE BEGIN PV */
uint32_t clut[256];
static uint8_t buffer0[STRIP_SIZE_BYTES] __attribute__((section(".video_buffers"), aligned(32)));
static uint8_t buffer1[STRIP_SIZE_BYTES] __attribute__((section(".video_buffers"), aligned(32)));
volatile uint32_t current_strip = 0 ;
volatile uint8_t need_fill = 0;
volatile uint8_t active_buffer = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_LTDC_Init(void);
/* USER CODE BEGIN PFP */
void FillStrip(uint8_t* buffer);
void GenerateTestPattern(uint8_t *buffer, uint32_t strip_idx);
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

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

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
  MX_LTDC_Init();
  /* USER CODE BEGIN 2 */
  // 1. Заполняем первый буфер данными для полосы 0
    FillStrip(buffer0);    // функция FillStrip заполняет буфер 256 КБ

    // 2. Указываем LTDC адрес первого буфера
    HAL_LTDC_SetAddress(&hltdc, (uint32_t)buffer0, LTDC_LAYER_1);
    active_buffer = 0;

    // 3. Программируем прерывание на конец первой полосы (200-я строка)
    HAL_LTDC_ProgramLineEvent(&hltdc, STRIP_HEIGHT);

    // 4. Включаем прерывание в NVIC
    HAL_NVIC_EnableIRQ(LTDC_IRQn);

    // 5. Запускаем LTDC
    HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_IMMEDIATE);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    __WFI();
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 32;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV4;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};


  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = H_SYNC_WIDTH - 1;
  hltdc.Init.VerticalSync = V_SYNC_WIDTH - 1;
  hltdc.Init.AccumulatedHBP = H_SYNC_WIDTH + H_BACK_PORCH - 1;
  hltdc.Init.AccumulatedVBP = V_SYNC_WIDTH + V_BACK_PORCH - 1;
  hltdc.Init.AccumulatedActiveW = H_SYNC_WIDTH + H_BACK_PORCH + LCD_ACTIVE_WIDTH - 1;
  hltdc.Init.AccumulatedActiveH = V_SYNC_WIDTH + V_BACK_PORCH + LCD_ACTIVE_HEIGHT - 1;
  hltdc.Init.TotalWidth = H_TOTAL - 1;
  hltdc.Init.TotalHeigh = V_TOTAL - 1;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = LCD_ACTIVE_WIDTH;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = LCD_ACTIVE_HEIGHT;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_L8;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  pLayerCfg.FBStartAdress = (uint32_t)buffer0;
  pLayerCfg.ImageWidth = LCD_ACTIVE_WIDTH;
  pLayerCfg.ImageHeight = LCD_ACTIVE_HEIGHT;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */
  for(int i = 0 ; i < 256; i++){
    clut[i] = i << 16 | i << 8 | i; 
  }
  HAL_LTDC_ConfigCLUT(&hltdc, clut, 256, LTDC_LAYER_1);
  /* USER CODE END LTDC_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void FillStrip(uint8_t *buffer){
  for(uint32_t row = 0 ; row < STRIP_HEIGHT; row++){
    for(uint32_t column = 0; column < LCD_ACTIVE_WIDTH; column++){
      buffer[row*LCD_ACTIVE_WIDTH + column]= 0xFF;
    }
  }
}

void LTDC_IRQHandler(void)
{
    if (__HAL_LTDC_GET_FLAG(&hltdc, LTDC_FLAG_LI))
    {
        __HAL_LTDC_CLEAR_FLAG(&hltdc, LTDC_FLAG_LI);

        current_strip++;   // переходим к следующей полосе

        if (current_strip < TOTAL_STRIPS)   // 0..11
        {
            // ---- 1. Переключаем LTDC на другой буфер (уже заполненный) ----
            if (active_buffer == 0)
                HAL_LTDC_SetAddress(&hltdc, (uint32_t)buffer1, LTDC_LAYER_1);
            else
                HAL_LTDC_SetAddress(&hltdc, (uint32_t)buffer0, LTDC_LAYER_1);

            // ---- 2. Инвертируем флаг активного буфера ----
            active_buffer ^= 1;

            // ---- 3. Программируем следующее прерывание (на конец следующей полосы) ----
            HAL_LTDC_ProgramLineEvent(&hltdc, (current_strip + 1) * STRIP_HEIGHT);

            // ---- 4. Заполняем освободившийся буфер данными для будущей полосы ----
            if (active_buffer == 0)
                FillStrip(buffer0);   // current_strip уже увеличен
            else
                FillStrip(buffer1);
        }
        else
        {
            // Конец кадра: сбрасываем счётчик, начинаем новый кадр
            current_strip = 0;

            // Переключаем LTDC на другой буфер (для первой полосы нового кадра)
            if (active_buffer == 0)
                HAL_LTDC_SetAddress(&hltdc, (uint32_t)buffer1, LTDC_LAYER_1);
            else
                HAL_LTDC_SetAddress(&hltdc, (uint32_t)buffer0, LTDC_LAYER_1);
            active_buffer ^= 1;

            // Программируем прерывание на конец первой полосы нового кадра
            HAL_LTDC_ProgramLineEvent(&hltdc, STRIP_HEIGHT);

            // Заполняем освободившийся буфер для первой полосы нового кадра
            if (active_buffer == 0)
                FillStrip(buffer0);
            else
                FillStrip(buffer1);
        }
    }
}  


/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

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
