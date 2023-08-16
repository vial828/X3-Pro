#ifndef __APP_OLED_DISPLAY_H
#define __APP_OLED_DISPLAY_H

#include "stdint.h"
#include "message.h"

void post_msg_to_oled(opcode_e opcode);
void post_msg_to_oled_with_arg(opcode_e opcode, uint32_t arg);
void oled_display_init(void);
void oled_display_task(void);
void oled_queue_reset(void);

#endif
