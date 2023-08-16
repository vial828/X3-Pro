#include "HWI_i2c.h"

#define MAX_RELOAD_SIZE          (255)

typedef enum
{
    I2C_START = 0,
    I2C_SEND_ADDRESS,
    I2C_RESTART,
    I2C_TRANSMIT_DATA,
    I2C_RELOAD,
    I2C_STOP,
    I2C_END
}i2c_process_enum;



static void i2c_bus_reset()
{
    hwi_I2C_Init(1);  
    /* configure SDA/SCL for GPIO */
    GPIO_BC(GPIOB) |= GPIO_PIN_15;
    GPIO_BC(GPIOA) |= GPIO_PIN_8;
    /* reset PB15 and PA8 */
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_15);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_15);
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_8);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_8);
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    /* stop signal */
    GPIO_BOP(GPIOB) |= GPIO_PIN_15;
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    GPIO_BOP(GPIOA) |= GPIO_PIN_8;
    /* connect PB15 to I2C1_SCL */
    /* connect PA8 to I2C1_SDA */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_15);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, GPIO_PIN_15);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_8);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, GPIO_PIN_8);    
}

void hwi_I2C_Init(uint8_t channel)
{
	if(1 == channel)
	{
		rcu_periph_clock_enable(RCU_I2C1);
    i2c_deinit(I2C1);
		/* configure I2C timing */
		i2c_timing_config(I2C1, 0, 0x4, 0);
		i2c_master_clock_config(I2C1, 0x1A, 0x4F);
		/* enable I2C1 */
		i2c_enable(I2C1);
	}
}

void hwi_I2C_MspInit(uint8_t channel)
{
    /* i2c gpio init */
	if(1 == channel)
	{
		/* enable GPIOB clock */
		rcu_periph_clock_enable(RCU_GPIOB);
		/* enable GPIOC clock */
		rcu_periph_clock_enable(RCU_GPIOA);
		
		/* connect PA8 to I2C1_SDA */
		gpio_af_set(GPIOA, GPIO_AF_6, GPIO_PIN_8);
		/* connect PB15 to I2C1_SCL */
		gpio_af_set(GPIOB, GPIO_AF_6, GPIO_PIN_15);
		/* configure GPIO pins of I2C1 */
		gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_8);
		gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, GPIO_PIN_8);
		gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_15);
		gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, GPIO_PIN_15);
	}
}

HWI_StatusTypeDef hwi_I2C_Mem_Write(uint8_t channel, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    i2c_process_enum state = I2C_START;
    uint16_t timeout = 0;
    uint8_t end_flag = 0;
    
    while(!end_flag){
        switch(state){
        case I2C_START:
            /* configure slave address */
            i2c_master_addressing(I2C1, DevAddress<<1, I2C_MASTER_TRANSMIT);
            /* configure number of bytes to be transferred */
            i2c_transfer_byte_number_config(I2C1, Size+1);
            /* clear I2C_TDATA register */
            I2C_STAT(I2C1) |= I2C_STAT_TBE;
            /* enable I2C automatic end mode in master mode */
            i2c_automatic_end_enable(I2C1);
            /* i2c master sends start signal only when the bus is idle */
            while(i2c_flag_get(I2C1, I2C_FLAG_I2CBSY) && (timeout < Timeout)){
                timeout++;
            }
            if(timeout < Timeout){
                i2c_start_on_bus(I2C1);
                
                timeout = 0;
                state = I2C_SEND_ADDRESS;
            }else{
                /* timeout, bus reset */
                i2c_bus_reset();
                return HWI_TIMEOUT;
                //printf("i2c bus is busy in page write!\n");
            }
            break;

        case I2C_SEND_ADDRESS:
            /* wait until the transmit data buffer is empty */
            while((!i2c_flag_get(I2C1, I2C_FLAG_TBE)) && (timeout < Timeout)){
                timeout++;
            }
            if(timeout < Timeout){
                /* send the EEPROM's internal address to write to : only one byte address */
                i2c_data_transmit(I2C1, MemAddress);
                timeout = 0;
                state = I2C_TRANSMIT_DATA;
            }else{
                /* timeout, bus reset */
                i2c_bus_reset();
                return HWI_TIMEOUT;
                //printf("i2c master sends address timeout in page write!\n");
            }
            break;

        case I2C_TRANSMIT_DATA:
            while(Size--){
                /* wait until TI bit is set */
                while((!i2c_flag_get(I2C1, I2C_FLAG_TI)) && (timeout < Timeout)){
                    timeout++;
                }
                if(timeout < Timeout){
                    /* while there is data to be written */
                    i2c_data_transmit(I2C1, *pData);
                    /* point to the next byte to be written */
                    pData++; 
                    timeout = 0;
                    state = I2C_STOP;
                }else{
                    /* wait TI timeout */
                    /* timeout, bus reset */
                    i2c_bus_reset();
                    return HWI_TIMEOUT;
                    //printf("i2c master sends data timeout in page write!\n");
                }
            }
            break;

        case I2C_STOP:
            /* wait until the stop condition is finished */
            while((!i2c_flag_get(I2C1, I2C_FLAG_STPDET)) && (timeout < Timeout)){
                timeout++;
            }
            if(timeout < Timeout){
                /* clear STPDET flag */
                i2c_flag_clear(I2C1, I2C_FLAG_STPDET);
                timeout = 0;
                state = I2C_END;
                end_flag = 1;
                return HWI_OK;
            }else{
                //printf("i2c master sends stop signal timeout in page write!\n");
                /* timeout, bus reset */
                i2c_bus_reset();
                return HWI_TIMEOUT;              
            }
            break;

        default:
            /* default status */
            state = I2C_START;
            end_flag = 1;
            timeout = 0;
            //printf("i2c master sends start signal in page write!\n");
            break;
        }
    }
    i2c_bus_reset();
    return HWI_ERROR;    
}


HWI_StatusTypeDef hwi_I2C_Mem_Read(uint8_t channel, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    uint32_t nbytes_reload = 0;
    i2c_process_enum state = I2C_START;
    uint32_t timeout = 0;
    uint8_t end_flag = 0;
    uint8_t restart_flag = 0;
    uint8_t first_reload_flag = 1;

    while(!end_flag){
        switch(state){
        case I2C_START:
            if(0 == restart_flag){
                /* clear I2C_TDATA register */
                I2C_STAT(I2C1) |= I2C_STAT_TBE;
                /* configure slave address */
                i2c_master_addressing(I2C1, DevAddress<<1, I2C_MASTER_TRANSMIT);
                /* configure number of bytes to be transferred */
                i2c_transfer_byte_number_config(I2C1, 1);
                /* disable I2C automatic end mode in master mode */
                i2c_automatic_end_disable(I2C1);
                /* i2c master sends start signal only when the bus is idle */
                while(i2c_flag_get(I2C1, I2C_FLAG_I2CBSY) && (timeout < Timeout)){
                    timeout++;
                }
                if(timeout < Timeout){
                    i2c_start_on_bus(I2C1);
                    timeout = 0;
                    state = I2C_SEND_ADDRESS;
                }else{
                    /* timeout, bus reset */
                    i2c_bus_reset();
                    return HWI_TIMEOUT;                  
                    //printf("i2c bus is busy in read!\n");
                }
            }else{
                /* restart */
                i2c_start_on_bus(I2C1);
                restart_flag = 0;
                state = I2C_TRANSMIT_DATA;
            }
            break;

        case I2C_SEND_ADDRESS:
            /* wait until the transmit data buffer is empty */
            while((!i2c_flag_get(I2C1, I2C_FLAG_TBE)) && (timeout < Timeout)){
                timeout++;
            }
            if(timeout < Timeout){
                /* send the EEPROM's internal address to write to : only one byte address */
                i2c_data_transmit(I2C1, MemAddress);
                timeout = 0;
                state = I2C_RESTART;
            }else{
                i2c_bus_reset();
                return HWI_TIMEOUT;
                //printf("i2c master sends data timeout in read!\n");
            }
            break;

        case I2C_RESTART:
            /* wait until the transmit data buffer is empty */
            while((!i2c_flag_get(I2C1, I2C_FLAG_TC)) && (timeout < Timeout)){
                timeout++;
            }
            if(timeout < Timeout){
                /* configure the EEPROM's internal address to write to : only one byte address */
                i2c_master_addressing(I2C1, DevAddress<<1, I2C_MASTER_RECEIVE);
                /* enable I2C reload mode */
                i2c_reload_enable(I2C1);
                /* configure number of bytes to be transferred */
                timeout = 0;
                state = I2C_RELOAD;
                restart_flag = 1;
            }else{
                i2c_bus_reset();
                return HWI_TIMEOUT;
                //printf("i2c master sends EEPROM's internal address timeout in read!\n");
            }
            break;

        case I2C_RELOAD:
            if(Size > MAX_RELOAD_SIZE){
                Size = Size - MAX_RELOAD_SIZE;
                nbytes_reload = MAX_RELOAD_SIZE;
            }else{
                nbytes_reload = Size;
            }
            if(1 == first_reload_flag){
                /* configure number of bytes to be transferred */
                i2c_transfer_byte_number_config(I2C1, nbytes_reload);
                first_reload_flag = 0;
                state = I2C_START;
            }else{
                /* wait for TCR flag */
                while((!i2c_flag_get(I2C1, I2C_FLAG_TCR)) && (timeout < Timeout)){
                    timeout++;
                }
                if(timeout < Timeout){
                    /* configure number of bytes to be transferred */
                    i2c_transfer_byte_number_config(I2C1, nbytes_reload);
                    /* disable I2C reload mode */
                    if(Size <= MAX_RELOAD_SIZE){
                        i2c_reload_disable(I2C1);
                        /* enable I2C automatic end mode in master mode */
                        i2c_automatic_end_enable(I2C1);
                    }
                    timeout = 0;
                    state = I2C_TRANSMIT_DATA;
                }else{
                    i2c_bus_reset();
                    return HWI_TIMEOUT;
                    //printf("i2c master reload data timeout in read!\n");
                }
            }
            break;

        case I2C_TRANSMIT_DATA:
            /* wait until TI bit is set */
            while((!i2c_flag_get(I2C1, I2C_FLAG_TBE)) && (timeout < Timeout)){
                timeout++;
            }
            if(timeout < Timeout){
                while(nbytes_reload){
                    /* wait until the RBNE bit is set and clear it */
										/* wait until TI bit is set */
										timeout=0;
										while((!i2c_flag_get(I2C1, I2C_FLAG_RBNE)) && (timeout < Timeout)){
												timeout++;
										}
										if(timeout < Timeout){											
                        /* read a byte from the EEPROM */
                        *pData = i2c_data_receive(I2C1);
                        /* point to the next location where the byte read will be saved */
                        pData++;
                        /* decrement the read bytes counter */
                        nbytes_reload--;
                    }else{
												/* wait TI timeout */
												i2c_bus_reset();
												return HWI_TIMEOUT;
												//printf("i2c master read data timeout in read!\n");											
										}
								}
                timeout = 0;
                /* check if the reload mode is enabled or not */
                if(I2C_CTL1(I2C1) & I2C_CTL1_RELOAD){
                    timeout = 0;
                    state = I2C_RELOAD;
                }else{
                    timeout = 0;
                    state = I2C_STOP;
                }
            }else{
                /* wait TI timeout */
                i2c_bus_reset();
                return HWI_TIMEOUT;
                //printf("i2c master read data timeout in read!\n");
            }
            break;

        case I2C_STOP:
            /* wait until the stop condition is finished */
            while((!i2c_flag_get(I2C1, I2C_FLAG_STPDET)) && (timeout < Timeout)){
                timeout++;
            }
            if(timeout < Timeout){
                /* clear STPDET flag */
                i2c_flag_clear(I2C1, I2C_FLAG_STPDET);
                timeout = 0;
                state = I2C_END;
                end_flag = 1;
                return HWI_OK;                 
            }else{
                i2c_bus_reset();
                return HWI_TIMEOUT;
                //printf("i2c master sends stop signal timeout in read!\n");
            }
            break;

        default:
            /* default status */
            state = I2C_START;
            end_flag = 1;
            timeout = 0;
            //printf("i2c master sends start signal in read!\n");
            break;
        }
    }
    i2c_bus_reset();
    return HWI_ERROR;       
}
