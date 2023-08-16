#include "HWI_Hal.h"
#include "usr_cmd.h"
#include "rtc.h"

power_on_reason_t power_on_reason_s = {0};




//WFI_CMD,WFE_CMD
void hwi_power_sleep_mode(uint8_t slp_entry)
{
    /* enter power sleep mode */
  	pmu_to_sleepmode(slp_entry);
}

//WFI_CMD,WFE_CMD
void hwi_power_stop_mode(uint32_t regulator, uint8_t stop_entry) 
{
    /* enter stop mode */
//pmu_to_deepsleepmode(PMU_LDO_NORMAL,PMU_LOWDRIVER_DISABLE,deepsleepmodecmd);
}

void hwi_power_standby_mode(void)
{
		if(ReadBackupRegister(BKP_DR5)==0xa55a)
		{
			WriteBackupRegister(BKP_DR5, 0);
			__disable_irq();
			pmu_wakeup_pin_enable(PMU_CS0_WUPEN1|PMU_CS0_WUPEN2|PMU_CS0_WUPEN3);
			pmu_flag_clear(PMU_FLAG_RESET_WAKEUP);
  
			pmu_to_standbymode();		
		}
		else if(ReadBackupRegister(BKP_DR5)==0xaaaa)
		{	
			WriteBackupRegister(BKP_DR5, 0xa55a);		
			NVIC_SystemReset();
		}

    /* enter standby mode */
}

void hwi_SOFT_RESET(void)
{
	__NVIC_SystemReset();
}


void hwi_power_on_reason(void)
{
#if 1
    uint32_t reason = 0;		

		reason = RCU_RSTSCK;
		rcu_all_reset_flag_clear();
    if(hwi_ReadBackupRegister(BKP_DR1) != 0x12345678)
    {
        hwi_WriteBackupRegister(BKP_DR1,0x12345678);
        power_on_reason_s.COLD_BOOT = 1;
        return;
    }
		

    if ((reason&RCU_RSTSCK_FWDGTRSTF) != RESET)
    {
        power_on_reason_s.IWDG_RST = 1;
        return;
    }
    if ((reason&RCU_RSTSCK_SWRSTF) != RESET)
    {
        power_on_reason_s.SFT_RST = 1;
        return;
    }
    if ((reason&RCU_RSTSCK_OBLRSTF) != RESET)
    {
        power_on_reason_s.OBL_RST = 1;
        return;
    }
    if ((reason&RCU_RSTSCK_EPRSTF) != RESET)
    {
        power_on_reason_s.PIN_RST = 1;
        return;
    }
#endif
}

#if 0
uint8_t hwi_is_pinrst(void)
{
     uint32_t reason = RCU_RSTSCK;
	  if ((reason&RCU_RSTSCK_EPRSTF) != RESET)
		{
				return 1;
		}
		return 0;
}
#endif
void hwi_power_shutdown_mode(void)  
{
#if 1
    if(1 == get_no_shutdown_flag()){
        //LOGI("Shutdown is disabled, HW reset or FW upgrade will enable it.");
        return;
    }
//    update_data_flash(DATA_CHANGE_FREQUENT,NONE_SESSION_DATA);
//    Dev_BQ25898X_EnableADC(0);
//    app_haptic_shutdown();

//    __HAL_RCC_PWR_CLK_ENABLE();
//#ifdef NICO
//    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_HIGH);
//#else
//    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_LOW);//PC13 start button
//#endif
//    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN4_LOW);//PA2 USB
//    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN6_LOW);//PB5 mode button
//    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1_HIGH);
//    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF2);
////    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF1);
//    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF4);

//    HAL_PWR_EnableBkUpAccess();

//    dev_pwm_set_duty(pwm_dac, 0);
//    TAMP->BKP0R = 0x1234;

    /*
        Write 0x1234 when start running: Device is running if reset
        Write 0x5678 when get into shutdown: Device is shutdown if reset
        Maybe user want to HW reset if start btn has been pressed
    */
//	  while(app_read_button(start_btn) == btn_down){
//		  hwi_HAL_Delay(20);
//		  FeedIwdg();
//	  }
    WriteBackupRegister(BKP_DR2, 0x5678);
    WriteBackupRegister(BKP_DR5, 0xaaaa);

    hwi_power_standby_mode();
#endif
}
