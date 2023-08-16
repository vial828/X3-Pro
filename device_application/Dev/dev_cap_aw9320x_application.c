/*
* Copyright : ©2021 Shanghai Awinic Technology Co., Ltd. All Rights Reserved
* Description: a demo of how to use aw9320x driver code.
*/
#include "dev_cap_aw9320x_application.h"
#include "dev_cap_aw9320x.h"
#include "log.h"
#include "HWI_i2c.h"
#include "batTimer.h"
#include "manager.h"
#include "HWI_gpio.h"
#include "usr_cmd.h"
struct aw9320x_hw_info aw9320x_hw_info = {
        .i2c_dev_addr = AW9320X_I2C_DEV_ADDR,
};

static int32_t awinic_i2c_write( uint8_t *buf, uint32_t len)
{
        uint32_t ret = 0;
        uint16_t addr = 0;
        uint8_t *data = NULL;
        addr = buf[0]<<8|buf[1];
        data = &buf[2];
        ret = hwi_I2C_Mem_Write(GPIO_CHANNEL,aw9320x_hw_info.i2c_dev_addr,addr,AW_REG_ADDR_LEN,data,len-2,0xfff);


        return (ret == 0) ? AW_OK : -AW_ERR;
}

static int32_t awinic_i2c_read(uint8_t *addr, uint8_t addr_len, uint8_t *data_buf, uint32_t data_len)
{
        uint32_t ret = 0;
        uint16_t addr_offset = 0;
        addr_offset = addr[0]<<8|addr[1];
        ret = hwi_I2C_Mem_Read(GPIO_CHANNEL,aw9320x_hw_info.i2c_dev_addr,addr_offset,addr_len,data_buf,data_len,0xfff);

        return (ret == 0) ? AW_OK : -AW_ERR;
}

/***************** delay ******************/
static void aw9320x_delay(uint32_t ms)
{
        vTaskDelay(ms);
}


struct aw9320x_func aw9320x_demo_func = {
        .i2c_func = {
                .i2c_r = awinic_i2c_read,
                .i2c_w = awinic_i2c_write,
        },
        .delay                     = aw9320x_delay,
};

void dev_aw93205_init(void)
{
        int ret = 0;

        ret = (int)aw9320x_init(&aw9320x_demo_func);
        if(ret < 0) {
               LOGD("some func has not initalized.");
               return;
        }
}
void send_cigar_dect_log_bytes(uint8_t *pdata, uint16_t len)
{
    comm_send(CIGAR_DECT_LOG, PC_ADDR, pdata, len);
}
void read_cigar_dect_to_pc(uint8_t type,uint32_t* data0,uint32_t* data1)
{
    uint8_t cap_buffer[11];
    cap_buffer[0] = type;
    cap_buffer[1] = 0x00;
    cap_buffer[2] = (*data0 >> 24) & 0xff;
    cap_buffer[3] = (*data0 >> 16) & 0xff;
    cap_buffer[4] = (*data0 >> 8) & 0xff;
    cap_buffer[5] = (*data0) & 0xff;
    cap_buffer[6] = 0x01;
    cap_buffer[7] = (*data1 >> 24) & 0xff;
    cap_buffer[8] = (*data1 >> 16) & 0xff;
    cap_buffer[9] = (*data1 >> 8) & 0xff;
    cap_buffer[10] = (*data1) & 0xff;
    send_cigar_dect_log_bytes(cap_buffer,11);
}

void dev_aw9320x_data_log(void)
{
    if((hwi_GPIO_ReadPin(HALL_INT_DOOR_E) == 1) && (1 == get_cycle_log_flag()) && (get_comm_lock() == 0))
    {
        uint32_t diff0[1] = {0};
        uint32_t diff1[1] = {0};
        uint32_t raw0[1] = {0};
        uint32_t raw1[1] = {0};
        aw9320x_diff_get0(diff0);
        aw9320x_diff_get1(diff1);
        aw9320x_raw_get0(raw0);
        aw9320x_raw_get1(raw1);
        read_cigar_dect_to_pc(1,diff0,diff1);
        read_cigar_dect_to_pc(2,raw0,raw1);
    }
}
void dev_aw9320x_self_cap_parasitic_data_get(void)
{
    float reg_data_cfg[1];
    aw9320x_self_cap_parasitic_data_get(reg_data_cfg);
}

void dev_aw9320x_process(void){
//      uint32_t diff[1] ;
    
//      aw9320x_diff_get(diff);
//    if((HWI_PIN_RESET == hwi_GPIO_ReadPin(CAP_INT_E))&&(flag1 == 0))
//    {
//             LOGD("The smoke insertion detection interrupt is triggered");
//             flag1 =1;
//    }
//    else if((HWI_PIN_SET == hwi_GPIO_ReadPin(CAP_INT_E))&&(flag2 == 1))
//        {
//        LOGD("cap is 1");
//         flag2 = 0;
//    }
}
int8_t dev_aw93205_id_get(void)
{
    int8_t ret = 0;
    uint32_t reg_val = 0;

    ret = aw9320x_i2c_read(REG_CHIPID, &reg_val);
    if (ret < 0) {
        return 0;
    }

    switch (reg_val)
    {
        case AW93205DNR_CHIP_ID:
            ret = 1;
        break;
        default:
            ret = 0;
        break;
    }

    return ret;
}

