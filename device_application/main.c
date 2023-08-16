/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "HWI_Hal.h"
#include "stratos_defs.h"
#include "version.h"
#include "log.h"
#include "kernel.h"
#include "uart_cmd.h"
#include "manager.h"
#include "app_button.h"
#include "app_haptic.h"
#include "performance.h"
#include "app_charge.h"
#include "dev_pwm_ll.h"
#include "dev_adc.h"
#include "app_heat.h"
#include "dev_temperature.h"
#include "wdg.h"
#include "self_flash.h"
#include "error_code.h"
#include <string.h>
#include "spi_flash.h"
#include "storage.h"
#include "rtc.h"
#include "error_code_led_output.h"
#include "usr_cmd.h"
#include "m24c64.h"
#include "i2c.h"
#include "dev_bq25898x.h"
#include "app_charge.h"
#include "app_button.h"
#include "app_pd_com.h"
#include "gd25qxx.h"
#include "taskInit.h"
#include "app_oled_UI.h"
#include "dev_oled_driver.h"
#include "dev_gauge_bq27z561r2.h"

void init_task(void * pvParameters);
/* USER CODE END Includes */


uint8_t btn_flag = btn_up;


/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */


//typedef struct _period_task
//{
//    uint16_t m_period;
//    FUNC    m_func;
//}P_TASK_T;

//typedef struct _scan_task
//{
//    FUNC    m_func;
//}S_TASK_T;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
void Test_Proc(void)
{
//  LOGD("D:hello world\n\r");
}
//const P_TASK_T P_Task_Array[] =
//{
//    //{5000, test_display_lifecycle_100_sessions},
//    //{5000, test_display_error},
//    //{1000, test_error_code},
//    //{5000, test_error_code_read_write},
//    //{5000, test_lifeCycle_read_write},
//    //{5000, test_100sessions_read_write},
//    //{1000, Test_Proc},
//    //{1000,testcharglog},
//    //{1000,test_idle_logs},
//    {16, record_life_cycle_task},
//#ifndef DISABLE_WDG
//    {1001, FeedIwdg},
//#endif
//    //{20, uart_cmd_task},
//    {5,  manager_task},
//    {20, app_button_task},
//    //{10, haptic_task},
//    {LED_UPDATE_PERIOD2,  app_led_process},
//    {16, app_charge_task},
//    {1000, app_charge_ic_task},
//    {1000, check_charge_task},
//    {16, check_heat_task},
//    {10000, app_CheckChgICRegData},
//    {8,app_pd_process},
//};

//const S_TASK_T Scan_Task_Array[] =
//{
//    //LogSendBuffer,
//    {timer_task},
//    {dev_adc_task},
//    {error_check_task},

//#ifdef TEST_PERFORMANCE
//    {test_performance},
//#endif
//    {comm_task},
//    //Test_Proc
//};

extern power_on_reason_t power_on_reason_s;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* Divide CheckPowerOnReason into Check1 & Check2 for
locking serial communication before ALL_Dev_Init if in HW reset */
void CheckPowerOnReason1(void)
{
    hwi_power_on_reason();
    boot_record_t *brt = get_boot_record_from_flash();
    if (//1 == power_on_reason_s.COLD_BOOT ||
        1 == power_on_reason_s.OBL_RST
        || 1 == power_on_reason_s.IWDG_RST
        || 1 == power_on_reason_s.SFT_RST)
    {
        return;
    }

    if (1 == power_on_reason_s.COLD_BOOT){
#ifdef SSCOM
        //serial communication locked when cold boot
        if (brt->comm_lock_flag != 0)
        {
            brt->comm_lock_flag = 0;
            update_data_flash(BOOT_RECORD, INVALID);
        }
#endif
        return ;
    }
    
    if (hwi_ReadBackupRegister(BKP_DR2) != 0x5678)
    {
        power_on_reason_s.HW_RST = 1;
#ifdef SSCOM
        //serial communication locked when HW reset
        if (brt->comm_lock_flag != 0)
        {
            brt->comm_lock_flag = 0;
            update_data_flash(BOOT_RECORD, INVALID);
        }
#endif
        return ;
    }
}


/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
void mcu_sw25_init(void)
{
	hwi_GPIO_WritePin(EN_2V5_SW_E, HWI_PIN_RESET);
}

void ALL_Dev_Init(void)
{
    /*Check btn status as quickly as possible to avoid missing short press*/
    app_button_init();
    if(app_read_button() == btn_down){
        btn_flag = btn_down;
    }

    dev_pwm_output_init_usr();
    mcu_sw25_init();
    //haptic_gpio_init();
    hwi_delay_ms(260);//wait to initialize the soc then can read soc value
    hwi_I2C_Init(1);
    UartInit();
    swd_reinit();
    UartGpioInit();
  
    app_charge_enable(0);
    RtcEnableDomainRegisterInit();
    heating_off_pin_init();

    hwi_GPIO_WritePin(EN_2V8_OLED_VDD_E,HWI_PIN_RESET);
    hwi_GPIO_WritePin(OLED_VDD_EN_E,HWI_PIN_SET);

    //haptic_driver_init();
    dev_adc_init();  /*must place it before app_charge_init for that need check adc in app_charge_init*/
    hwi_dma_config_init();
    hwi_adc_reinit();
    app_bat_id_init();
    app_charge_init();
    app_pd_init();
    /*button_init timer_init haptic_driver_init app_charge_init must before than CheckPowerOnReason*/

    spi_flash_init();// exflash init
    //This project does not require LED lights
    //LogSuspend();//disable debug log
    //set_cycle_log_flag(0);//disable cycle log
    dev_dma_config();
    dev_oled_init();
    app_hall_door_init();
}


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
#ifdef DEBUG_OFF // vector table
    SCB->VTOR = FLASH_BASE | 0x11000;
#endif

    hwi_HAL_Init();
    hwi_SystemClock_Config();
    
    /* configure 4 bits pre-emption priority */
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

    uart_lock_create();   //never LOG out before uart_lock_create();
    i2c_lock_create();
    LOGI("... Hello Stratos ...\n\r");

    RtcEnableDomainRegisterInit();
    CheckPowerOnReason1();
    

    boot_record_t *brt = get_boot_record_from_flash();
#ifdef SSCOM
    if(brt->comm_lock_flag == COMM_UNLOCK_FLAG){
        set_comm_lock(0);
        log_mask = LOG_ALL;
    }
#else
    set_comm_lock(0);
    log_mask = LOG_ALL;
#endif
    /*also sync data from self_flash to ram*/
    get_self_flash_record();
    /****all initialization for all device****/
    ALL_Dev_Init();  

    LOGI("... %s ...\n\r", DEVICE_APP_VER);
    char boot_ver[33] = {0};
    strncat(boot_ver, (char *)brt->bootloader_version, 32);
    if(!strncmp(boot_ver, "Device Bootloader", strlen("Device Bootloader")))
    {
        LOGI("... %s ...\n\r", boot_ver);
    }
    else if(!strncmp(boot_ver, BL_VERSION, strlen(BL_VERSION)))
    {
        LOGI("... %s ...\n\r", boot_ver);
    }
    else
    {
        LOGI("... Device Bootloader unkown ...\n\r");
    }
    (void)get_data_change_frequent_from_flash();
    
    ext_flash_record_t *extFlash_r = get_version_record_from_ext_flash();
    char UI_ver[33] = {0};
    strncat(UI_ver, (char *)extFlash_r->ui_version, 32);
    LOGI("... %s ...\n\r", UI_ver);
    
    /* USER CODE BEGIN WHILE */

   /*
       Write 0x1234 when start running: Device is running if reset
       Write 0x5678 when get into shutdown: Device is shutdown if reset
    */
    WriteBackupRegister(BKP_DR2, 0x1234);
    /*error check must be later then boot complete*/
    set_error_check_status(disable_s);
    
    LOGD("ExecuteTask running ...\n\r");  
    
    /* init task */
    xTaskCreate(init_task, "INIT", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    /*** start scheduler ***/
    vTaskStartScheduler();
    
    while (1)
    {
    }
    /* USER CODE END 3 */
}


void init_task(void * pvParameters)
{
    task_init(); 
    for( ;; ){
        vTaskDelete(NULL);
    }
}


/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
//void Error_Handler(void)
//{
//  /* USER CODE BEGIN Error_Handler_Debug */
//  /* User can add his own implementation to report the HAL error return state */

//  /* USER CODE END Error_Handler_Debug */
//}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
