#ifndef __COMM_H
#define __COMM_H

#include "stdint.h"

#define C_CASE_ADDR 0x12
#define PC_ADDR     0x13

#define CMD_COMM_START      0x10
#define CMD_COMM_END        0x3F
#define CMD_USR_START       0x40
#define CMD_USR_END         0x6F
#define CMD_CIT_START       0x70
#define CMD_CIT_END         0x9F
#define CMD_UPDATE_START    0xA0
#define CMD_UPDATE_END      0xBF
#define CMD_USR_EX_START    0xC0
#define CMD_USR_EX_END      0xCF

#define CMD_GET_CHALLENGE         0x10
#define CMD_SIG_VERIFY        0x11
#define CMD_COMM_LOCK       0x12

#define COMM_UNLOCK_FLAG    0xCC

void uart_lock_create(void);
uint8_t get_comm_lock(void);
void set_comm_lock(uint8_t flag);
void comm_init(void);
int8_t comm_send(uint8_t cmd, uint8_t des, uint8_t *pdata, uint16_t len);
void comm_send_now(uint8_t cmd, uint8_t des, uint8_t *pdata, uint16_t len);
void comm_task(void);
int16_t get_one_cmd(uint8_t * pbuf);
void comm_send_proc(void);

#endif

