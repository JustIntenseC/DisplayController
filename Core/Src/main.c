/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include <string.h>

/* USER CODE BEGIN PD */

// Разрешение
/* USER CODE BEGIN PD */
// 720x480 @ 60Hz (480p) — стандарт CEA-861
#define LCD_ACTIVE_WIDTH     720
#define LCD_ACTIVE_HEIGHT    480

#define H_SYNC_WIDTH         62
#define H_FRONT_PORCH        16
#define H_BACK_PORCH         60

#define V_SYNC_WIDTH         6
#define V_FRONT_PORCH        9
#define V_BACK_PORCH         30

#define H_TOTAL              858
#define V_TOTAL              525

#define FRAME_BUFFER_SIZE    (LCD_ACTIVE_WIDTH * LCD_ACTIVE_HEIGHT)  // L8 ≈ 345 КБ
/* USER CODE END PD */

static uint8_t frame_buffer[FRAME_BUFFER_SIZE] 
    __attribute__((section(".video_buffers"), aligned(32)));

/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
LTDC_HandleTypeDef hltdc;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_LTDC_Init(void);
void FillFrameBuffer(void);
void LoadCLUT(void);

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{
  HAL_MPU_Disable();
  __DSB();
  __ISB();

  // MPU_Config();
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_LTDC_Init();

  /* USER CODE BEGIN 2 */
  // Загружаем CLUT (256 градаций серого)
  LoadCLUT();

  // Заполняем буфер тестовым градиентом (серый)
  FillFrameBuffer();

  // Устанавливаем адрес буфера для слоя
  if(HAL_LTDC_SetAddress(&hltdc, (uint32_t)frame_buffer, LTDC_LAYER_1)!=HAL_OK){
    Error_Handler();
  };

  if(HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_IMMEDIATE)!=HAL_OK){
    Error_Handler();
  };

  // Включаем LTDC
  __HAL_LTDC_ENABLE(&hltdc);
  /* USER CODE END 2 */

  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOMEDIUM;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                              | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    Error_Handler();

  // === LTDC CLOCK = 27.000 MHz ===
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  PeriphClkInitStruct.PLL3.PLL3M = 8;
  PeriphClkInitStruct.PLL3.PLL3N = 27;
  PeriphClkInitStruct.PLL3.PLL3P = 2;
  PeriphClkInitStruct.PLL3.PLL3Q = 2;
  PeriphClkInitStruct.PLL3.PLL3R = 8;           // 216 / 8 = 27 MHz
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_3;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    Error_Handler();
}

/**
  * @brief LTDC Initialization (L8 with CLUT)
  */
static void MX_LTDC_Init(void)
{
  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  hltdc.Instance = LTDC;

  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity  = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity  = LTDC_PCPOLARITY_IPC;     // ← правильная константа

  hltdc.Init.HorizontalSync     = H_SYNC_WIDTH - 1;
  hltdc.Init.VerticalSync       = V_SYNC_WIDTH - 1;
  hltdc.Init.AccumulatedHBP     = H_SYNC_WIDTH + H_BACK_PORCH - 1;
  hltdc.Init.AccumulatedVBP     = V_SYNC_WIDTH + V_BACK_PORCH - 1;
  hltdc.Init.AccumulatedActiveW = H_SYNC_WIDTH + H_BACK_PORCH + LCD_ACTIVE_WIDTH - 1;
  hltdc.Init.AccumulatedActiveH = V_SYNC_WIDTH + V_BACK_PORCH + LCD_ACTIVE_HEIGHT - 1;
  hltdc.Init.TotalWidth         = H_TOTAL - 1;
  hltdc.Init.TotalHeigh         = V_TOTAL - 1;

  hltdc.Init.Backcolor.Blue  = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red   = 0;

  if (HAL_LTDC_Init(&hltdc) != HAL_OK) Error_Handler();

  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = LCD_ACTIVE_WIDTH;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = LCD_ACTIVE_HEIGHT;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_L8;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = (uint32_t)frame_buffer;
  pLayerCfg.ImageWidth = LCD_ACTIVE_WIDTH;
  pLayerCfg.ImageHeight = LCD_ACTIVE_HEIGHT;

  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, LTDC_LAYER_1) != HAL_OK)
    Error_Handler();
}

/**
  * @brief GPIO Initialization
  */
static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
}

/* USER CODE BEGIN 4 */

/**
  * @brief  Загрузить CLUT (Color Look-Up Table) для L8
  * @note   Запись 0 → чёрный, 255 → белый
  */
void LoadCLUT(void)
{
  uint32_t clut[256];

  for (int i = 0; i < 256; i++) {
    uint8_t gray = i;                     // градация серого
    clut[i] = (gray << 16) | (gray << 8) | gray; // RGB
  }

  if (HAL_LTDC_ConfigCLUT(&hltdc, clut, 256, LTDC_LAYER_1) != HAL_OK) {
    Error_Handler();
  }
}

/**
  * @brief  Заполнение буфера тестовым градиентом (8‑бит L8)
  */
void FillFrameBuffer(void)
{
  memset(frame_buffer, 0xff, FRAME_BUFFER_SIZE);
}
/* USER CODE END 4 */

/* MPU Configuration */
void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};
  HAL_MPU_Disable();

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

  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}