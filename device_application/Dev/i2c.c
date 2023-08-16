#include "HWI_Hal.h"


#include "i2c.h"
#include "log.h"


//I2C_HandleTypeDef hi2c1;


/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
void MX_I2C1_Init(void)
{
	hwi_I2C_Init(CHANNEL);

//  /** Configure Analogue filter 
//  */
//  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_DISABLE) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  /** Configure Digital filter 
//  */
//  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  /* USER CODE BEGIN I2C2_Init 2 */
//
//  /* USER CODE END I2C2_Init 2 */

}


void HAL_I2C_MspInit(uint8_t channel)
{
	hwi_I2C_MspInit(CHANNEL);
}



