
#include "main.h"




#ifndef LTDC_DRIVER_H
#define LTDC_DRIVER_H

#define SD

#ifdef SD

#define LTDC_PCLK_VALUE      25
#define LCD_ACTIVE_WIDTH     640
#define LCD_ACTIVE_HEIGHT    480

#define H_SYNC_WIDTH         96
#define H_FRONT_PORCH        16
#define H_BACK_PORCH         48

#define V_SYNC_WIDTH         2
#define V_FRONT_PORCH        10
#define V_BACK_PORCH         33

#define H_TOTAL              800
#define V_TOTAL              525
#endif 

#ifdef SVGA 

#define LTDC_PCLK_VALUE      40
#define LCD_ACTIVE_WIDTH     800
#define LCD_ACTIVE_HEIGHT    600

#define H_SYNC_WIDTH         128
#define H_FRONT_PORCH        40
#define H_BACK_PORCH         88

#define V_SYNC_WIDTH         4
#define V_FRONT_PORCH        1
#define V_BACK_PORCH         23

#define H_TOTAL              1056
#define V_TOTAL              628
#endif


#ifdef XGA_1024x768

#define LTDC_PCLK_VALUE      65
#define LCD_ACTIVE_WIDTH     1024
#define LCD_ACTIVE_HEIGHT    768

#define H_SYNC_WIDTH         136
#define H_FRONT_PORCH        24
#define H_BACK_PORCH         160

#define V_SYNC_WIDTH         6
#define V_FRONT_PORCH        3
#define V_BACK_PORCH         29

#define H_TOTAL              1344  // 1024 + 24 + 136 + 160
#define V_TOTAL              806   // 768 + 3 + 6 + 29

#endif


#ifdef HXXZ_4894K


#define LTDC_PCLK_VALUE      162
#define LCD_ACTIVE_WIDTH     1280
#define LCD_ACTIVE_HEIGHT    2400

#define H_SYNC_WIDTH         48
#define H_FRONT_PORCH        160
#define H_BACK_PORCH         160

#define V_SYNC_WIDTH         2
#define V_FRONT_PORCH        16
#define V_BACK_PORCH         20

#define H_TOTAL              1648
#define V_TOTAL              2438


#endif

#ifdef HXXZ_4894K_DDR

#define LTDC_PCLK_VALUE      81      
#define LCD_ACTIVE_WIDTH     640     
#define LCD_ACTIVE_HEIGHT    2400    

#define H_SYNC_WIDTH         16      
#define H_FRONT_PORCH        80     
#define H_BACK_PORCH         88     

#define V_SYNC_WIDTH         4       
#define V_FRONT_PORCH        3       
#define V_BACK_PORCH         31      

#define H_TOTAL              824     
#define V_TOTAL              2438    

#endif

#define FRAME_BUFFER_SIZE    (LCD_ACTIVE_WIDTH * LCD_ACTIVE_HEIGHT)  

typedef enum {
    FSM_IDLE,           // Вывод отключён, LTDC остановлен
    FSM_START_FRAME,    // Подготовка к выводу нового кадра
    FSM_RUNNING,        // LTDC активен, передача полос идёт (прерывания)
    FSM_END_FRAME,      // Кадр завершён, можно перезапустить или остановиться
} fsm_state_t;

typedef enum{
    FRAME_NOT_READY,
    FRAME_READY
} frame_state_t;

typedef struct{
    fsm_state_t state;
    uint8_t * frame_buffer;
    uint8_t * frame_buffer_back;
    LTDC_HandleTypeDef hltdc;
    LTDC_LayerCfgTypeDef pLayerCfg;
    volatile uint32_t frame_status;
} LTDC_Block_t;


void LTDC_Init(LTDC_HandleTypeDef *hltdc, LTDC_LayerCfgTypeDef *pLayerCfg, uint8_t* frame_buffer);
void LoadCLUT(LTDC_HandleTypeDef *hltdc);
void FillFrameBuffer(uint8_t value, uint8_t *frame_buffer);
void LTDC_FSM_Handle(LTDC_Block_t *MainLTDC);
#endif