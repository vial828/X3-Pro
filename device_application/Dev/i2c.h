#ifndef _I2C_H
#define _I2C_H

#include "stdint.h"
//extern I2C_HandleTypeDef hi2c1;
#define I2C_MEMADD_SIZE_8BIT            (0x00000001U)
#define I2C_MEMADD_SIZE_16BIT           (0x00000002U)

#define CHANNEL  1


void MX_I2C1_Init(void);
void HAL_I2C_MspInit(uint8_t channel);


#endif
