#include "uart.h"
#include "log.h"
#include <string.h>
#include "comm.h"
#include "kernel.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

uint8_t log_mask = LOG_ALL & (~LOG_DEBUG);
uint8_t log_mask_store;

//uint8_t log_mask = (LOG_FCT | LOG_DEBUG | LOG_INFO | LOG_ERR | LOG_ASSERT);

#define  UART_SEND_DATA(buffer,count)     UartSendDma((char *)buffer,count)
#define  UART_SEND_STATUS_CHECK()         UartStatusCheck()


/*************************************************************************************************
  * @brief    : Send debugging log to pc
  * @param1   : Log level
  * @param2   : Contents of log
  * @return   : None
*************************************************************************************************/
void log_print(uint8_t level, const char *fmt, ...)
{
    char temp_buffer[ONE_LOG_SIZE] = {0};
    uint8_t pre_len = 15;

//    memset(temp_buffer,0,ONE_LOG_SIZE);
    switch(level){
        case LOG_FCT:
            snprintf(temp_buffer, ONE_LOG_SIZE, "\r\n%010d,F:", GetTick());
            break;
        case LOG_DEBUG:
            snprintf(temp_buffer, ONE_LOG_SIZE, "\r\n%010d,D:", GetTick());
            break;
        case LOG_INFO:
            snprintf(temp_buffer, ONE_LOG_SIZE, "\r\n%010d,I:", GetTick());
            break;
        case LOG_ERR:
            snprintf(temp_buffer, ONE_LOG_SIZE, "\r\n%010d,E:", GetTick());
            break;
        /*case LOG_CUSTOM:
            snprintf(temp_buffer, ONE_LOG_SIZE, "\r\n");
            pre_len = 2;
            break;*/
        default:
            break;
    }
    //snprintf(temp_buffer, ONE_LOG_SIZE, "\r\n%010d:", GetTick());

    va_list ap;
    va_start(ap,fmt);
    vsnprintf(&temp_buffer[pre_len],ONE_LOG_SIZE-pre_len,fmt,ap);

    comm_send(0xD0, PC_ADDR, (uint8_t *)temp_buffer, strlen(temp_buffer));
    va_end(ap);
}

void LogSuspend(void){

    log_mask_store = log_mask;
    log_mask = LOG_NULL;
}

void LogResume(void){

    log_mask = log_mask_store;
}

