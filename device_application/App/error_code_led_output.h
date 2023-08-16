#ifndef __ERROR_CODE_LED_OUTPUT_H
#define __ERROR_CODE_LED_OUTPUT_H

#include "stdint.h"
#include "self_flash.h"

#define ERR_EXIST   0x6789
#define ERR_CLEAR   0x0000

//2min
#define ERR_EXIST_TIME   (1000*60*2)
#define ERR_UI_TIME   (5000)
#define ERR_QR_TIME   (10000)
#define ERR_LOW_BATV_TIME   (4000)

typedef enum
{
    RETURN_ERR = 0,
    RESET_ERR,
    HIGH_BATT_ERR,
    RECOVER_ERR,
    LOW_BATV_ERR,
    NO_SUCH_ERR
}err_type_e;

uint8_t get_error_pos(uint32_t value);
void start_error_led_output(err_type_e error_type);
uint8_t process_error_code(uint8_t errorType, uint32_t value);
err_type_e record_err_type(uint8_t errorPos);

void upload_error(errorCode_e error_code);
#endif
