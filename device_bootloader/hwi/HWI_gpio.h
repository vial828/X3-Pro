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
    BATTERY_STACK_INT_E,
    HEATING_OFF_E,
    LED_SDB_E,
    USB_INT_E,
    SW_GND_E,
    DRV_NRST_E,
    EN_3V3_SW_E,
    SWITCH_IN2_E,
    CHRG_EN_E,
    SWITCH_IN1_E,
    EN_2V5_SW_E,
    BAT_ID_E
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

#endif

