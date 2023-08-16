#include "uart.h"
#include <string.h>
#include "uart_cmd.h"
#include "comm.h"

#define OPEN_UART_RX_DMA_ISR    0



/***************************************  config  *****************************************/



/* Definition for USARTx's NVIC */
#define USARTx_IRQn                      USART1_IRQn
//#define USARTx_IRQHandler                USART1_IRQHandler

uint8_t uart_rx_buffer[UART_RX_SIZE];
uint8_t uart_rx_enalbe_flag;
uint8_t uart_tx_enalbe_flag;

/***************************************  TxUartEnable  *****************************************/
void TxUartEnable(void)
{
    if(uart_tx_enalbe_flag == 0)
    {
        usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);
        uart_tx_enalbe_flag = 1;
    }
}

/***************************************  init  *****************************************/
void UartInit(void)
{
	if(uart_rx_enalbe_flag == 0)
	{
        hwi_UartInit(1);
	}
	
	uart_rx_enalbe_flag = 1;
}

/***************************************  Dinit  *****************************************/
void UartDInit(void)
{
	if(uart_rx_enalbe_flag == 1)
	{
	
        hwi_UartDInit();
	}
	
	uart_rx_enalbe_flag = 0;
}

//void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
//{
//    hwi_UART_MspDeInit(1);
//}

void UartGpioInit(void)
{
    hwi_UartGpioInit();
}

void UartGpioDinit(void)
{
    hwi_UartGpioDinit();
}

/***************************************  Send  *****************************************/
//uint8_t UartReady = 1;

void UartSendDma(char* buffer,uint32_t send_num)
{
	//if(UartReady == RESET)
	{
//		Error_Handler();
	}
	
	if(hwi_UART_Transmit_DMA(1, (uint8_t*)buffer, send_num)!= HWI_OK)
     
  {
//			Error_Handler();
	}
	
	//UartReady = RESET;
}

uint8_t UartStatusCheck(void)
{
	return hwi_UartStatusCheck();
}
#if 0
/***************************************  Receive  *****************************************/
void UartRxFun(uint8_t * buffer,uint32_t length)
{
		memset(buffer,0,length);
}
#endif
/***************************************  ISR & Callback  *****************************************/
void USARTx_IRQHandler(void)
{
#if 0
	if(((UartHandle.Instance->ISR) & USART_ISR_IDLE) == USART_ISR_IDLE)
	{
		__HAL_UART_CLEAR_FLAG(&UartHandle, UART_CLEAR_IDLEF);
		
		uint32_t rx_num = 0;
		rx_num	=	UART_RX_SIZE	-	UartHandle.hdmarx->Instance->CNDTR;
		
		if(rx_num > 0)
		{
			UartRxFun(uart_rx_buffer,rx_num);
		}
		
		memset(uart_rx_buffer,0,UART_RX_SIZE);
		HAL_UART_AbortReceive(&UartHandle);
		if(HAL_UART_Receive_DMA(&UartHandle, (uint8_t *)uart_rx_buffer, UART_RX_SIZE) != HAL_OK)
		{
		//	Error_Handler();
		}
	}
	
  HAL_UART_IRQHandler(& UartHandle);
  #endif
}


void USARTx_DMA_TX_IRQHandler(void)
{
  //HAL_DMA_IRQHandler(UartHandle.hdmatx);

}

#if OPEN_UART_RX_DMA_ISR 
void DMA1_Channel2_3_IRQHandler(void)
{
  //HAL_DMA_IRQHandler(UartHandle.hdmarx);
}
#endif

