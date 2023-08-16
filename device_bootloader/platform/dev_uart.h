#ifndef  _DEV_UART_H
#define  _DEV_UART_H

#include "stdint.h"

#define  DEV_UART1     0
#define  DEV_UART2     1
#define  DEV_UART_MAX  2

#define BUFFER_SIZE  150

void Dev_UART_Init(uint8_t id);
void Dev_UART_DeInit(uint8_t id);
void Dev_UART_GPIO_Init(uint8_t id);
uint8_t Dev_UART_Send(uint8_t id, uint8_t* pdata, uint8_t len);
void Dev_USART2_IRQHandler(void);
extern uint8_t Ring_Buffer[BUFFER_SIZE];
extern uint8_t gUartRcvDataFlag;

#endif
