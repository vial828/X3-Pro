#include <stdlib.h>
#include <string.h>
#include "stm32g0xx_hal.h"
#include "kernel.h"
#include "m24c64.h"
#include "log.h"
#include "i2c.h"

#define EE_MODEL_NAME		"M24C64"
#define EE_DEV_ADDR			0xA0			/* 设备地址 */
#define EE_PAGE_SIZE		32				/* 页面大小(字节) */
#define EE_SIZE				(8*1024)		/* 总容量(字节) */
#define EE_ADDR_BYTES		2			 	/* 地址字节个数 */
#define I2C_WR              0
#define I2C_RD              1

#define GPIO_PORT_I2C	GPIOB			     
#define I2C_SCL_PIN		GPIO_PIN_6			 
#define I2C_SDA_PIN		GPIO_PIN_7			 

/* 定义读写SCL和SDA的宏 */
#define I2C_SCL_1()  HAL_GPIO_WritePin(GPIOB, I2C_SCL_PIN, GPIO_PIN_SET);					
#define I2C_SCL_0()  HAL_GPIO_WritePin(GPIOB, I2C_SCL_PIN, GPIO_PIN_RESET);

#define I2C_SDA_1()  HAL_GPIO_WritePin(GPIOB, I2C_SDA_PIN, GPIO_PIN_SET);
#define I2C_SDA_0()  HAL_GPIO_WritePin(GPIOB, I2C_SDA_PIN, GPIO_PIN_RESET);

#define I2C_SDA_READ()  (HAL_GPIO_ReadPin(GPIOB,I2C_SDA_PIN))
#define I2C_SCL_READ()  (HAL_GPIO_ReadPin(GPIOB,I2C_SCL_PIN))


static void i2c_Delay(void)
{
	uint8_t i;
	for (i = 0; i < 40; i++);
}
void i2c_Start(void)
{	
	I2C_SDA_1();
	I2C_SCL_1();
	i2c_Delay();
	I2C_SDA_0();
	i2c_Delay();
	
	I2C_SCL_0();
	i2c_Delay();
}

void i2c_Stop(void)
{
	I2C_SDA_0();
	I2C_SCL_1();
	i2c_Delay();
	I2C_SDA_1();
	i2c_Delay();
}
void i2c_SendByte(uint8_t _ucByte)
{
	uint8_t i;
	
	for (i = 0; i < 8; i++)
	{
		if (_ucByte & 0x80)
		{
			I2C_SDA_1();
		}
		else
		{
			I2C_SDA_0();
		}
		i2c_Delay();
		I2C_SCL_1();
		i2c_Delay();
		I2C_SCL_0();
		if (i == 7)
		{
			 I2C_SDA_1(); 	
		}
		_ucByte <<= 1;		
		i2c_Delay();
	}
}
uint8_t i2c_ReadByte(void)
{
	uint8_t i;
	uint8_t value;

	value = 0;
	for (i = 0; i < 8; i++)
	{
		value <<= 1;
		I2C_SCL_1();
		i2c_Delay();
		if (I2C_SDA_READ())
		{
			value++;
		}
		I2C_SCL_0();
		i2c_Delay();
	}
	return value;
}


uint8_t i2c_WaitAck(void)
{
	uint8_t re;

	I2C_SDA_1();	
//	i2c_Delay();
	I2C_SCL_1();	
	i2c_Delay();

	if (I2C_SDA_READ())	
	{
		re = 1;
	}
	else
	{
		re = 0;
	}

	I2C_SCL_0();
	i2c_Delay();
	return re;
}

void i2c_Ack(void)
{
	I2C_SDA_0();	
	i2c_Delay();
	I2C_SCL_1();	
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();
	I2C_SDA_1();	
}

void i2c_NAck(void)
{
	I2C_SDA_1();	
	i2c_Delay();
	I2C_SCL_1();	
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();
}

uint8_t i2c_CheckDevice(uint8_t _Address)
{
	uint8_t ucAck;

	if (I2C_SDA_READ() && I2C_SCL_READ())
	{
		i2c_Start();		

		i2c_SendByte(_Address | I2C_WR);
		ucAck = i2c_WaitAck();	

		i2c_Stop();			

		return ucAck;
	}
	return 1;	
}

uint8_t ee_CheckOk(void)
{
	if (i2c_CheckDevice(EE_DEV_ADDR) == 0)
	{
		return 1;
	}
	else
	{
		i2c_Stop();
		return 0;
	}
}

uint8_t ee_WriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize)
{
    #if 1
    uint8_t ret;
    ret = HAL_I2C_Mem_Write(&hi2c1,EE_DEV_ADDR,_usAddress,I2C_MEMADD_SIZE_16BIT,_pWriteBuf,_usSize,0xfff);
    
    return ret;
	#else
    uint16_t i,m;
	uint16_t usAddr;
	usAddr = _usAddress;
	for (i = 0; i < _usSize; i++)
	{
		if ((i == 0) || (usAddr & (EE_PAGE_SIZE - 1)) == 0)
		{
			i2c_Stop();
			for (m = 0; m < 1000; m++)
			{
				i2c_Start();
                i2c_SendByte(EE_DEV_ADDR | I2C_WR);
				if (i2c_WaitAck() == 0)
				{
					break;
				}
			}
			if (m  == 1000)
			{
				goto cmd_fail;	/* EEPROM器件写超时 */
			}
			if (EE_ADDR_BYTES == 1)
			{
				i2c_SendByte((uint8_t)usAddr);
				if (i2c_WaitAck() != 0)
				{
					goto cmd_fail;	
				}
			}
			else
			{
				i2c_SendByte(usAddr >> 8);
				if (i2c_WaitAck()!= 0)
				{
					goto cmd_fail;	
				}

				i2c_SendByte(usAddr);
				if (i2c_WaitAck() != 0)
				{
					goto cmd_fail;	
				}
			}
		}

		i2c_SendByte(_pWriteBuf[i]);

		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	
		}
		usAddr++;	
	}

	i2c_Stop();

	for (m = 0; m < 1000; m++)
	{
		i2c_Start();
		#if EE_ADDR_A8 == 1
			i2c_SendByte(EE_DEV_ADDR | I2C_WR | ((_usAddress >> 7) & 0x0E));	
		#else		
			i2c_SendByte(EE_DEV_ADDR | I2C_WR);	
		#endif

		if (i2c_WaitAck() == 0)
		{
			break;
		}
	}
	if (m  == 1000)
	{
		goto cmd_fail;	
	}
	i2c_Stop();	

	return 1;

cmd_fail: 
	i2c_Stop();
	return 0;
    #endif
}
uint8_t ee_ReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize)
{
    #if 1
    uint8_t ret;
    ret = HAL_I2C_Mem_Read(&hi2c1,EE_DEV_ADDR,_usAddress,I2C_MEMADD_SIZE_16BIT,_pReadBuf,_usSize,1000);
    
    return ret;
    
    #else
	uint16_t i;

	i2c_Start();
	i2c_SendByte(EE_DEV_ADDR | I2C_WR);	/* 此处是写指令 */
	if (i2c_WaitAck() != 0)
	{	
		goto cmd_fail;	
	}
	if (EE_ADDR_BYTES == 1)
	{
		i2c_SendByte((uint8_t)_usAddress);
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	
		}
	}
	else
	{
		i2c_SendByte(_usAddress >> 8);
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	
		}

		i2c_SendByte(_usAddress);
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	
		}
	}

	i2c_Start();

	
	i2c_SendByte(EE_DEV_ADDR | I2C_RD);	

	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	
	}

	for (i = 0; i < _usSize; i++)
	{
		_pReadBuf[i] = i2c_ReadByte();	

		if (i != _usSize - 1)
		{
			i2c_Ack();	
		}
		else
		{
			i2c_NAck();	
		}
	}
	i2c_Stop();

	return 1;	

cmd_fail: 
	i2c_Stop();
	return 0;
    #endif
}

void m24c64_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  __HAL_RCC_GPIOB_CLK_ENABLE();


  /*Configure GPIO pins : PB6 PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;						
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;				
  GPIO_InitStruct.Pull = GPIO_NOPULL;											
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;//IO口速度配置
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);									
}
