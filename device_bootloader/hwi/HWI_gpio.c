#include "HWI_Hal.h"
#include "HWI_gpio.h"

/**
  * @brief  Initialize the GPIOx peripheral according to the specified parameters in the GPIO_Init.
  * @param  GPIOx where x can be (A..F) to select the GPIO peripheral for STM32G0xx family
  * @param  GPIO_Init pointer to a GPIO_InitTypeDef structure that contains
  *         the configuration information for the specified GPIO peripheral.
  * @retval None
  */
void hwi_GPIO_Init(void)
{
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOC);

	//io out bit set
	//gpio_bit_set(GPIOA,GPIO_PIN_0);
    gpio_bit_set(GPIOB,GPIO_PIN_11);// lgf
	/*PB7：timer3_CH1*/
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_7);
    gpio_af_set(GPIOB, GPIO_AF_2, GPIO_PIN_7);
	
	//PB9:GPIO OUTPUT
	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_9 | GPIO_PIN_11);// lgf
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_9 | GPIO_PIN_11);// lgf
	
	//PA15: EXTI INPUT
	{
	rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_SYSCFG);

    /* configure button pin as input */
    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_15);

//    /* enable and set key EXTI interrupt to the lowest priority */
//    nvic_irq_enable(EXTI10_15_IRQn, 2U, 0U);

//    /* connect key EXTI line to key GPIO pin */
//    syscfg_exti_line_config(EXTI_SOURCE_GPIOA, EXTI_SOURCE_PIN15);

//   /* configure key EXTI line */
//    exti_init(EXTI_15, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
//    exti_interrupt_flag_clear(EXTI_15);
	}
	
		//PA0/PA1/PA2/PA3/PC0/PC1/PC2/PC3/PA4: ADC input.
		gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_0| GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3|GPIO_PIN_4);
		gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_0| GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3);
	
    //PA5:GPIO OUTPUT
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_5);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_5);

    //PA6:USART_TX;PA7:USART_RX.
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_6);
    gpio_af_set(GPIOA, GPIO_AF_10, GPIO_PIN_6);

    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_7);
    gpio_af_set(GPIOA, GPIO_AF_8, GPIO_PIN_7);
	
	// PC4/PB0:不清楚作为输出还是输入，需客户自主添加配置
	
    // PC5: GPIO OUTPUT.
    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_5|GPIO_PIN_4);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_5|GPIO_PIN_4);

    	// PB1: GPIO OUTPUT.
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_1|GPIO_PIN_0);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_1|GPIO_PIN_0);
		
		
	//PB2: EXTI INPUT
	{
//		rcu_periph_clock_enable(RCU_GPIOA);
//    rcu_periph_clock_enable(RCU_SYSCFG);

    /* configure button pin as input */
    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_2);

//    /* enable and set key EXTI interrupt to the lowest priority */
//    nvic_irq_enable(EXTI2_IRQn, 2U, 0U);

//    /* connect key EXTI line to key GPIO pin */
//    syscfg_exti_line_config(EXTI_SOURCE_GPIOB, EXTI_SOURCE_PIN2);

//   /* configure key EXTI line */
//    exti_init(EXTI_2, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
//    exti_interrupt_flag_clear(EXTI_2);
	}
	
		  /*PB12：timer0_CH3*/
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_12);
    gpio_af_set(GPIOB, GPIO_AF_2, GPIO_PIN_12);
	
		//PB13:GPIO INPUT
    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_13);
	
		//PA8:IICSCL PB15:IIC_SDA
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_8);
    gpio_af_set(GPIOA, GPIO_AF_6, GPIO_PIN_8);

    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_15);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_15);
    gpio_af_set(GPIOB, GPIO_AF_6, GPIO_PIN_15);
	
		//PA12: EXTI INPUT
	{
//		rcu_periph_clock_enable(RCU_GPIOA);
//    rcu_periph_clock_enable(RCU_SYSCFG);

    /* configure button pin as input */
    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_12);

//    /* enable and set key EXTI interrupt to the lowest priority */
//    nvic_irq_enable(EXTI10_15_IRQn, 2U, 0U);

//    /* connect key EXTI line to key GPIO pin */
//    syscfg_exti_line_config(EXTI_SOURCE_GPIOA, EXTI_SOURCE_PIN12);

//   /* configure key EXTI line */
//    exti_init(EXTI_12, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
//    exti_interrupt_flag_clear(EXTI_12);
	}
	
		// PC6: GPIO OUTPUT.
	gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_6);
	gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_6);
	
		//PC7:GPIO INPUT
	gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_7);
	

}


/**
  * @brief  De-initialize the GPIOx peripheral registers to their default reset values.
  * @param  GPIOx where x can be (A..F) to select the GPIO peripheral for STM32G0xx family
  * @param  GPIO_Pin specifies the port bit to be written.
  *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @retval None
  */
void hwi_GPIO_DeInit(void)
{
	
	gpio_deinit(GPIOA);
	gpio_deinit(GPIOB);
	gpio_deinit(GPIOC);
}


/** @addtogroup GPIO_Exported_Functions_Group2
 *  @brief GPIO Read, Write, Toggle, Lock and EXTI management functions.
 *
@verbatim
 ===============================================================================
                       ##### IO operation functions #####
 ===============================================================================
@endverbatim
  * @{
  */
/**
  * @brief  Read the specified input port pin.
  * @param  GPIOx where x can be (A..F) to select the GPIO peripheral for STM32G0xx family
  * @param  GPIO_Pin specifies the port bit to read.
  *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @retval The input port pin value.
  */
hwi_GPIO_Value hwi_GPIO_ReadPin(hwi_GPIO_Pin GPIO_Pin)
{
	  hwi_GPIO_Value bitstatus;
	//其他引脚可参考以下代码根据实际使用增加。
	if(GPIO_Pin == BATTERY_STACK_INT_E)
	{
		if(gpio_input_bit_get(GPIOC, GPIO_PIN_6)==SET)
		{
		bitstatus = HWI_PIN_SET;
		}
		else{
			bitstatus = HWI_PIN_RESET;
		}
	}
  return bitstatus;
}

/**
  * @brief  Set or clear the selected data port bit.
  *
  * @note   This function uses GPIOx_BSRR and GPIOx_BRR registers to allow atomic read/modify
  *         accesses. In this way, there is no risk of an IRQ occurring between
  *         the read and the modify access.
  *
  * @param  GPIOx where x can be (A..F) to select the GPIO peripheral for STM32G0xx family
  * @param  GPIO_Pin specifies the port bit to be written.
  *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @param  PinState specifies the value to be written to the selected bit.
  *         This parameter can be one of the GPIO_PinState enum values:
  *            @arg GPIO_PIN_RESET: to clear the port pin
  *            @arg GPIO_PIN_SET: to set the port pin
  * @retval None
  */
void hwi_GPIO_WritePin(hwi_GPIO_Pin GPIO_Pin, hwi_GPIO_Value Value)
{
	//其他引脚可参考以下代码根据实际使用增加。
	if(GPIO_Pin == HEATING_OFF_E)
	{
		if(Value == HWI_PIN_SET)
		{
			gpio_bit_set(GPIOC, GPIO_PIN_7);
		}
		else
		{
			gpio_bit_reset(GPIOC, GPIO_PIN_7);
		}
	}
	else if (GPIO_Pin == 100)
	{
		if(Value == HWI_PIN_SET)
		{
			gpio_bit_set(GPIOB, GPIO_PIN_11);
		}
		else
		{
			gpio_bit_reset(GPIOB, GPIO_PIN_11);
		}	
	}
}

/**
  * @brief  Toggle the specified GPIO pin.
  * @param  GPIOx where x can be (A..F) to select the GPIO peripheral for STM32G0xx family
  * @param  GPIO_Pin specifies the pin to be toggled.
  *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @retval None
  */
void hwi_GPIO_TogglePin(hwi_GPIO_Pin GPIO_Pin)
{
		//其他引脚可参考以下代码根据实际使用增加。
	if(GPIO_Pin == HEATING_OFF_E)
	{
		gpio_bit_toggle(GPIOC, GPIO_PIN_7);
	}
}
//新增EXTI2和EXTI10_15中断服务函数
void EXTI2_IRQHandler(void)
{
    exti_interrupt_flag_clear(EXTI_2);
}
void EXTI10_15_IRQHandler(void)
{
	if(exti_interrupt_flag_get(EXTI_12) ==SET)
	{
		exti_interrupt_flag_clear(EXTI_12);
	}
	if(exti_interrupt_flag_get(EXTI_15) ==SET)
	{
		exti_interrupt_flag_clear(EXTI_15);
	}
}



