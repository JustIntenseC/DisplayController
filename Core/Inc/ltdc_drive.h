
#include "main.h"


#define SD

#ifndef LTDC_DRIVER_H
#define LTDC_DRIVER_H

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

#define FRAME_BUFFER_SIZE    (LCD_ACTIVE_WIDTH * LCD_ACTIVE_HEIGHT)  

typedef enum {
    FSM_IDLE,           // Вывод отключён, LTDC остановлен
    FSM_START_FRAME,    // Подготовка к выводу нового кадра
    FSM_FILL_STRIP,     // Заполнение первого буфера (полоса 0)
    FSM_RUNNING,        // LTDC активен, передача полос идёт (прерывания)
    FSM_WAIT_IRQ,       // Ожидание прерывания (неблокирующее, фактически не нужно)
    FSM_CHANGE_BUFFER,     // Переключение буферов, перезапись следующей полосы
    FSM_END_FRAME,      // Кадр завершён, можно перезапустить или остановиться
    FSM_STOP_FRAME            // Остановка LTDC по запросу пользователя
} fsm_state_t;

typedef struct{
    fsm_state_t state;
    uint8_t * frame_buffer;
    LTDC_HandleTypeDef hltdc;
    LTDC_LayerCfgTypeDef pLayerCfg;
    volatile uint32_t count;
} LTDC_Block_t;


void LTDC_Init(LTDC_HandleTypeDef *hltdc, LTDC_LayerCfgTypeDef *pLayerCfg, uint8_t* frame_buffer);
void LoadCLUT(LTDC_HandleTypeDef *hltdc);
void FillFrameBuffer(uint8_t value, uint8_t *frame_buffer);
void LTDC_FSM_Handle(LTDC_Block_t *MainLTDC);
#endif