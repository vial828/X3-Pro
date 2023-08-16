#include "kernel.h"
#include "log.h"
#include "update_cmd.h"
#include "self_flash.h"
#include "power.h"
#include "manager.h"
#include "usr_cmd.h"
#include "app_haptic.h"
#include "app_charge.h"
#include "error_code_led_output.h"
#include "HWI_power.h"
#include "gd25qxx.h"
#include "batTimer.h"
#include "taskinit.h"
#include "power.h"

static int8_t update_image(uint8_t cmd, uint8_t *pdata, uint16_t len);
static int8_t ParseDataPacket(const uint8_t *dat,uint16_t length);

ptimer_t reset_timer;
uint32_t UI_size_from_PC = 0;
uint32_t check_sum_from_PC = 0;
uint32_t sum_device = 0;
int8_t UI_update_status = 0;
uint32_t FlashDestination = EXFlash_ADDRESS;

uint8_t g_plain_packet_data[PLAIN_PACKET_DATA_SIZE] = {0};
uint32_t g_plain_packet_data_index = 0;

typedef struct
{
    uint8_t bat_left;
    uint8_t update_cutoff_volt_percentage;
}update_batv_t;

update_batv_t update_batv_s = {0, UPDATE_CUTOFF_VOLT_PERCENTAGE};
/*************************************************************************************************
 * @brief   :parse update command
 * @param   :command
 * @return  :none
*************************************************************************************************/
void parse_update_cmd(uint8_t cmd)
{
    if(cmd == UPDATE_FLAG_1P){ //reboot bootloader
        /*get battery soc and usb plug status*/
        update_batv_s.bat_left = app_get_bat_left();
        uint8_t chg_usb_status = app_check_usb_plug_status();
        /*make sure soc can support update consumption*/
        if(update_batv_s.bat_left < UPDATE_CUTOFF_VOLT_PERCENTAGE && chg_usb_status != WELL_USB_PLUG)
        {
            respond_usr_cmd(UPDATE_PROHIBITION_FLAG, (uint8_t*)&update_batv_s, sizeof(update_batv_s));
            return;
        }
        app_haptic_shutdown();
        /*change update flag for bootloader*/
        boot_record_t * brt = get_boot_record_from_ram();
        brt->app_update_flag = 0x12345;
        update_data_flash(BOOT_RECORD, INVALID);

        vTaskDelay(500);
        //SOFT_RESET();
        hwi_SOFT_RESET();
    }
}

void call_back_reset(const ptimer_t tm)
{
    if(reset_timer){
        if(bat_timer_delete(reset_timer, portMAX_DELAY)==pdPASS){
            reset_timer = NULL;
        }
    }
    power_soft_reset();

}
static void update_reset_timer(void){
    if(reset_timer){
        if(bat_timer_delete(reset_timer, portMAX_DELAY)==pdPASS){
            reset_timer = NULL;
        }
    }
    reset_timer = bat_timer_reset_ext(reset_timer, "reset_timer", 1500 , TIMER_OPT_ONESHOT , call_back_reset);
    bat_timer_start(reset_timer, portMAX_DELAY);
}

/*************************************************************************************************
 * @brief   : check sum in exflash
 * @param   :UI_size from first_frame
 * @param   :check_sum_from_PC from last_frame
 * @return  :ststus  1(success) 0(failed)
*************************************************************************************************/
static uint8_t exflash_check_sum(uint32_t UI_size,uint32_t check_sum_from_PC){
    uint32_t start_addr = 0x0;
    uint8_t flash_read_buffer[PACKET_2048B_SIZE];
    //uint32_t sum_device = 0;
    uint16_t ui_frames=0;
    if(UI_size % PACKET_2048B_SIZE == 0){
        ui_frames = UI_size / PACKET_2048B_SIZE;
    }else{
        ui_frames = UI_size / PACKET_2048B_SIZE + 1;
    }

    for (uint16_t i = 0;i<ui_frames;i++){
    //    if (i%50==0){
    //        LOGD("UI check frame = %d",i);
    //    }
        spi_flash_buffer_read_not_dma(&flash_read_buffer[0],start_addr+PACKET_2048B_SIZE*i,PACKET_2048B_SIZE);
        for(int j = 0;j<PACKET_2048B_SIZE;j++){
            sum_device = sum_device + flash_read_buffer[j];
        }
        memset(flash_read_buffer, 0, PACKET_2048B_SIZE);
    }
    
    if (sum_device == check_sum_from_PC){
        return 1;
    }else{
        return 0;
    }
}

void parse_image_update_cmd(uint8_t cmd, uint8_t *pdata, uint16_t len)
{
    uint8_t check_result = 0;

    uint8_t data[1]= {0};
    if(cmd == 0xB8){
        LogSuspend();//disable debug log
        set_cycle_log_flag(0);//disable cycle log
#ifdef SSCOM
        stop_comm_lock_timer();
#endif
        vTaskDelay(100);
        UartInit();
        SuspendCheckTask();
        //cy_serial_flash_qspi_enable_xip(false);
        post_msg_to_manager(op_UI_updating);
        spi_flash_bulk_erase();//erase the entire flash
        //NVIC_DisableIRQ(pass_interrupt_sar_IRQn);// disable ADC
        vTaskDelay(200);
        comm_send(cmd,0x00,data,0);
    }
    else{
        //vTaskDelay(100);
        UI_update_status = update_image(cmd, pdata, len);
        if(UI_update_status == 1){
            //LogResume();
            //set_cycle_log_flag(1);//enable cycle log
            check_result = exflash_check_sum(UI_size_from_PC,check_sum_from_PC);
            if(check_result==1){
                data[0]= 1;
                comm_send_now(UPDATE_CHECKSUM_FLAG,0x00,data,1);
                //LOGD("UI imge update ok,UI_size_from_PC=%d",UI_size_from_PC);
                update_reset_timer();
            }else{
                data[0]= 2;
                comm_send_now(UPDATE_CHECKSUM_FLAG,0x00,data,1);
                //LOGD("UI imge update fail,UI_size_from_PC=%d,sum_device = %d,check_sum_from_PC = %d",UI_size_from_PC,sum_device,check_sum_from_PC);
                spi_flash_bulk_erase();//erase the entire flash
                update_reset_timer();
            }
        }
    }
}

static int8_t update_image(uint8_t cmd, uint8_t *pdata, uint16_t len)
{
    uint8_t data[2]= {0};
    static uint16_t  packets_received = RX_UPDATE_START;
    int8_t  ret = 0;
    update_reset_timer();
    if(cmd == RX_UPDATE_START){
        //ParseFirstPacket
        ret = 0;
        //errorCode = ParseFirstPacket(pdata);
        UI_size_from_PC = ((pdata[0]<<24)|(pdata[1]<<16)|(pdata[2]<<8)|pdata[3]);
        FlashDestination = EXFlash_ADDRESS;
        packets_received = RX_UPDATE_START+1;
    }
    else if(cmd == RX_UPDATE_DONE){
        if(reset_timer){
            if(bat_timer_delete(reset_timer, portMAX_DELAY)==pdPASS){
                reset_timer = NULL;
            }
        }
        //ParseLastPacket
        ret = 1;
        check_sum_from_PC = ((pdata[0]<<24)|(pdata[1]<<16)|(pdata[2]<<8)|pdata[3]);
        FlashDestination = EXFlash_ADDRESS;
        packets_received = RX_UPDATE_START;
        comm_send_now(cmd,0x00,data,0);
        vTaskDelay(1);
        return ret;
    }
    else /*packet receive*/
    {
        if (cmd != packets_received){
            ret = ERR_PACKET_NUM;  // packet Number lost
        }
        else{
            if(ParseDataPacket(pdata,PACKET_2048B_SIZE) != 0){
                ret = ERR_FLASH_PROGRAM;  // Program Flash error
                comm_send(ret,0x00,data,0);
                power_soft_reset();
            }
            else{
                packets_received ++;
                if(packets_received > 0xAF)
                {
                    packets_received = 0xA1;
                }
            }
        }
    }
    if(ret == 0){
        comm_send(cmd,0x00,data,0);
//      comm_send(errorCode,0x00,data,0);
        //vTaskDelay(10);
    }
    else{//when  errors occur
        comm_send(ret,0x00,data,0);
        //vTaskDelay(10);
    }
    return ret;
}

static int8_t ParseDataPacket(const uint8_t *dat,uint16_t length)
{
    int8_t ret = 0;
    uint32_t i = 0;

    uint8_t plainData[PACKET_2048B_SIZE] = {0};
    memcpy(plainData, dat,length);

   // for (i = 0; i < length; ++i)
   // {
   //     g_plain_packet_data[g_plain_packet_data_index++] = plainData[i];

   //     if (PLAIN_PACKET_DATA_SIZE == g_plain_packet_data_index)
   //     {
            //uint32_t write_status = cy_serial_flash_qspi_write(FlashDestination, PLAIN_PACKET_DATA_SIZE,g_plain_packet_data);
            spi_flash_buffer_write(plainData,FlashDestination,PLAIN_PACKET_DATA_SIZE);
            g_plain_packet_data_index = 0;
            memset(plainData, 0, PLAIN_PACKET_DATA_SIZE);
            //if (HWI_OK == write_status)
            //{
            //    ret = 0;
                FlashDestination += PLAIN_PACKET_DATA_SIZE;
            //}
            //else
            //{
            //    ret = ERR_FLASH_PROGRAM;  /* program failed */
            //    break;
            //}
//        }
//    }

    return ret;
}

