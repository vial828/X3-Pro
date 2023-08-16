#ifndef POWER_H
#define POWER_H

#include "stdint.h"


void power_sleep_mode(uint8_t slp_entry);
void power_stop_mode(uint32_t regulator, uint8_t stop_entry);
void power_standby_mode(void);
void power_shutdown_mode(void);
void power_ship_mode(void);
void power_soft_reset(void);

#endif
