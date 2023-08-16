#ifndef __HWI_POWER_H
#define __HWI_POWER_H

#include "stdint.h"

/* USER CODE BEGIN PTD */
typedef struct
{
    uint8_t COLD_BOOT   :  1;
    uint8_t OBL_RST     :  1;
    uint8_t IWDG_RST    :  1;
    uint8_t SFT_RST     :  1;
    uint8_t BTN_WKUP    :  1;
    uint8_t HW_RST      :  1;
    uint8_t PIN_RST     :  1;
    uint8_t RESERVE     :  1;
}power_on_reason_t;

extern power_on_reason_t power_on_reason_s;
#define SOFT_RESET()    NVIC_SystemReset()
void hwi_SOFT_RESET(void);
void hwi_power_sleep_mode(uint8_t slp_entry);
void hwi_power_stop_mode(uint32_t regulator, uint8_t stop_entry);
void hwi_power_standby_mode(void);
void hwi_power_shutdown_mode(void);
void hwi_power_on_reason(void);

#endif
