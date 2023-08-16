#ifndef __UPDATE_CMD_H
#define __UPDATE_CMD_H

#include "stdint.h"
#include "string.h"

#define RX_UPDATE_START   0xA0
#define RX_UPDATE_DONE    0xB0

#define UPDATE_FLAG_1P                  0xB1
#define UPDATE_PROHIBITION_FLAG         0xB2
#define UPDATE_CHECKSUM_FLAG            0xB3

#define UPDATE_CUTOFF_VOLT_PERCENTAGE   20

#define PACKET_SEQNO_INDEX      (4)

#define FILE_NAME_LENGTH        (48)
#define FILE_SIZE_LENGTH        (4)
#define PACKET_2048B_SIZE        (2048)

#define NAK_TIMEOUT             (300)

#define EXFlash_ADDRESS             (0x000000)
#define PLAIN_PACKET_DATA_SIZE  2048

typedef enum
{
    ERR_RECV_TIMEOUT = 0xAB,
    ERR_PACKET_HEAD = -2,       //0xFE
    ERR_PACKET_TAIL = -3,       //0xFD
    ERR_PACKET_CRC = -4,        //0xFC
    ERR_PACKET_SIZE = -5,       //0xFB
    ERR_PACKET_SIZE_ZERO = -6,  //0xFA
    ERR_PACKET_NUM = -7,        //0xF9
    ERR_FILE_NAME = -8,         //0xF8
    ERR_FILE_SIZE = -9,         //0xF7
    ERR_FILE_CRC = -10,         //0xF6
    ERR_FLASH_ERASE = -11,      //0xF5
    ERR_FLASH_PROGRAM = -12,    //0xF4
    ERR_SECURITY_BOOT = -13,    //0xF3
    ERR_APP_VERSION = -14,      //0xF2
}Update_ErrTypeDef;

void parse_update_cmd(uint8_t cmd);
void parse_image_update_cmd(uint8_t cmd, uint8_t *pdata, uint16_t len);
#endif


