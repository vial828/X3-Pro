/******************************************************************************
*
*    Copyright (C), 2019-2020, xi'an byd Co., Ltd.
*
*******************************************************************************
* file name :
* description:

* author    :
* vertion   :
* data      :

*
*history version:
*      <author>         <date>           <version>      <description>
*
******************************************************************************/


/*******************************--includes--**********************************/
#include "string.h"

#include "HWI_Hal.h"

#include "ymodem.h"

#include "dev_uart.h"

/*******************************--define--************************************/



/*****************************--type define--*********************************/


/***************************--global variable--*******************************/
uint8_t Ring_Buffer[BUFFER_SIZE] = {0};
uint8_t RxBuffer[BUFFER_SIZE] = {0};

uint8_t gUartRcvDataFlag = 0;
uint32_t gUartRcvDataFlag_time = 0;

/**************************--function declaration--***************************/


/*****************************************************************************/
void Dev_UART_DMA_Init(uint8_t channel)
{
#if 0
    if(dev == USART1)
    {
        /* USART1_TX Init */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
        /* DMA interrupt init */
        /* DMA1_Channel1_IRQn interrupt configuration */
        NVIC_SetPriority(DMA1_Channel1_IRQn, 0);
        NVIC_EnableIRQ(DMA1_Channel1_IRQn);
        LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_1, LL_DMAMUX_REQ_USART1_RX);
        LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_1, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
        LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PRIORITY_LOW);
        LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MODE_NORMAL);
        LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PERIPH_NOINCREMENT);
        LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MEMORY_INCREMENT);
        LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PDATAALIGN_BYTE);
        LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MDATAALIGN_BYTE);
        LL_USART_EnableDMAReq_RX(USART1);
        LL_DMA_SetPeriphAddress(DMA1,LL_DMA_CHANNEL_1,(uint32_t)(&USART1->RDR));

        LL_DMA_SetDataLength(DMA1,LL_DMA_CHANNEL_1,sizeof(RxBuffer));
        LL_DMA_SetMemoryAddress(DMA1,LL_DMA_CHANNEL_1,(uint32_t)RxBuffer);
        LL_DMA_EnableChannel(DMA1,LL_DMA_CHANNEL_1);
       /* USART1 interrupt Init */
    }
#endif
    
}

/******************************************************************************
 * function name:
 * description:
 * input:
 * return:
 * exception:
******************************************************************************/
void Dev_UART_GPIO_Init(uint8_t id)
{
#if 0
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);

    if(id == DEV_UART1)
    {
        GPIO_InitStruct.Pin = LL_GPIO_PIN_9|LL_GPIO_PIN_10;
    }
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif
		rcu_periph_clock_enable(RCU_GPIOB);
		rcu_periph_clock_enable(RCU_GPIOA);
		rcu_periph_clock_enable(RCU_GPIOC);
	    //PA6:USART_TX;PA7:USART_RX.
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_6);
    gpio_af_set(GPIOA, GPIO_AF_10, GPIO_PIN_6);

    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_7);
    gpio_af_set(GPIOA, GPIO_AF_8, GPIO_PIN_7);
}
/******************************************************************************
 * function name:
 * description:
 * input:
 * return:
 * exception:
******************************************************************************/
void Dev_UART_Init(uint8_t id)
{
#if 0
    LL_USART_InitTypeDef USART_InitStruct = {0};
    USART_TypeDef* dev = (USART_TypeDef*)0;
    /* Peripheral clock enable */
    if(id < DEV_UART_MAX)
    {
        if(id == DEV_UART1)
        {
            Dev_UART_GPIO_Init(DEV_UART1);
            LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
            dev = USART1;
            NVIC_SetPriority(USART1_IRQn, 0);
            NVIC_EnableIRQ(USART1_IRQn);
        }

        /** USART1 GPIO Configuration
        PA9   ------> USART1_TX
        PA10  ------> USART1_RX
        **/
        USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
        USART_InitStruct.BaudRate = 460800;
        USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
        USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
        USART_InitStruct.Parity = LL_USART_PARITY_NONE;
        USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
        USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
        USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
        LL_USART_Init(dev, &USART_InitStruct);
        LL_USART_SetTXFIFOThreshold(dev, LL_USART_FIFOTHRESHOLD_1_8);
        LL_USART_SetRXFIFOThreshold(dev, LL_USART_FIFOTHRESHOLD_1_8);
        //LL_USART_EnableFIFO(USART1);
        LL_USART_DisableFIFO(dev);
        LL_USART_ConfigAsyncMode(dev);
        Dev_UART_DMA_Init(dev);
        LL_USART_ClearFlag_IDLE(dev);
        LL_USART_EnableIT_IDLE(dev);
//        LL_USART_EnableIT_RXNE_RXFNE(dev);
        LL_USART_Enable(dev);
        /* Polling USART1 initialisation */
        while((!(LL_USART_IsActiveFlag_TEACK(dev))) || (!(LL_USART_IsActiveFlag_REACK(dev))))
        {
        }
    }
#endif
dma_single_data_parameter_struct dma_init_struct; 
	  Dev_UART_GPIO_Init(0);
		rcu_periph_clock_enable(RCU_USART2);	
		rcu_periph_clock_enable(RCU_DMA0);	
    /* USART configure */
    usart_deinit(USART2);
  
    usart_word_length_set(USART2, USART_WL_8BIT);
    usart_stop_bit_set(USART2, USART_STB_1BIT);
    usart_parity_config(USART2, USART_PM_NONE);  
    usart_baudrate_set(USART2, 460800U);
  
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);
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
    dma_init_struct.memory0_addr = (uint32_t)&RxBuffer; 
    dma_init_struct.number = BUFFER_SIZE;
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

void Dev_UART_DeInit(uint8_t id)
{
#if 0
    USART_TypeDef* dev = (USART_TypeDef*)0;

    if(id < DEV_UART_MAX)
    {
        LL_AHB1_GRP1_DisableClock(LL_AHB1_GRP1_PERIPH_DMA1);
        if(id == DEV_UART1)
        {
            LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_USART1);
            dev = USART1;
            NVIC_DisableIRQ(USART1_IRQn);
            NVIC_DisableIRQ(DMA1_Channel1_IRQn);
        }
    }
    LL_USART_Disable(dev);
#endif
}

/******************************************************************************
 * function name:
 * description:
 * input:
 * return:
 * exception:
******************************************************************************/
uint8_t Dev_UART_Send(uint8_t id, uint8_t* pdata, uint8_t len)
{
    uint8_t i = 0;
    uint32_t tickstart = 0;

		while(i < len)
		{
			  usart_data_transmit(USART2,pdata[i]);
				tickstart = hwi_SysTick_GetTick();
				while(RESET == usart_flag_get(USART2, USART_FLAG_TBE))
				{
						if((hwi_SysTick_GetTick() - tickstart)>= 10)
						{
								return 1;
						}
				}
				i++;
		}  
		return 0;
}


/******************************************************************************
 * function name:
 * description:
 * input:
 * return:
 * exception:
******************************************************************************/
uint8_t Dev_UART_Receive(uint8_t id,uint8_t *pdata,uint8_t len,uint8_t timeout)
{
#if 0
    uint8_t i = 0;
    uint32_t tickstart = 0;

    USART_TypeDef* dev = (USART_TypeDef*)0;
    if(id < DEV_UART_MAX)
    {
        if(id == DEV_UART1)
        {
            dev = USART1;
        }
        while(len > 0)
        {
            tickstart = GetTicks();
            while(LL_USART_IsActiveFlag_RXNE(dev) != 1)
            {
                if((GetTicks() - tickstart)>= timeout)
                {
                    return 1;
                }
            }
            pdata[i++] = LL_USART_ReceiveData8(dev);
            len--;
        }
    }
#endif
    return 0;
}

void Dev_USART2_IRQHandler(void)
{
#if 1
    uint8_t uart_len = 0;
    if(RESET != usart_flag_get(USART2, USART_FLAG_IDLE))
    {
			  usart_flag_clear(USART2, USART_FLAG_IDLE);
				usart_dma_disable(USART2, USART_DMA_RECEIVE);
        if(dma_flag_get(DMA0, DMA_CH1, DMA_FLAG_FTF))
        {
            NVIC_SystemReset();
        }			  
        else
        {
            uart_len = BUFFER_SIZE - dma_transfer_number_get(DMA0, DMA_CH1);
            if(uart_len > 0)
            {
                memcpy(Ring_Buffer,RxBuffer,uart_len);
                gUartRcvDataFlag = 1;
                gUartRcvDataFlag_time = hwi_SysTick_GetTick();
                memset(RxBuffer,0,BUFFER_SIZE);
            }
        }
			/* disable DMA and reconfigure */
			dma_channel_disable(DMA0, DMA_CH1);
			usart_dma_disable(USART2, USART_DMA_RECEIVE);      
			dma_flag_clear(DMA0, DMA_CH1, DMA_FLAG_FTF);
			dma_transfer_number_config(DMA0, DMA_CH1, BUFFER_SIZE);
			dma_channel_enable(DMA0, DMA_CH1);
			usart_dma_enable(USART2, USART_DMA_RECEIVE); 
    }
#endif
}

