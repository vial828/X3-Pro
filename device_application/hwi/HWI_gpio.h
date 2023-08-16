#ifndef __HWI_GPIO_H
#define __HWI_GPIO_H

#include "HWI_Hal.h"



#ifndef STATE_ON
#define STATE_ON          (0U)
#endif

#ifndef STATE_OFF
#define STATE_OFF         (1U)
#endif

typedef enum
{
    PD_INT_E,
    HEATING_OFF_E,
    OLED_ID_E,
    USB_INT_E,
    CAP_INT_E,
    //BOOST_EN_E,
    CHRG_EN_E,
    SWITCH_IN_E,
    HALL_INT_DOOR_E,
    HALL_INT_MODE_E,
    EN_2V5_SW_E,
    HW_ID_BIT0_E,
    HW_ID_BIT1_E,
    HW_ID_BIT2_E,
    HW_ID_BIT3_E,
    GET_BAT_ID_E,
    //EN_7V6_SW_E,
    OLED_VDD_EN_E,
    EN_2V8_OLED_VDD_E,
    SWDIO_EN,
    SWCLK_EN,
    FULL_B_EN1,
    FULL_B_EN2
}hwi_GPIO_Pin;

typedef enum
{
  HWI_PIN_RESET = 0U,
  HWI_PIN_SET
} hwi_GPIO_Value;


void hwi_GPIO_Init(void);
void hwi_GPIO_DeInit(void);
hwi_GPIO_Value hwi_GPIO_ReadPin(hwi_GPIO_Pin GPIO_Pin);
void hwi_GPIO_WritePin(hwi_GPIO_Pin GPIO_Pin, hwi_GPIO_Value Value);
void swd_reinit(void);
uint8_t read_hw_id(void);
void hwi_oled_spi_GPIO_DeInit(void);
void hwi_oled_spi_GPIO_Init(void);

#endif

