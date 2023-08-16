#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "HWI_Hal.h"

/***************************************  config - usr  *****************************************/
#define UART_BOUNDRATE     460800
#define UART_RX_SIZE       3072


typedef struct 
{
	uint8_t buffer[UART_RX_SIZE];
	uint32_t  length;
}rx_buffer;


/****************function****************/
void TxUartEnable(void);
void UartInit(void);
void UartDInit(void);

void UartGpioInit(void);
void UartGpioDinit(void);

void UartSendDma(char* buffer,uint32_t send_num);
uint8_t UartStatusCheck(void);

#ifdef __cplusplus
}
#endif

#endif
