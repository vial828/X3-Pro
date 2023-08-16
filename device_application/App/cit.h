#ifndef __CIT_H
#define __CIT_H

#include "stdint.h"

typedef enum{
    OLED_ID_1 = 0,
    OLED_ID_2,
}OLED_TYPE_E;

void clear_cit_mode_flag(void);
void set_cit_mode_flag(void);
void parse_cit_cmd(uint8_t cmd, uint8_t *pdata, uint16_t len);
void read_smt_sn(uint8_t *pdata);
#endif


