#include "uart.h"
#include <string.h>
#include "uart_cmd.h"
#include "comm.h"

#define OPEN_UART_RX_DMA_ISR    0
extern uint8_t uart_rx_buffer[UART_RX_SIZE];


/***************************************  init  *****************************************/
void hwi_UartInit(uint8_t channel)
{
    dma_single_data_parameter_struct dma_init_struct; 
	
		rcu_periph_clock_enable(RCU_USART2);	
		rcu_periph_clock_enable(RCU_DMA0);	
    /* USART configure */
    usart_deinit(USART2);
  
    usart_word_length_set(USART2, USART_WL_8BIT);
    usart_stop_bit_set(USART2, USART_STB_1BIT);
    usart_parity_config(USART2, USART_PM_NONE);  
    usart_baudrate_set(USART2, 460800U);
  
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_DISABLE);
//    usart_dma_enable(USART2,USART_DMA_TRANSMIT);
//    usart_dma_enable(USART2,USART_DMA_RECEIVE);
  
    usart_enable(USART2);
  
    /* deinitialize DMA0 channel3 */
    dma_deinit(DMA0, DMA_CH3);
    dma_deinit(DMA0, DMA_CH1);  
  
    dma_single_data_para_struct_init(&dma_init_struct);
    dma_init_struct.periph_addr = (uint32_t)&USART_TDATA(USART2);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
//    dma_init_struct.memory0_addr = (uint32_t)txbuffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
//    dma_init_struct.number = ARRAYNUM(txbuffer);
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(DMA0, DMA_CH3, &dma_init_struct);  

    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
    dma_init_struct.periph_addr = (uint32_t)&USART_RDATA(USART2); 
    dma_init_struct.memory0_addr = (uint32_t)&uart_rx_buffer; 
    dma_init_struct.number = UART_RX_SIZE;
    dma_single_data_mode_init(DMA0, DMA_CH1, &dma_init_struct);   

    
        /* DMA channel peripheral select */
    dma_channel_subperipheral_select(DMA0, DMA_CH3, DMA_SUBPERI4);
    /* DMA0 channel peripheral select */
    dma_channel_subperipheral_select(DMA0, DMA_CH1, DMA_SUBPERI4);    
		
    /* enable DMA0 channel1 */
    dma_channel_enable(DMA0, DMA_CH1);
    /* USART DMA enable for reception */
    usart_dma_enable(USART2, USART_DMA_RECEIVE);		
  
    /*wait IDLEF set and clear it*/
    while(RESET == usart_flag_get(USART2, USART_FLAG_IDLE));
    usart_flag_clear(USART2, USART_FLAG_IDLE);
    usart_interrupt_enable(USART2, USART_INT_IDLE);
		
		nvic_irq_enable(USART2_IRQn,0,0);
}

void hwi_UART_MspInit(uint8_t channel)
{
	
}

/***************************************  Dinit  *****************************************/
void hwi_UartDInit(void)
{
    usart_deinit(USART2);
}

void hwi_UART_MspDeInit(uint8_t channel)
{
  
}

void hwi_UartGpioInit(void)
{

}

void hwi_UartGpioDinit(void)
{

}

/***************************************  Send  *****************************************/
uint8_t hwi_UartStatusCheck(void)
{
    if (dma_flag_get(DMA0,DMA_CH3,DMA_FLAG_FTF) !=SET && (DMA_CH3CTL(DMA0) & DMA_CHXCTL_CHEN) !=RESET)
    {
        /* sending */
        return 0;
    }
    /* send completed */
    return 1;
}



/***************************************  ISR & Callback  *****************************************/
void hwi_USARTx_IRQHandler(void)
{

    uint32_t rx_num = 0;
    if(RESET != usart_interrupt_flag_get(USART2, USART_INT_FLAG_IDLE))
    {
        usart_interrupt_flag_clear(USART2, USART_INT_FLAG_IDLE);
        /* number of data received */
//        rx_count = 256 - (dma_transfer_number_get(DMA0, DMA_CH1));
//        receive_flag = 1;
        rx_num=UART_RX_SIZE-(dma_transfer_number_get(DMA0, DMA_CH1));
        if(rx_num > 0)
        {
            UartRxFun(uart_rx_buffer,rx_num);
        }
        usart_interrupt_flag_clear(USART2, USART_INT_FLAG_ERR_ORERR);
        memset(uart_rx_buffer,0,rx_num);
        /* disable DMA and reconfigure */
        dma_channel_disable(DMA0, DMA_CH1);
        usart_dma_disable(USART2, USART_DMA_RECEIVE);
        dma_flag_clear(DMA0, DMA_CH1, DMA_FLAG_FTF);
        dma_transfer_number_config(DMA0, DMA_CH1, UART_RX_SIZE);
        dma_channel_enable(DMA0, DMA_CH1);
        usart_dma_enable(USART2, USART_DMA_RECEIVE);
    }
}


void hwi_USARTx_DMA_TX_IRQHandler(void)
{

}

#if OPEN_UART_RX_DMA_ISR 
void hwi_DMA1_Channel2_3_IRQHandler(void)
{

}
#endif

void hwi_HAL_UART_TxCpltCallback(void)
{
  /* Set transmission flag: trasfer complete*/
 // UartReady = SET;
}

uint8_t hwi_UART_Transmit_DMA(uint8_t channel, uint8_t* buffer, uint16_t send_num)
{
		uint8_t ret=HWI_OK;
	
    uint32_t timeout=0;
    timeout=hwi_SysTick_GetTick(); 
    while(dma_flag_get(DMA0,DMA_CH3,DMA_FLAG_FTF)!=SET && (DMA_CH3CTL(DMA0)&DMA_CHXCTL_CHEN)!=RESET)
    {
        if((hwi_SysTick_GetTick()-timeout)>200U)
        {
					ret=HWI_ERROR;
          break;
        }   
    }
  
    dma_flag_clear(DMA0,DMA_CH3,DMA_FLAG_FTF);
  
    usart_dma_disable(USART2, USART_DMA_TRANSMIT);    
    /* enable DMA0 channel3 */
    dma_channel_disable(DMA0, DMA_CH3);
  
    dma_transfer_number_config(DMA0, DMA_CH3,send_num);
    dma_memory_address_config(DMA0, DMA_CH3,DMA_MEMORY_0,(uint32_t)buffer);

    dma_channel_enable(DMA0, DMA_CH3);  
    /* USART DMA enable for transmission */
    usart_dma_enable(USART2, USART_DMA_TRANSMIT);  
		
		return ret;
}

