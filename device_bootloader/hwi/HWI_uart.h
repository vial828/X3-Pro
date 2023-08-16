#ifndef __HAL_UART_H
#define __HAL_UART_H

#ifdef __cplusplus
 extern "C" {
#endif



/***************************************  config - usr  *****************************************/
void hwi_UartInit(uint8_t channel);
void hwi_UART_MspInit(uint8_t channel);
void hwi_UartDInit(void);
void hwi_UART_MspDeInit(uint8_t channel);
void hwi_UartGpioInit(void);
void hwi_UartGpioDinit(void);
void hwi_UartSendDma(char* buffer,uint32_t send_num);
uint8_t hwi_UartStatusCheck(void);
void hwi_USARTx_IRQHandler(void);
void hwi_USARTx_DMA_TX_IRQHandler(void);
void hwi_DMA1_Channel2_3_IRQHandler(void);
//void hwi_HAL_UART_TxCpltCallback(void);
uint8_t hwi_UART_Transmit_DMA(uint8_t channel, uint8_t* buffer, uint16_t send_num);




#ifdef __cplusplus
}
#endif

#endif
