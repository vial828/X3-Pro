#include "HWI_Hal.h"

power_on_reason_t power_on_reason_s = {0};



//WFI_CMD¡¢WFE_CMD
void hwi_power_sleep_mode(uint8_t slp_entry)
{
    /* enter power sleep mode */
  	pmu_to_sleepmode(slp_entry);
}

//WFI_CMD¡¢WFE_CMD
void hwi_power_stop_mode(uint32_t regulator, uint8_t stop_entry) 
{
    /* enter stop mode */
//pmu_to_deepsleepmode(PMU_LDO_NORMAL,PMU_LOWDRIVER_DISABLE,deepsleepmodecmd);
}

void hwi_power_standby_mode(void)
{
		if(hwi_ReadBackupRegister(BKP_DR5)==0xa55a)
		{
			hwi_WriteBackupRegister(BKP_DR5, 0);
			__disable_irq();
			pmu_wakeup_pin_enable(PMU_CS0_WUPEN1|PMU_CS0_WUPEN2|PMU_CS0_WUPEN3);
			pmu_flag_clear(PMU_FLAG_RESET_WAKEUP);
  
			pmu_to_standbymode();	
		}
		else if(hwi_ReadBackupRegister(BKP_DR5)==0xaaaa)
		{	
			hwi_WriteBackupRegister(BKP_DR5, 0xa55a);		
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
#endif
}
void hwi_power_shutdown_mode(void)  
{
#if 1
//    update_data_flash(DATA_CHANGE_FREQUENT,NONE_SESSION_DATA);
//    Dev_BQ25898X_EnableADC(0);
//    haptic_shutdown();
		
//    __HAL_RCC_PWR_CLK_ENABLE();
//#ifdef NICO
//    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_HIGH);
//#else
//    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_LOW);//PC13 start button
//#endif
//    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN4_LOW);//PA2 USB
//    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN6_LOW);//PB5 mode button
    //HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1_HIGH);
//    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF2);
////  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF1);
//    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF4);

//    HAL_PWR_EnableBkUpAccess();

//  app_pwm_set_duty(pwm_dac, 0);
//  TAMP->BKP0R = 0x1234;

    /*
       Write 0x1234 when start running: Device is running if reset
       Write 0x5678 when get into shutdown: Device is shutdown if reset
	     Maybe user want to HW reset if start btn has been pressed
    */
//	while(read_button(start_btn) == btn_down){
//		hwi_HAL_Delay(20);
//		FeedIwdg();
//	}
		hwi_WriteBackupRegister(BKP_DR2, 0x5678);
		hwi_WriteBackupRegister(BKP_DR5, 0xaaaa);
    hwi_HAL_Delay(500);
    hwi_power_standby_mode();
    #endif
}
