/******************************************************************************
*
*    Copyright (C), 2019-2020, xi'an byd Co., Ltd.
*
*******************************************************************************
* file name :
* description:

* author    :
* vertion   :
* data      :

*
*history version:
*      <author>         <date>           <version>      <description>
*
******************************************************************************/


/*******************************--includes--**********************************/
#include "string.h"
#include "HWI_Hal.h"

#include "flash.h"

#include "iap_config.h"

#include "ymodem.h"

#include "test_flash.h"

/*******************************--define--************************************/



/*****************************--type define--*********************************/

extern uint8_t Packet_Data[PACKET_128B_SIZE + PACKET_OVERHEAD];
extern int8_t Ymodem_ParseDataPacket(const uint8_t *dat,uint16_t length);


/***************************--global variable--*******************************/



/**************************--function declaration--***************************/


/*****************************************************************************/



/******************************************************************************
 * function name:
 * description:
 * input:
 * return:
 * exception:
******************************************************************************/
void Test_Flash_Program(void)
{
    uint64_t u64data = 0x12345678;
    uint32_t address = 0x0801FFF0;
    Flash_Program(address,u64data);

    while(1);
}


