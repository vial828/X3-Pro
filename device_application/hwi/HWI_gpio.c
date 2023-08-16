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
    
    //PB8: timer3_CH2 HTR_EN_2
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_8);
    gpio_af_set(GPIOB, GPIO_AF_2, GPIO_PIN_8);
    
    //PB7:GPIO OUTPUT FULL_B_EN2
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_7);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_7);

    //PB1:GPIO OUTPUT FULL_B_EN1
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_1);

    //PB0: timer2_CH1 HTR_EN_1
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_0);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_0);
    gpio_af_set(GPIOB, GPIO_AF_2, GPIO_PIN_0);

    /*PB10: timer0_CH1 :haptic*/
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_10);
    gpio_af_set(GPIOB, GPIO_AF_8, GPIO_PIN_10);

    //PA15: GPIO INPUT:USB_INT
    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_15);

    //PA0/PA1/PA2/PA3/PA4/PC0/PC1/PC2/PC3: ADC input.
    gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_0|GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3|GPIO_PIN_4);
    gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_0| GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3);

    //PA5:GPIO OUTPUT EN_2V8_SW
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_5);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_5);

    //PA6:USART_TX;PA7:USART_RX.
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_6);
    gpio_af_set(GPIOA, GPIO_AF_10, GPIO_PIN_6);

    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_7);
    gpio_af_set(GPIOA, GPIO_AF_8, GPIO_PIN_7);

    //PC5  EXTI INPUT. CAP_INT
    //{
    //  rcu_periph_clock_enable(RCU_GPIOC);
    //  rcu_periph_clock_enable(RCU_SYSCFG);

    /* configure button pin as input */
      gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_5);

//    /* enable and set key EXTI interrupt to the lowest priority */
//      nvic_irq_enable(EXTI2_IRQn, 2U, 0U);

//    /* connect key EXTI line to key GPIO pin */
//      syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN15);

//   /* configure key EXTI line */
//      exti_init(EXTI_2, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
//      exti_interrupt_flag_clear(EXTI_2);
//	}

    //PC14: GPIO INPUT. BAT_ID
    gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_14);

    //PB5: GPIO OUTPUT. CHRG_EN_N
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_5);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ, GPIO_PIN_5);

    //PC4: GPIO INPUT. BATTERY_STACK_IN
    gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_4);

    //PB2: GPIO INPUT: SWITCH_IN
    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_2);

    //PB11:GPIO INPUT: BASE or BOOST MODE
    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE,GPIO_PIN_11);

    //PA12:GPIO INPUT: OPEN DOOR
    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_12);

    //PC15: GPIO INPUT: OLED_ID
    gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_15);

    //PA8:IICSCL PB15:IIC_SDA
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ,GPIO_PIN_8);
    gpio_af_set(GPIOA, GPIO_AF_6, GPIO_PIN_8);

    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_15);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ,GPIO_PIN_15);
    gpio_af_set(GPIOB, GPIO_AF_6, GPIO_PIN_15);

    /* OLED GPIO config:PC6/TE, RESX/PC7, AVDDEN/PB4, DCX/PB3*/
    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_7);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_7);

    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_3);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_3);

    gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE,GPIO_PIN_6);

    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_4);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_4);

    /* flash SPI GPIO config: SCK/PA11, MISO/PA10, MOSI/PA9*/
    gpio_af_set(GPIOA, GPIO_AF_0, GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_9);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_9);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_9);		
    /*FLASH_SPI_CS/PB6 */
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_6);

    hwi_oled_spi_GPIO_DeInit();
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

void hwi_oled_spi_GPIO_Init(void){

    /* OLED SPI GPIO config: SCK/PB13, MISO/PB14, MOSI/PB9 */
    gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_9 );
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_9 );
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_9 );

    /*OLED_SPI_CS/PB12*/
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ, GPIO_PIN_12);
}

void hwi_oled_spi_GPIO_DeInit(void)
{
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_9 | GPIO_PIN_12);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_9 | GPIO_PIN_12);
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
    hwi_GPIO_Value bitstatus = HWI_PIN_RESET;

    if(GPIO_Pin == PD_INT_E)  //PC4
    {
        if(gpio_input_bit_get(GPIOC, GPIO_PIN_4)==SET)
        {
            bitstatus = HWI_PIN_SET;
        }
        else{
            bitstatus = HWI_PIN_RESET;
        }
    }
    else if(GPIO_Pin == USB_INT_E)  //PA15
    {
        if(gpio_input_bit_get(GPIOA, GPIO_PIN_15)==SET)
        {
            bitstatus = HWI_PIN_SET;
        }
        else{
            bitstatus = HWI_PIN_RESET;
        }
    }
    else if(GPIO_Pin == CAP_INT_E)  //PC5
    {
        if(gpio_input_bit_get(GPIOC, GPIO_PIN_5)==SET)
        {
            bitstatus = HWI_PIN_SET;
        }
        else{
            bitstatus = HWI_PIN_RESET;
        }
    }
    else if(GPIO_Pin == SWITCH_IN_E)  //PB2
    {
        if(gpio_input_bit_get(GPIOB, GPIO_PIN_2)==SET)
        {
            bitstatus = HWI_PIN_SET;
        }
        else{
            bitstatus = HWI_PIN_RESET;
        }
    }
    else if(GPIO_Pin == HALL_INT_DOOR_E)  //PA12
    {
        if(gpio_input_bit_get(GPIOA, GPIO_PIN_12)==SET)
        {
            bitstatus = HWI_PIN_SET;
        }
        else{
            bitstatus = HWI_PIN_RESET;
        }
    }
    else if(GPIO_Pin == HALL_INT_MODE_E)  //PB11
    {
        if(gpio_input_bit_get(GPIOB, GPIO_PIN_11)==SET)
        {
            bitstatus = HWI_PIN_SET;
        }
        else{
            bitstatus = HWI_PIN_RESET;
        }
    }
    else if(GPIO_Pin == CHRG_EN_E)  //PB5
    {
        if(gpio_input_bit_get(GPIOB, GPIO_PIN_5)==SET)
        {
            bitstatus = HWI_PIN_SET;
        }
        else{
            bitstatus = HWI_PIN_RESET;
        }
    }
    else if(GPIO_Pin == GET_BAT_ID_E)  //PC14
    {
        if(gpio_input_bit_get(GPIOC, GPIO_PIN_14)==SET)
        {
            bitstatus = HWI_PIN_SET;
        }
        else{
            bitstatus = HWI_PIN_RESET;
        }
    }
    else if (GPIO_Pin == OLED_ID_E) //PC15
    {
        if(gpio_input_bit_get(GPIOC, GPIO_PIN_15)==SET)
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

//    else if (GPIO_Pin == BOOST_EN_E) //PC5
//    {
//        if(Value == HWI_PIN_SET)
//        {
//            gpio_bit_set(GPIOC, GPIO_PIN_5);
//        }
//        else
//        {
//            gpio_bit_reset(GPIOC, GPIO_PIN_5);
//        }
//    }
    if (GPIO_Pin == CHRG_EN_E)  //PB5
    {
        if(Value == HWI_PIN_SET)
        {
            gpio_bit_set(GPIOB, GPIO_PIN_5);
        }
        else
        {
            gpio_bit_reset(GPIOB, GPIO_PIN_5);
        }
    }
    else if (GPIO_Pin == EN_2V5_SW_E)  //PA5
    {
        if(Value == HWI_PIN_SET)
        {
            gpio_bit_set(GPIOA, GPIO_PIN_5);
        }
        else
        {
            gpio_bit_reset(GPIOA, GPIO_PIN_5);
        }
    }
//    else if (GPIO_Pin == EN_7V6_SW_E)  //PC14
//    {
//        if(Value == HWI_PIN_SET)
//        {
//            gpio_bit_set(GPIOC, GPIO_PIN_14);
//        }
//        else
//        {
//            gpio_bit_reset(GPIOC, GPIO_PIN_14);
//        }
//    }
    else if (GPIO_Pin == OLED_VDD_EN_E)  //PB4
    {
        if(Value == HWI_PIN_SET)
        {
            gpio_bit_set(GPIOB, GPIO_PIN_4);
        }
        else
        {
            gpio_bit_reset(GPIOB, GPIO_PIN_4);
        }
    }
    else if (GPIO_Pin == EN_2V8_OLED_VDD_E)  //PB6
    {
        if(Value == HWI_PIN_SET)
        {
            gpio_bit_set(GPIOB, GPIO_PIN_6);
        }
        else
        {
            gpio_bit_reset(GPIOB, GPIO_PIN_6);
        }
    }
    else if (GPIO_Pin == SWDIO_EN)  //PA13
    {
        if(Value == HWI_PIN_SET)
        {
            gpio_bit_set(GPIOA, GPIO_PIN_13);
        }
        else
        {
            gpio_bit_reset(GPIOA, GPIO_PIN_13);
        }
    }
    else if (GPIO_Pin == SWCLK_EN)  //PA14
    {
        if(Value == HWI_PIN_SET)
        {
            gpio_bit_set(GPIOA, GPIO_PIN_14);
        }
        else
        {
            gpio_bit_reset(GPIOA, GPIO_PIN_14);
        }
    }
    else if(GPIO_Pin == FULL_B_EN1)  //PB1
    {
        if(Value == HWI_PIN_SET)
        {
            gpio_bit_set(GPIOB, GPIO_PIN_1);
        }
        else
        {
            gpio_bit_reset(GPIOB, GPIO_PIN_1);
        }
    }
    else if(GPIO_Pin == FULL_B_EN2)  //PB7
    {
        if(Value == HWI_PIN_SET)
        {
            gpio_bit_set(GPIOB, GPIO_PIN_7);
        }
        else
        {
            gpio_bit_reset(GPIOB, GPIO_PIN_7);
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

/**
  * @brief  SWDIO and SWCLK init .
  * @param  NONE
  * @retval NONE
  */
void swd_reinit(void)
{
    /*Start by init the SWD pin to plain GPIO*/
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_13|GPIO_PIN_14);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ,GPIO_PIN_13|GPIO_PIN_14);

    /*Init GPIO to low */
    hwi_GPIO_WritePin(SWDIO_EN,HWI_PIN_RESET);
    hwi_GPIO_WritePin(SWCLK_EN,HWI_PIN_RESET);

    /*Define GPIO as SWD again to keep SWDIO low*/
    gpio_af_set(GPIOA, GPIO_AF_0, GPIO_PIN_13);
    gpio_af_set(GPIOA, GPIO_AF_0, GPIO_PIN_14);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13|GPIO_PIN_14);
}
/**
  * @brief  Read the HW_ID, .
  * @param  NONE
  * @retval The HW_ID value from 0b0000 - 0b1111(0x00-0x0F).
  */
uint8_t read_hw_id(void)
{

    uint8_t hwId = 0;

    if (HWI_PIN_SET == hwi_GPIO_ReadPin(HW_ID_BIT0_E))
    {
        hwId = HWI_PIN_SET;
    }
 
    if (HWI_PIN_SET == hwi_GPIO_ReadPin(HW_ID_BIT1_E))
    {
        hwId |= HWI_PIN_SET<<1 ;
    }

    if (HWI_PIN_SET == hwi_GPIO_ReadPin(HW_ID_BIT2_E))
    {
        hwId |= HWI_PIN_SET<<2 ;
    }

    if (HWI_PIN_SET == hwi_GPIO_ReadPin(HW_ID_BIT3_E))
    {
        hwId |= HWI_PIN_SET<<3 ;
    }

    return hwId;
 
}

