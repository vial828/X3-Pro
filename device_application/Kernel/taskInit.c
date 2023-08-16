/* Includes ------------------------------------------------------------------*/
#include "log.h"
#include "manager.h"
#include "app_charge.h"
#include "dev_adc.h"
#include "app_heat.h"
#include "app_button.h"
#include "error_code.h"
#include "app_haptic.h"
#include "batTimer.h"
#include "app_pd_com.h"
#include "wdg.h"
#include "dev_cap_aw9320x_application.h"
#include "app_oled_UI.h"
#include "app_oled_display.h"
#include "dev_oled_driver.h"
#include "kernel.h"

extern uint8_t btn_flag;
uint8_t hall_flag = door_close;
TaskHandle_t taskcheck;
//static timer_bat_t *pbooting_timer;
static ptimer_t pbooting_timer;

static void call_back_booting(const ptimer_t tm)
{
    if(pbooting_timer){
        if(bat_timer_delete(pbooting_timer, portMAX_DELAY)==pdPASS){
            pbooting_timer = NULL;
        }
    }
    post_msg_to_manager(opt_boot_complete);
//    LOGD("booting finish\r\n");
}

static void button_booting(void)
{
    if(pbooting_timer){
        if(bat_timer_delete(pbooting_timer, portMAX_DELAY)==pdPASS){
            pbooting_timer = NULL;
        }
    }
    post_msg_to_manager(opt_boot_complete);
    LOGD("btn booting finish");

    if(app_read_button() == btn_up){
        post_msg_to_manager(op_btn_short_up);
    }
}
void hardware_reboot_haptic(void)
{
    app_haptic_buzz('a');
}

uint8_t check_error(void)
{
    boot_record_t *brt = get_boot_record_from_ram();
    if(1 == power_on_reason_s.HW_RST || 1 == power_on_reason_s.COLD_BOOT)
    {
        if(ERR_EXIST == brt->reset_err){
            brt->reset_err = ERR_CLEAR;
            brt->error_pos = 0xFFFFFFFF;
            update_data_flash(BOOT_RECORD, INVALID);
        }
     }

    if(ERR_EXIST == brt->return_err){
        if(brt->error_pos == flt_de_end_of_life)
        {
            start_error_ui(ERROR_EOL);
        }else
        {
            start_error_ui(ERROR_RETURN);
        }
        post_msg_to_manager_with_arg(op_existing_error, RETURN_ERR);
        return 1;
    }
    else if(ERR_EXIST == brt->reset_err){
        start_oled_error_reset_oneshot();
        post_msg_to_manager_with_arg(op_existing_error, RESET_ERR);
        return 2;
    }
    else{
        brt->error_pos = 0xFFFFFFFF;
        return 0;
    }
}

//void show_battery_and_door_mode(void)
//{
//   uint8_t err = check_error();
//    if(err == 1)
//    {
//        LOGD("btn return ERR");
//    }
//    else if(err == 2)   
//    {
//        LOGD("btn reset ERR");
//    }
//    else if(err == 0)
//    {
//        LOGD("btn no ERR");
//        start_oled_dispaly_battery_check(app_get_bat_left());;
//    }
//    else
//    {
//    }
//}

void  show_reboot()
{
    uint8_t err = check_error();
    if(err == 1)
    {
        LOGD("brand return ERR");
    }
    else if(err == 2)
    {
        LOGD("brand reset ERR");
    }
    else if(err == 0)
    {
        LOGD("brand no ERR");
        start_oled_reboot_ui();
    }
    else
    {
        //nothing to do
    }
}
void  show_brand_or_low_batt()
{
    uint8_t err = check_error();
    if(err == 1)
    {
        LOGD("brand return ERR");
    }
    else if(err == 2)
    {
        LOGD("brand reset ERR");
    }
    else if(err == 0)
    {
        LOGD("brand no ERR");
        if(app_get_bat_left()<6)
        {
            start_oled_display_battery_check(app_get_bat_left());
            app_haptic_buzz('a');
            LOGD("hapticA for low soc");
        }
        else if(hall_flag == door_base || hall_flag == door_boost)
        {
            start_oled_welcome_brand();
        }
        else if(hall_flag == door_close)
        {
            start_no_mode_closing_animation();
        }
    }
    else
    {
        //nothing to do
    }
}
#if 0
/*******************************************************************************
 * Macros
 ********************************************************************************/
//#define DISABLE_WDG
#define MEM_SLOT_NUM            (0u)      /* Slot number of the memory to use */
#define QSPI_BUS_FREQUENCY_HZ   (50000000lu)

/*******************************************************************************
 * Function Name: check_status
 ****************************************************************************//**
 * Summary:
 *  Prints the message, indicates the non-zero status by turning the LED on, and
 *  asserts the non-zero status.
 *
 * Parameters:
 *  message - message to print if status is non-zero.
 *  status - status for evaluation.
 *
 *******************************************************************************/
void check_status(char *message, uint32_t status) {
	if (0u != status) {
		LOGI(
				"\n================================================================================\n");
		LOGI("\nFAIL: %s\n", message);
		LOGI("Error Code: 0x%08lX\n", (unsigned long )status);
		LOGI(
				"\n================================================================================\n");

		while (1)
			; /* Wait forever here when error occurs. */
	}
}

void serial_flash_qspi_init() {
	cy_rslt_t result;
	LOGI(
			"*************** PSoC 6 MCU: External Flash Access in XIP Mode ***************\n\n");
	/* Initialize the QSPI block */
	result = cy_serial_flash_qspi_init(smifMemConfigs[MEM_SLOT_NUM],
			CYBSP_QSPI_D0, CYBSP_QSPI_D1, CYBSP_QSPI_D2, CYBSP_QSPI_D3, NC, NC,
			NC, NC, CYBSP_QSPI_SCK, CYBSP_QSPI_SS, QSPI_BUS_FREQUENCY_HZ);
	check_status("Serial Flash initialization failed", result);
	/* Put the device in XIP mode */
	LOGI("\n5. Entering XIP Mode.\n");
	cy_serial_flash_qspi_enable_xip(true);

}
#endif
void CheckPowerOnReason2(void)
{
    dev_first_get_soc();
    data_change_frequent_t* pDataChangeFreq = get_data_change_frequent_from_ram();
    hall_flag = app_get_hall_door_status();
    pbooting_timer = bat_timer_reset_ext(pbooting_timer, "pbooting_timer", 2200, TIMER_OPT_ONESHOT, call_back_booting);
    bat_timer_start(pbooting_timer, portMAX_DELAY);
    if(1 == power_on_reason_s.COLD_BOOT){
        show_reboot();
        hardware_reboot_haptic();
        record_g_b_adc();
        app_chg_entry_suspend(CHG_HW_RESET_SUSPEND);
        pDataChangeFreq->lifeCycleData[power_on_reason] = POWERON_COLD_BOOT;
        LOGD("mcu cold boot");
    }
    else if(1 == power_on_reason_s.OBL_RST){
        show_reboot();
        hardware_reboot_haptic();
        pDataChangeFreq->lifeCycleData[power_on_reason] = POWERON_OBLRST;
        LOGD("OBLRST reset");
    }
    else if(1 == power_on_reason_s.IWDG_RST){
        show_reboot();
        hardware_reboot_haptic();
        pDataChangeFreq->lifeCycleData[power_on_reason] = POWERON_IWDGRST;
        LOGD("IWDGRST reset");
    }
    else if(1 == power_on_reason_s.SFT_RST){
        show_reboot();
        record_g_b_adc();
        hardware_reboot_haptic();
        pDataChangeFreq->lifeCycleData[power_on_reason] = POWERON_SFTRST;
        LOGD("SFTRST reset");
    }
    else if(1 == power_on_reason_s.HW_RST){
        show_reboot();
        record_g_b_adc();
        hardware_reboot_haptic();
        app_chg_entry_suspend(CHG_HW_RESET_SUSPEND);
        LOGD("HW reset");
//#ifdef SSCOM
//        TIMER_SAFE_RESET(pbooting_timer , 2*1000 , TIMER_OPT_ONESHOT , call_back_booting , NULL);
//        boot_record_t *brt = get_boot_record_from_ram();
//        if(ERR_EXIST == brt->reset_err){
//            brt->reset_err = ERR_CLEAR;
//            brt->error_pos = 0xFFFFFFFF;
//            update_data_flash(BOOT_RECORD, INVALID);
//        }
//#else 
          //change because factory will plug the battery when HW RESET

//#endif
    }
    else{
        if(btn_flag == btn_down)
        {
            pmu_flag_clear(PMU_FLAG_WAKEUP);
            record_g_b_adc();
            uint8_t err = check_error();
            if(err == 1)
            {
                LOGD("btn wakeup: return ERR");
            }
            else if(err == 2)   
            {
                LOGD("btn wakeup: reset ERR");
            }
            else if(err == 0)
            {
                LOGD("btn wakeup: no ERR");
                //pbooting_timer = bat_timer_reset_ext(pbooting_timer, "pbooting_timer", 500, TIMER_OPT_ONESHOT, call_back_button_booting);
                //bat_timer_start(pbooting_timer, portMAX_DELAY);
                button_booting();
            }
            pDataChangeFreq->lifeCycleData[power_on_reason] = POWERON_BUTTON_WAKEUP;


        }
        else if(app_GetVbusVolt() > 4000)
        {
            pmu_flag_clear(PMU_FLAG_WAKEUP);
            cal_i_sense_b_RE();
            uint8_t err = check_error();
            if(err == 1)
            {
                LOGD("usb wakeup: return ERR");
            }
            else if(err == 2)   
            {
                LOGD("usb wakeup: reset ERR");
            }
            else if(err == 0)
            {
                LOGD("usb wakeup: no ERR");
                pbooting_timer = bat_timer_reset_ext(pbooting_timer, "pbooting_timer", 500, TIMER_OPT_ONESHOT, call_back_booting);
                bat_timer_start(pbooting_timer, portMAX_DELAY);
            }
            pDataChangeFreq->lifeCycleData[power_on_reason] = POWERON_USB_WAKEUP;

        }
        else if(hall_flag == door_base || hall_flag == door_boost || (hall_flag == door_close&&pmu_flag_get(PMU_FLAG_WAKEUP) == 1))
        {
            pmu_flag_clear(PMU_FLAG_WAKEUP);
            record_g_b_adc();
            show_brand_or_low_batt();
            //show_brand_and_door_mode();
            pDataChangeFreq->lifeCycleData[power_on_reason] = POWERON_HALL_WAKEUP;
            LOGD("hall wakeup");
        }
        else{
                uint8_t err = check_error();
                if (1 == power_on_reason_s.PIN_RST)
                {
                    if(err == 1)
                    {
                        LOGD("PINRST: return ERR");
                    }
                    else if(err == 2)   
                    {
                        LOGD("PINRST: reset ERR");
                    }
                    else if(err == 0)
                    {
                        LOGD("PINRST: no ERR");
                    }
                }
                else
                {
                    if(err == 1)
                    {
                        LOGD("unknown RST: return ERR");
                    }
                    else if(err == 2)   
                    {
                        LOGD("unknown RST: reset ERR");
                    }
                    else if(err == 0)
                    {
                        LOGD("unknown RST: no ERR");
                    }
                    
                }

        }
    }
}


/* USER CODE BEGIN PFP */
void TaskManager(void *arg) {
    (void) arg;
    /* Initialize hardware */
    for (;;) {
        /* Do something */
        //MyActions();
        manager_task();
        /* Allow other tasks to execute */
        vTaskDelay(5);
    }
}

void Task_check(void *arg) {
    (void) arg;
    /* Initialize hardware */
    uint16_t index = 0;
    for (;;) {

        /* Do something */
        //MyActions();
        if(index % 2 == 0){// task_16ms
            record_life_cycle_task();
            error_check_task();
            check_heat_task();
        }

        app_button_task();
        app_hall_door_task();
        //hall_checking_process();
#if 0
        if(index % 5 == 0){
            dev_aw9320x_data_log();
        }
#endif
        if(index % 125 == 0){// task_1000ms
            check_charge_task();
            read_soc_from_gauge();
        }
        if(index % 1250 == 0){// task_10000ms
            index = 0;
            app_CheckChgICRegData();
            app_CheckGaugeData();
        }
        index++;

        /* Allow other tasks to execute */
        vTaskDelay(8);
    }
}

void TaskCharge(void *arg) {
    (void) arg;
    /* Initialize hardware */

    uint8_t index = 0;
    for (;;) {

        /* Do something */

        //process charge fsm
        if(index % 50 == 0){//task_200ms
            //index = 0;
            app_charge_ic_task();
            app_charge_task();        
        }
        if(index % 250 == 0){// task_1000ms
            index = 0;
#ifndef DISABLE_WDG
            FeedIwdg();
#endif
        }
        index++;

        app_pd_process();
        /* Allow other tasks to execute */
        vTaskDelay(4);
    }
}

void TaskDisplay(void *arg) {
    (void) arg;
    /* Initialize hardware */

//    oled_dis_demo();
    for (;;) {
        /* Do something */
        //MyActions();
        oled_display_task();
        /* Allow other tasks to execute */
        vTaskDelay(5);
    }
}

/* USER CODE BEGIN PFP */
void TaskLogCmd(void *arg) {
    (void) arg;
    /* Initialize hardware */

    for (;;) {
        /* Do something */
        //MyActions();
        comm_task();
        /* Allow other tasks to execute */
        vTaskDelay(2);
    }
}

//void TaskSendLog(void *arg) {
//	(void) arg;

//    comm_init();
//	for (;;) {
//		//send commands through uart
//		comm_send_proc();
// //       timer_task();
//		/* Allow other tasks to execute */
//		vTaskDelay(1);
//	}
//}

void TaskADCData(void *arg) {
    (void) arg;
    /* Initialize hardware */

    for (;;) {
        /* Do something */
        //MyActions();
        dev_adc_task();
        dev_cal_adc_result();

        /* Allow other tasks to execute */
        vTaskDelay(4);
    }
}


/*************************************************************************************************
 * @brief    :init all the task
 * @param    :void
 * @return   :void
 *************************************************************************************************/
void task_init(void) 
{
#ifdef ENABLE_STICK_SENSOR
    dev_aw93205_init();
#endif
    /*add initialization for task*/
    app_heat_init();
    dev_cal_adc_result();
    manager_init();     //must be the last init
    oled_display_init();    

    //CheckPowerOnReason2 must after the create_mngr_msg_queue() in manger_init();
    check_cic_config_error();
    CheckPowerOnReason2();
    app_chg_entry_suspend(CHG_HW_RESET_SUSPEND);

//    if (pdPASS != xTaskCreate(TaskSendLog, // Task function
//            "task_log", // Task name
//            256, // Task stack size, 256*4 byte
//            NULL, // Parameters passed to task
//            5, // Task priority
//            NULL) // Task handle
//            ) {
//        configASSERT(0);
//    }

    if (pdPASS != xTaskCreate(TaskLogCmd, // Task function
            "T_LogCmd", // Task name
            2048, // Task stack size
            NULL, // Parameters passed to task
            2, // Task priority
            NULL) // Task handle
            ) {
        configASSERT(0);
    }

    if (pdPASS != xTaskCreate(TaskManager, // Task function
            "T_manager", // Task name
            512, // Task stack size
            NULL, // Parameters passed to task
            4, // Task priority
            NULL) // Task handle
            ) {
        configASSERT(0);
    }

    if (pdPASS != xTaskCreate(TaskADCData, // Task function
            "T_ADCdata", // Task name
            384, // Task stack size
            NULL, // Parameters passed to task
            6, // Task priority
            NULL) // Task handle
            ) {
        configASSERT(0);
    }

    if (pdPASS != xTaskCreate(TaskCharge, // Task function
            "T_charge", // Task name
            512, // Task stack size
            NULL, // Parameters passed to task
            3, // Task priority
            NULL) // Task handle
            ) {
        configASSERT(0);
    }

    if (pdPASS != xTaskCreate(Task_check, // Task function
            "T_check", // Task name
            512, // Task stack size
            NULL, // Parameters passed to task
            5, // Task priority
             &taskcheck) // Task handle
            ) {
        configASSERT(0);
    }

    if (pdPASS != xTaskCreate(TaskDisplay, // Task function
            "T_display", // Task name
            2560, // Task stack size
            NULL, // Parameters passed to task
            1, // Task priority
            NULL) // Task handle
            ) {
        configASSERT(0);
    }

#ifdef ENABLE_MONITOR_TASK_LOG
    if (pdPASS != xTaskCreate(monitor_func, // Task function
            "T_monitor", // Task name
            512, // Task stack size
            NULL, // Parameters passed to task
            3, // Task priority
            NULL) // Task handle
            ) {
        configASSERT(0);
    }
#endif
}
void SuspendCheckTask(void){
    vTaskSuspend(taskcheck);
}

