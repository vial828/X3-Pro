#include "uart.h"
#include "log.h"
#include "string.h"
#include "message.h"
#include "manager.h"
#include "uart_cmd.h"
#include "dev_adc.h"

#define CMD_BUFFER_SIZE   1

#if 0
typedef struct {
    char cmd[32];
    opcode_e msg_op;
}cmd_t;

static rx_buffer cmd_buffer[CMD_BUFFER_SIZE];
static cmd_t cmd_map[] = {
    {"op_button",                   op_button},
    {"op_button_3s",                op_button_3s},
    {"op_button_3s_up",         op_button_3s_up},
    {"op_heat_finish",      op_heat_finish},
    {"op_heat_end_soon",     op_heat_end_soon},
    {"op_entry_cit",        op_entry_cit},
    {"op_exit_cit",         op_exit_cit},
};

#if 0
void UartRxFun(uint8_t * buffer,uint32_t length)
{
    for(uint8_t i = 0;i< CMD_BUFFER_SIZE;i++)
    {
        if(cmd_buffer[i].length == 0)
        {
                memcpy( (uint8_t *)cmd_buffer[i].buffer,buffer,length);
                cmd_buffer[i].length = length;
                break;
        }
    }
}
#endif

static void handle_cmd(char * buffer)
{
    uint16_t size = ARRAY_SIZE_OF(cmd_map);

    for(uint16_t i=0; i<size; i++){
        if(!strcmp(cmd_map[i].cmd, buffer)){
            post_msg_to_manager(cmd_map[i].msg_op);
            break;
        }
    }
}

static void handle_test(char * buffer)
{
#if 0
    if(!strcmp("led1 flash", buffer)){
        led_set_para(led_num1, led_flash1);
    }else if(!strcmp("led1 breath1", buffer)){
        led_set_para(led_num1, led_breath1);
    }else if(!strcmp("led1 breath2", buffer)){
        led_set_para(led_num1, led_breath2);
    }else if(!strcmp("led1 bright", buffer)){
        led_set_para(led_num1, led_bright);
    }else if(!strcmp("led1 dark", buffer)){
        led_set_para(led_num1, led_dark);
    }else if(!strcmp("led1 fade on", buffer)){
        led_set_para(led_num1, led_fade_on);
    }else   if(!strcmp("led2 flash", buffer)){
        led_set_para(led_num2, led_flash1);
    }else if(!strcmp("led2 breath1", buffer)){
        led_set_para(led_num2, led_breath1);
    }else if(!strcmp("led2 breath2", buffer)){
        led_set_para(led_num2, led_breath2);
    }else if(!strcmp("led2 bright", buffer)){
        led_set_para(led_num2, led_bright);
    }else if(!strcmp("led2 dark", buffer)){
        led_set_para(led_num2, led_dark);
    }else if(!strcmp("led2 fade on", buffer)){
        led_set_para(led_num2, led_fade_on);
    }else   if(!strcmp("led3 flash", buffer)){
        led_set_para(led_num3, led_flash1);
    }else if(!strcmp("led3 breath1", buffer)){
        led_set_para(led_num3, led_breath1);
    }else if(!strcmp("led3 breath2", buffer)){
    	led_set_para(led_num3, led_breath2);
    }else if(!strcmp("led3 bright", buffer)){
    	led_set_para(led_num3, led_bright);
    }else if(!strcmp("led3 dark", buffer)){
        led_set_para(led_num3, led_dark);
    }else if(!strcmp("led3 fade on", buffer)){
        led_set_para(led_num3, led_fade_on);
    }else if(!strcmp("led1 fade off", buffer)){
        led_set_para(led_num1, led_fade_off);
    }else if(!strcmp("led2 fade off", buffer)){
        led_set_para(led_num2, led_fade_off);
    }else if(!strcmp("led3 fade off", buffer)){
        led_set_para(led_num3, led_fade_off);
    }else if(!strcmp("read adc", buffer)){
        //LOGD("VBUS_VOLT:%f V\r\n", dev_get_voltage(VBUS_VOLT));
        LOGD("VBAT_VOLT:%f V\r\n", dev_get_voltage(VBAT_VOLT));
        //LOGD("COIL_1_TEMP:%f V\r\n", dev_get_voltage(COIL_1_TEMP));
        //LOGD("COIL_2_TEMP:%f V\r\n", dev_get_voltage(COIL_2_TEMP));
        LOGD("USB_TEMP:%f V\r\n", dev_get_voltage(USB_TEMP));
        LOGD("COLD_JUNC:%f V\r\n", dev_get_voltage(COLD_JUNC));
        LOGD("BAT_TEMP:%f V\r\n", dev_get_voltage(BAT_TEMP));
        LOGD("I_SENSE:%f V\r\n", dev_get_voltage(I_SENSE));
        LOGD("TC1:%f V\r\n", dev_get_voltage(TC1));
        LOGD("TC2:%f V\r\n", dev_get_voltage(TC2));
    }
    #endif
}

void uart_cmd_task(void)
{
    if(cmd_buffer[0].length != 0){
        cmd_buffer[0].length = 0;  //clear the length for next use
//      LOGD("%s\r\n", cmd_buffer[0].buffer);
        handle_cmd((char *)cmd_buffer[0].buffer);
        handle_test((char *)cmd_buffer[0].buffer);
        memset(cmd_buffer[0].buffer,0,UART_RX_SIZE);
    }
}
#endif


