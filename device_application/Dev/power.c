#include "HWI_Hal.h"
#include "power.h"
#include "app_charge.h"
#include "kernel.h"
#include "error_code_led_output.h"
#include "dev_pwm_ll.h"
#include "self_flash.h"
#include "dev_bq25898x.h"
#include "mp2731.h"
#include "rtc.h"
#include "app_haptic.h"
#include "app_charge.h"
#include "usr_cmd.h"
#include "log.h"
#include "app_button.h"
#include "wdg.h"
#include "dev_oled_driver.h"
#include "FreeRTOS.h"
#include "task.h"

void power_sleep_mode(uint8_t slp_entry)
{
    hwi_power_sleep_mode( slp_entry);
}

void power_stop_mode(uint32_t regulator, uint8_t stop_entry)
{
    hwi_power_stop_mode(regulator, stop_entry);
}

void power_standby_mode(void)
{
    hwi_power_standby_mode();
}
static void pre_power_off(void){
    if(1 == get_no_shutdown_flag()){
        LOGD("Shutdown is disabled, HW reset or FW upgrade will enable it.");
        return;
    }
    update_data_flash(DATA_CHANGE_FREQUENT,NONE_SESSION_DATA);
    if(app_get_current_chrg_drv() == BQ25898E)
     {
        Dev_BQ25898X_EnableADC(0);
      }
     else if(app_get_current_chrg_drv() == MP2731)
     {
       Dev_MP2731_EnableADC(0);
     }
     else
     {
        LOGD("no chrg drv");
     }
    app_haptic_shutdown();

    /*
       Write 0x1234 when start running: Device is running if reset
       Write 0x5678 when get into shutdown: Device is shutdown if reset
         Maybe user want to HW reset if start btn has been pressed
    */
    while((hwi_GPIO_ReadPin(SWITCH_IN_E) ? btn_down:btn_up) == btn_down)
    {
        hwi_HAL_Delay(20);
#ifndef DISABLE_WDG
        FeedIwdg();
#endif
    }
    WriteBackupRegister(BKP_DR2, 0x5678);
//    hwi_delay_ms(100);
    dev_oled_power_off();

}
void power_shutdown_mode(void)
{
    pre_power_off();
    hwi_power_shutdown_mode();
}

void power_ship_mode(void)
{
    //erase_frquent_data_flash(FREQUENT_DATA_FLASH_ALL_PAGE);
    Dev_BQ25898X_BATFET_DIS();
}


void power_soft_reset(void)
{
    pre_power_off();
    hwi_SOFT_RESET();
}

