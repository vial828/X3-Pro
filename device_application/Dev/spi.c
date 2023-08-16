#include "stm32g0xx_hal.h"
#include "spi.h"

#define CS_PORT      GPIOD
#define CS_PIN       GPIO_PIN_3

SPI_HandleTypeDef hspi1;

/******************************************************************************
 * function name:Dev_SPI_Init
 * description:Init device of SPI1 and SPI2
 * input:void
 * return:void
 * exception:
******************************************************************************/
void spi_init(void)
{
//		GPIO_InitTypeDef  GPIO_InitStruct;
//	
//	 /* charge enable GPIO pin configuration */
//		GPIO_InitStruct.Pin       = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
//		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
//		GPIO_InitStruct.Pull      = GPIO_NOPULL;
//	  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
//		GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
//  
//		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//	
//		GPIO_InitStruct.Pin       = CS_PIN;
//		GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_OD;
//		GPIO_InitStruct.Pull      = GPIO_NOPULL;
//		GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
//  
//		HAL_GPIO_Init(CS_PORT, &GPIO_InitStruct);
//	
//    __HAL_RCC_SPI1_CLK_ENABLE();
//    hspi1.Instance = SPI1;
//    hspi1.Init.Mode = SPI_MODE_MASTER;
//    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
//    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
//    hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
//    hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
//    hspi1.Init.NSS = SPI_NSS_SOFT;
//    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
//    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
//    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
//    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//    hspi1.Init.CRCPolynomial = 7;
//    hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
//    hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
//    if (HAL_SPI_Init(&hspi1) != HAL_OK)
//    {
//        //Error_Handler();
//    }
}

void set_spi_cs(uint8_t value)
{
//	if(value){
//		HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);;
//	}else{
//		HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);;
//	}
}

uint8_t spi_send_byte(uint8_t data)
{
//	uint8_t r_byte;
//	
//	if (HAL_SPI_TransmitReceive(&hspi1, &data, &r_byte, 1, 10) != HAL_OK) {
//		r_byte = 0xFF;
//	}
//	return r_byte;
}

HAL_StatusTypeDef spi_write(uint8_t *pdata,uint8_t len)
{
//    HAL_StatusTypeDef ret;
//    ret = HAL_SPI_Transmit(&hspi1,pdata,len,10);
//    return ret;
}

HAL_StatusTypeDef spi_read(uint8_t *pdata,uint8_t len)
{
//    HAL_StatusTypeDef ret;
//    ret = HAL_SPI_Receive(&hspi1,pdata,len,100);
//    return ret;
}

HAL_StatusTypeDef spi_write_read(uint8_t *txdata,uint8_t *rxdata,uint8_t len)
{
//	HAL_StatusTypeDef ret;
//	ret = HAL_SPI_TransmitReceive(&hspi1, txdata, rxdata, len, 100);
//	return ret;
}


