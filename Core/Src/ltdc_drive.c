#include "ltdc_drive.h"

extern LTDC_Block_t lt;

void LTDC_Init(LTDC_HandleTypeDef *hltdc, LTDC_LayerCfgTypeDef *pLayerCfg, uint8_t* frame_buffer)
{

  hltdc->Instance = LTDC;

  hltdc->Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc->Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc->Init.DEPolarity  = LTDC_DEPOLARITY_AL;
  hltdc->Init.PCPolarity  = LTDC_PCPOLARITY_IPC;     

  hltdc->Init.HorizontalSync     = H_SYNC_WIDTH - 1;
  hltdc->Init.VerticalSync       = V_SYNC_WIDTH - 1;
  hltdc->Init.AccumulatedHBP     = H_SYNC_WIDTH + H_BACK_PORCH - 1;
  hltdc->Init.AccumulatedVBP     = V_SYNC_WIDTH + V_BACK_PORCH - 1;
  hltdc->Init.AccumulatedActiveW = H_SYNC_WIDTH + H_BACK_PORCH + LCD_ACTIVE_WIDTH - 1;
  hltdc->Init.AccumulatedActiveH = V_SYNC_WIDTH + V_BACK_PORCH + LCD_ACTIVE_HEIGHT - 1;
  hltdc->Init.TotalWidth         = H_TOTAL - 1;
  hltdc->Init.TotalHeigh         = V_TOTAL - 1;

  hltdc->Init.Backcolor.Blue  = 0;
  hltdc->Init.Backcolor.Green = 0;
  hltdc->Init.Backcolor.Red   = 0;

  if (HAL_LTDC_Init(hltdc) != HAL_OK) Error_Handler();

  pLayerCfg->WindowX0 = 0;
  pLayerCfg->WindowX1 = LCD_ACTIVE_WIDTH;
  pLayerCfg->WindowY0 = 0;
  pLayerCfg->WindowY1 = LCD_ACTIVE_HEIGHT;
  pLayerCfg->PixelFormat = LTDC_PIXEL_FORMAT_L8;
  pLayerCfg->Alpha = 255;
  pLayerCfg->Alpha0 = 0;
  pLayerCfg->BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg->BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg->FBStartAdress = (uint32_t)frame_buffer;
  pLayerCfg->ImageWidth = LCD_ACTIVE_WIDTH;
  pLayerCfg->ImageHeight = LCD_ACTIVE_HEIGHT;
  if (HAL_LTDC_ConfigLayer(hltdc, pLayerCfg, LTDC_LAYER_1) != HAL_OK)
    Error_Handler();

  HAL_LTDC_ProgramLineEvent(hltdc, LCD_ACTIVE_HEIGHT - 1);
  HAL_NVIC_SetPriority(LTDC_IRQn,0,0);
  HAL_NVIC_EnableIRQ(LTDC_IRQn);
}

void LoadCLUT(LTDC_HandleTypeDef *hltdc)
{
  uint32_t clut[256];

  for (int i = 0; i < 256; i++) {
    uint8_t gray = i;                   
    clut[i] = (gray << 16) | (gray << 8) | gray; 
  }

  if (HAL_LTDC_ConfigCLUT(hltdc, clut, 256, LTDC_LAYER_1) != HAL_OK) {
    Error_Handler();
  }
  if(HAL_LTDC_EnableCLUT(hltdc, LTDC_LAYER_1)!=HAL_OK){
    Error_Handler();
  }
}

void FillFrameBuffer(uint8_t value, uint8_t* frame_buffer)
{
  memset(frame_buffer, value, FRAME_BUFFER_SIZE);
  
}



void LTDC_IRQHandler(void){

   
    if (__HAL_LTDC_GET_FLAG(&lt.hltdc, LTDC_FLAG_LI))
    {
        __HAL_LTDC_CLEAR_FLAG(&lt.hltdc, LTDC_FLAG_LI);
        
        lt.frame_status = FRAME_READY;                   
        
    }
   

}

