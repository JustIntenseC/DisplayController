#include <uart.h>

UART_HandleTypeDef huart1;

volatile uint8_t uart_command_received = 0;
uint8_t packet_received = 0;
uint8_t packet_index = 0;
char packet_buffer[64] = "";
extern volatile bool display_enabled;
void UART_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    // Тактирование USART1 от PCLK2 (обычно 100 МГц)
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        Error_Handler();

    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Пины PB14 (TX), PB15 (RX) – альтернативная функция AF4
    GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_USART1;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Настройка UART
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;

    if (HAL_UART_Init(&huart1) != HAL_OK)
        Error_Handler();

    // Настройка прерывания
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    USART1->CR1 |= USART_CR1_RXNEIE;
}

void UART_HandlePacket(void){
    if(!strncmp(packet_buffer, "start", 5)){
        display_enabled = 1;
    }
    else if(!strncmp(packet_buffer, "end", 3)){
        display_enabled = 0;
    }
}

void USART1_IRQHandler(void){

    uint8_t byte = USART1 -> RDR & 0xFF;
    if (packet_index < MAX_PACKET_SIZE) {
        packet_buffer[packet_index] = byte;
        packet_index++;
        if (byte == '\n') {
            packet_received =1;
            packet_index = 0;
            uart_command_received = 1;
        }
    } else {
        packet_index = 0;
    }
}

