/**
  ******************************************************************************
  * @file    IAP/inc/ymodem.h
  * @author  MCD Application Team
  * @version V3.3.0
  * @date    10/15/2010
  * @brief   This file provides all the software function headers of the ymodem.c
  *          file.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _YMODEM_H_
#define _YMODEM_H_


#include <stdint.h>

/* Includes ------------------------------------------------------------------*/
#ifndef  COMMON_H
#define  COMMON_H

#include <stdint.h>
#include <string.h>

#define INSTR_NUM       5


/*Common Command*/
#define REQ_FW_VER       0x15

#define RX_UPDATE_START   0xA0
#define RX_UPDATE_DONE    0xB0
#define RX_SIGNATURE      0xC0

/*System Command*/

/*Maintain Command*/
#define  CMD_HEAD    0x7E
#define  CMD_VER     0x11
#define  CMD_SID     0x20
#define  CMD_OWNER   0x00
#define  CMD_TAIL    0x0D


#define  CMD_MIN_LEN  10

#define  RTN_OK                        0
#define  RTN_VERSION_ERR               1
#define  RTN_DATALEN_ERR               2
#define  RTN_CRCCHECK_ERR              3
#define  RTN_VALIDCMD_ERR              4
#define  RTN_CMDFMT_ERR                5
#define  RTN_RETURN_FAIL               6
#define  RTN_PARA_ERR                  7
#define  RTN_NO_NEED                   8

#define APP_FILE_NAME_HEAD    "_APP_S_"

typedef struct
{
    uint8_t mCommandInstr;
    uint8_t (*mFunc)(uint8_t*,uint16_t,uint8_t*,uint16_t*);
}COMMANDHANDLE;

typedef enum
{
    ERR_RECV_TIMEOUT = -85,     //0xAB
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
    ERR_USR_TO_FACTORY = -15,   //0xF1
}Update_ErrTypeDef;

extern struct RINGBUFF gUartRecvRingBuff;
extern uint8_t gUartRcvDataFlag;
extern uint32_t gUartRcvDataFlag_time;
extern uint32_t File_Size;

extern uint8_t u8_g_Commandexe_InstrExe(uint8_t* InstrBuf,uint16_t ParaLen,uint8_t* ReturnBuf,uint16_t* ReturnLen);

void vd_g_CommandExp_Init(void);
void vd_g_CommandExp_Handler(void);

#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

#define PACKET_HEADER           (7)
#define PACKET_TRAILER          (3)
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)

#define PACKET_SEQNO_INDEX      (4)

#define FILE_NAME_LENGTH        (48)
#define FILE_SIZE_LENGTH        (4)
#define PACKET_128B_SIZE        (128)

#define NAK_TIMEOUT             (300)
#define MAX_ERRORS              (5)

extern uint32_t FlashDestination;
extern uint8_t file_name[FILE_NAME_LENGTH];

extern uint8_t StartUpdateFlag;

void vd_g_CommandExp_PackSendData(uint8_t Command);

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int8_t IAP_Update (void);



#endif  /* _YMODEM_H_ */

/*******************(C)COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
