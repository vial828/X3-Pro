#include "HWI_flash.h"
#include "HWI_Hal.h"
#include "kernel.h"
#include "log.h"
#include "string.h"
#include "self_flash.h"
#include "dev_adc.h"
#include "dev_temperature.h"
#include "power.h"
#include "app_charge.h"
#include "app_heat.h"
#include "app_haptic.h"
#include "stratos_defs.h"
#include "dev_bq25898x.h"

#define FLASH_END_ADDRESS              (FLASH_BASE + FLASH_SIZE)


#define FLASH_USER_START_ADDR   (FLASH_END_ADDRESS - ((USER_DATA_FLASH_PAGES_NUM + FREQUENT_DATA_FLASH_PAGES_NUM) * FLASH_PAGE_SIZE))   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     (FLASH_USER_START_ADDR +  FLASH_PAGE_SIZE - 1)   /* End @ of user Flash area */
#define FLASH_DATA_CHANGE_FREQUENT_START_ADDR   (FLASH_USER_END_ADDR + 1)  /* Start @ of frequent data area */
#define FLASH_DATA_CHANGE_FREQUENT_END_ADDR     (FLASH_DATA_CHANGE_FREQUENT_START_ADDR + FLASH_PAGE_SIZE - 1) /* End @ of frequent data area */

#ifdef DEBUG_OFF
  #define FLASH_BOOT_RECORD_START_ADDR   (FLASH_BASE + (16 * FLASH_PAGE_SIZE))  /* Start @ of bootloader record Flash area */
  #define FLASH_BOOT_RECORD_END_ADDR     (FLASH_BASE + (17 * FLASH_PAGE_SIZE) - 1)   /* End @ of bootloader record Flash area */
#else
  #define FLASH_BOOT_RECORD_START_ADDR   (FLASH_USER_START_ADDR - FLASH_PAGE_SIZE)  /* Start @ of bootloader record Flash area */
  #define FLASH_BOOT_RECORD_END_ADDR     (FLASH_USER_START_ADDR - 1)   /* End @ of bootloader record Flash area */
#endif

/*Variable used for Erase procedure*/
static HWI_FLASH_EraseInit EraseInitStruct = {0};
static boot_record_t boot_record_s;
static flash_record_t flash_r_data;
static data_change_frequent_t data_change_frequent={0};
data_change_frequent_t data_change_frequent_temp = {0};

session_t g_session[100];

/*************************************************************************************************
  * @brief    : Return the page where the current flash address is located
  * @param1   : Flash address
  * @return   : Current page number ( The size of each page is 2*1024 bytes )
*************************************************************************************************/
static uint32_t GetPage(uint32_t Addr)
{
  return (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;;
}

/*************************************************************************************************
  * @brief    : Calculate the checksum
  * @param1   : Data
  * @param2   : Data number
  * @return   : Checksum ( Often used in communication )
*************************************************************************************************/
uint64_t CrcCheck(uint8_t* data, uint32_t num)
{
	uint64_t crc = 0;

	while(num --)
	{
		crc += *data;
		data++;
	}

	return crc;
}

/*************************************************************************************************
  * @brief    : Get bootloader record data from flash
  * @return   : Pointer to bootloader record
*************************************************************************************************/
boot_record_t * get_boot_record_from_flash(void)
{
    /* Read record data from bootloader flash and copy to boot_record_s */
    memcpy((void*)&boot_record_s, (void *)FLASH_BOOT_RECORD_START_ADDR, sizeof(boot_record_t));
    return &boot_record_s;
}

/*************************************************************************************************
  * @brief    : Get bootloader record data from ram
  * @return   : Pointer to bootloader record
*************************************************************************************************/
boot_record_t * get_boot_record_from_ram(void)
{
    return &boot_record_s;
}
/*************************************************************************************************
  * @brief    : Get ini version and check 
  * @return   : ini version state
*************************************************************************************************/
uint8_t ini_version_check(void)
{
    char * tmp_vr1 = strstr((char *)flash_r_data.ini_version, "test");
    if(tmp_vr1 != NULL){
        return NO_UPDATE_INI_VER;
    }

    char * tmp_vr2 = strstr((char *)flash_r_data.ini_version, "TEST");
    if(tmp_vr2 != NULL){
        return NO_UPDATE_INI_VER;
    }

    char * tmp = strstr((char *)flash_r_data.ini_version, "_20");
    if(tmp == NULL){
        return ERR_INI_VER;
    }
    char * flash_ini_version = tmp + 1;
    char * ram_ini_version = strstr(INI_VERSION, "_20") + 1;
    if(strcmp(ram_ini_version, flash_ini_version) > 0){
        return UPDATE_INI_VER;
    }
    else{
        return NO_UPDATE_INI_VER;
    }
}

/*************************************************************************************************
  * @brief    : Get record value from self flash
  * @param1   : None
  * @return   : Pointer of record flash
*************************************************************************************************/
flash_record_t* get_self_flash_record(void)
{
    uint64_t crc = 0;

    __disable_irq();
    icache_disable();
    /* Read record data from self flash and copy to flash_r_data */
    memcpy((void*)&flash_r_data, (void *)FLASH_USER_START_ADDR, sizeof(flash_record_t));
    icache_enable();
    __enable_irq();

    crc = CrcCheck((uint8_t *)&flash_r_data, (sizeof(flash_record_t) - 8));//calculate crc value except flag_crc(uint64_t)
    LOGD("flash_r_data crc=%lld,sum crc=%lld",flash_r_data.flag_crc, crc);
    /* If the verification fails, set all parameters to default values */
    if (crc != flash_r_data.flag_crc || crc == 0||ini_version_check() == UPDATE_INI_VER)
    {
        if(ini_version_check() == UPDATE_INI_VER)
        {
            LOGD("update flash data");
        }
        else
        {
            LOGD("crc error!");
        }
        memcpy(flash_r_data.n_heat_name,"default", 8);
        app_set_default_heat_profile(&(flash_r_data.zone1_n),'n',1);
        app_set_default_heat_profile(&(flash_r_data.zone2_n),'n',2);
        memcpy(flash_r_data.b_heat_name,"default", 8);
        app_set_default_heat_profile(&(flash_r_data.zone1_b),'b',1);
        app_set_default_heat_profile(&(flash_r_data.zone2_b),'b',2);

        memset(flash_r_data.ini_version,0, 32);
        memcpy(flash_r_data.ini_version,INI_VERSION, 17);

        flash_r_data.pwm_dac_duty = 30;

        flash_r_data.pwm_htr_duty_coil1=PWM_DUTY_COIL1;
        flash_r_data.pwm_htr_duty_coil2=PWM_DUTY_COIL2;

        flash_r_data.pwm_htr_clock_coil1 = PWM_FREQ_COIL1;
        flash_r_data.pwm_htr_clock_coil2 = PWM_FREQ_COIL2;

        flash_r_data.haptic_pwm_freq = PWM_FREQ_HAPTIC;

        flash_r_data.haptic_a.cycle_cnt = 1;
        flash_r_data.haptic_a.one_cycle_buzz_s[0].on_time = 250;

        flash_r_data.haptic_b.cycle_cnt = 1;
        flash_r_data.haptic_b.one_cycle_buzz_s[0].on_time = 750;

        flash_r_data.haptic_c.cycle_cnt = 1;
        flash_r_data.haptic_c.one_cycle_buzz_s[0].on_time = 750;

        flash_r_data.haptic_d.cycle_cnt = 3;
        flash_r_data.haptic_d.one_cycle_buzz_s[0].on_time = 750;
        flash_r_data.haptic_d.one_cycle_buzz_s[0].off_time = 750;
        flash_r_data.haptic_d.one_cycle_buzz_s[1].on_time = 750;
        flash_r_data.haptic_d.one_cycle_buzz_s[1].off_time = 750;
        flash_r_data.haptic_d.one_cycle_buzz_s[2].on_time = 750;
        flash_r_data.haptic_d.one_cycle_buzz_s[2].off_time = 750;

        flash_r_data.haptic_volt = HAPTIC_VOLT_DEFAULT;
        //flash_r_data.b2b_batt_temp_limit = 48;
        flash_r_data.stick_sensor_status = STICK_SENSOR_OFF;
        flash_r_data.charge_temp_protect = BAT_HOT_PROTECT_THRESHOLD;
        flash_r_data.charge_temp_protect_relesae = BAT_HOT_PROTECT_RELEASE;
        flash_r_data.extend_mode = EXTEND_UI_OFF;
        flash_r_data.eol_session = EOL_DEFAULT;
        flash_r_data.step1_session_nums = STEP1_NUMS_DEFAULT;
        flash_r_data.step2_session_nums = STEP2_NUMS_DEFAULT;
        flash_r_data.step3_session_nums = STEP3_NUMS_DEFAULT;
        flash_r_data.step4_session_nums = STEP4_NUMS_DEFAULT;
        flash_r_data.slow_chg_isense = SLOW_CHG_DEFAULT_MA;
        flash_r_data.slow_batv_h= SLOW_DEFAULT_H_BATV;
        flash_r_data.slow_batv_l = SLOW_DEFAULT_L_BATV;
        flash_r_data.wrong_chg_h_mv = WRONG_CHG_H_MV;
        flash_r_data.step1_chg_curr = FAST_CHARGE_CURR_STEP1_LIMIT;
        flash_r_data.step1_chg_volt= CHARGE_VOLT_LIMIT_STEP1;
        flash_r_data.step2_chg_curr = FAST_CHARGE_CURR_STEP2_LIMIT;
        flash_r_data.step2_chg_volt = CHARGE_VOLT_LIMIT_STEP2;
        flash_r_data.step3_chg_curr = FAST_CHARGE_CURR_STEP3_LIMIT;
        flash_r_data.step3_chg_volt= CHARGE_VOLT_LIMIT_STEP3;
        flash_r_data.step4_chg_curr = FAST_CHARGE_CURR_STEP4_LIMIT;
        flash_r_data.step4_chg_volt = CHARGE_VOLT_LIMIT_STEP4;
        flash_r_data.step2_chg_hot_limit = BAT_HOT_CHARGE_STEP2;
        flash_r_data.step2_chg_hot_protect = BAT_HOT_PROTECT_THRESHOLD_STEP2;

        flash_r_data.error_parameter.pre_session_temp_limit = PRE_SESSION_TEMP_LIMIT;
        flash_r_data.error_parameter.charge_temp_limit = BAT_HOT_CHARGING;
        flash_r_data.error_parameter.charge_temp_limit_clear = BAT_HOT_CHARGING_CLEAR;
        flash_r_data.error_parameter.bat_cold_charge_temp = BAT_COLD_CHARGE_TEMP;
        flash_r_data.error_parameter.bat_cold_charge_temp_clear = BAT_COLD_CHARGE_TEMP_CLEAR;
        flash_r_data.error_parameter.bat_hot_temp = BAT_HOT_TEMP;
        flash_r_data.error_parameter.bat_hot_temp_clear = BAT_HOT_TEMP_CLEAR;
        flash_r_data.error_parameter.bat_cold_heat_temp = BAT_COLD_HEAT_TEMP;
        flash_r_data.error_parameter.bat_cold_heat_temp_clear = BAT_COLD_HEAT_TEMP_CLEAR;
        flash_r_data.error_parameter.heat_empty_volt = DURING_SESSION_EMPTY_VOLT;
        flash_r_data.error_parameter.heat_cutoff_volt = PRE_HEAT_CUTOFF_VOLT;
        flash_r_data.error_parameter.heat_cutoff_volt_soc = PRE_HEAT_CUTOFF_VOLT_SOC;
        flash_r_data.error_parameter.bat_volt_damage_protect = BAT_VOLT_DAMAGE_PROTECT;
        flash_r_data.error_parameter.tc_zone1_hot = TC_ZONE1_HOT;
        flash_r_data.error_parameter.tc_zone1_hot_clear = TC_ZONE1_HOT_CLEAR;
        flash_r_data.error_parameter.tc_zone2_hot = TC_ZONE2_HOT;
        flash_r_data.error_parameter.tc_zone2_hot_clear = TC_ZONE2_HOT_CLEAR;
//        flash_r_data.error_parameter.tc_zone1_cold = TC_ZONE1_COLD;
//        flash_r_data.error_parameter.tc_zone1_cold_clear = TC_ZONE1_COLD_CLEAR;
//        flash_r_data.error_parameter.tc_zone2_cold = TC_ZONE2_COLD;
//        flash_r_data.error_parameter.tc_zone2_cold_clear = TC_ZONE2_COLD_CLEAR;
        flash_r_data.error_parameter.bat_volt_over = BAT_VOLTAGE_OVER;
        flash_r_data.error_parameter.bat_volt_over_clear = BAT_VOLTAGE_OVER_CLEAR;
        flash_r_data.error_parameter.discharge_current_over = BAT_DISCHARGE_CURR_OVER;
        flash_r_data.error_parameter.charge_current_over = BAT_CHARGE_CURR_OVER;
        flash_r_data.error_parameter.co_junc_hot = CO_JUNC_HOT;
        flash_r_data.error_parameter.co_junc_hot_clear = CO_JUNC_HOT_CLEAR;
//        flash_r_data.error_parameter.co_junc_cold = CO_JUNC_COLD;
//        flash_r_data.error_parameter.co_junc_cold_clear = CO_JUNC_COLD_CLEAR;
        flash_r_data.error_parameter.i_sense_damage = BAT_I_SENSE_DAMAGE;
        flash_r_data.error_parameter.charge_timeout = CHARGE_TIMEOUT;
        flash_r_data.error_parameter.coil_hot_temp = COIL_HOT_TEMP;
        flash_r_data.error_parameter.coil_hot_temp_clear = COIL_HOT_TEMP_CLEAR;
        flash_r_data.error_parameter.usb_hot_temp = USB_HOT_TEMP;
        flash_r_data.error_parameter.usb_hot_temp_clear = USB_HOT_TEMP_CLEAR;
//        flash_r_data.error_parameter.usb_cold_temp = USB_COLD_TEMP;
//        flash_r_data.error_parameter.usb_cold_temp_clear = USB_COLD_TEMP_CLEAR;

        update_data_flash(USR_DATA, INVALID);
    }
    return &flash_r_data;
}

/*************************************************************************************************
  * @brief    : Get change frequent data ( session data ) from self flash
  * @param1   : None
  * @return   : Pointer of data_change_frequent_t
*************************************************************************************************/
data_change_frequent_t* get_data_change_frequent_from_flash(void)
{
    int32_t sessionWritePos = 0;
    int32_t sessionNum = 0;
    uint8_t i = 0;
    uint64_t crc = 0;
    data_change_frequent.g_sessionWritePos = 0;
    data_change_frequent.g_sessionNum = 0;
    data_change_frequent.serialNum = 0;
    /* Read out the data in 4 flashes in turn, and then select the latest */
    while (i<FREQUENT_DATA_FLASH_PAGES_NUM)
    {
        /* Take out the data and make a crc judgment */
        __disable_irq();
        icache_disable();
        memcpy((void*)&data_change_frequent_temp, (void *)(FLASH_DATA_CHANGE_FREQUENT_START_ADDR+i*FLASH_PAGE_SIZE), sizeof(data_change_frequent_t));
        icache_enable();
        __enable_irq();

        crc = CrcCheck((uint8_t *)&data_change_frequent_temp, (sizeof(data_change_frequent_t)  - 8));
        LOGD("crc check!frequent from flash.crc=%lld,sum crc=%lld",data_change_frequent_temp.crc, crc);
        if ((data_change_frequent_temp.crc != crc) || (0xFFFFFFFF == data_change_frequent_temp.g_sessionNum))
        {
            LOGD("crc error!");
            memset((void*)&data_change_frequent_temp, 0, sizeof(data_change_frequent_t));
            erase_frquent_data_flash(i);
        }
        /* Judge whether the session data is up to date through g_sessionWritePos */
        if (data_change_frequent_temp.g_sessionWritePos > sessionWritePos)
        {
            sessionWritePos = data_change_frequent_temp.g_sessionWritePos;
            sessionNum = data_change_frequent_temp.g_sessionNum;
        }
        if (data_change_frequent_temp.serialNum > data_change_frequent.serialNum)
        {
            memcpy((void*)&data_change_frequent, (void *)&data_change_frequent_temp, sizeof(data_change_frequent_t));
        }
        memcpy((void*)&g_session[i*25], (void *)&data_change_frequent_temp.session, sizeof(session_t)*25);
        i++;
    }

    data_change_frequent.g_sessionNum = sessionNum;
    data_change_frequent.g_sessionWritePos = sessionWritePos;
    LOGD("g_sessionNum=%d,g_sessionWritePos=%d!", data_change_frequent.g_sessionNum, data_change_frequent.g_sessionWritePos);
    return &data_change_frequent;
}

/*************************************************************************************************
  * @brief    : Get record value from MCU RAM
  * @param1   : None
  * @return   : Pointer of record flash
*************************************************************************************************/
flash_record_t* get_self_flash_record_from_ram(void)
{
	return &flash_r_data;
}

/*************************************************************************************************
  * @brief    : Get session data and statistics data from MCU RAM
  * @param1   : None
  * @return   : Pointer of record flash
*************************************************************************************************/
data_change_frequent_t* get_data_change_frequent_from_ram(void)
{
	return &data_change_frequent;
}

/*************************************************************************************************
  * @brief    : Get 100 sessions data value from MCU RAM
  * @param1   : None
  * @return   : Pointer of record flash
*************************************************************************************************/
session_t* get_100sessions_from_ram(void)
{
    return g_session;
}

/*************************************************************************************************
  * @brief    : Write life cycle data to MCU RAM
  * @param1   : The type of life cycle
  * @param2   : The value of type
  * @return   : None
*************************************************************************************************/
void write_life_cycle_value_to_ram(life_cycle_e type, int32_t num)
{
    data_change_frequent.lifeCycleData[type] = num;
    LOGD("write life data to ram,lifeCycleData[%d]=%d", type, data_change_frequent.lifeCycleData[type]);
}

/*************************************************************************************************
  * @brief    : Write session data to MCU RAM
  * @param1   : The type of session data
  * @param2   : The value of type
  * @return   : None
*************************************************************************************************/
void write_session_to_ram(session_e type, float value)
{
    g_session[data_change_frequent.g_sessionWritePos%100].num[type] = value;
}

/*************************************************************************************************
  * @brief    : Write error data to MCU RAM
  * @param1   : The type of error data
  * @param2   : The value of type
  * @return   : None
*************************************************************************************************/
void write_error_occur_num_to_ram(errorCode_e error_code, uint16_t num)
{
    data_change_frequent.errorCodeData[error_code] += num;
    LOGD("write err num to ram,errCodeData[%d]=%d", error_code, data_change_frequent.errorCodeData[error_code]);
}

/*************************************************************************************************
  * @brief    : Updata self flash data from MCU RAM to MCU flash
  * @param1   : The type of self flash data
  * @param2   : Whether to update session data flag
  * @return   : None
*************************************************************************************************/
void update_data_flash(uint8_t dataType, uint8_t sessionDataFlag)
{
    uint32_t FirstPage = 0, NbOfPages = 0;
    uint32_t Flag_Address = 0;
    uint32_t PageError = 0;
    uint32_t record_num = 0;
    uint32_t StartAddr = 0;
    uint32_t EndAddr = 0;
    uint64_t* usr_data = NULL;
    uint8_t offsetPage = 0;
    uint8_t pos_temp = 0;

    switch (dataType)
    {
        case BOOT_RECORD:
            StartAddr = FLASH_BOOT_RECORD_START_ADDR;
            EndAddr = FLASH_BOOT_RECORD_END_ADDR;

            usr_data = (uint64_t*)&boot_record_s;
            record_num = sizeof(boot_record_t) / sizeof(uint64_t);
            if((sizeof(boot_record_t)%sizeof(uint64_t)) != 0)
            {
                record_num++;
            }
            LOGD("update_boot_record_flash...");
            break;
        case USR_DATA:
            StartAddr = FLASH_USER_START_ADDR;
            EndAddr = FLASH_USER_END_ADDR;

            flash_r_data.flag_crc = CrcCheck((uint8_t *)&flash_r_data, (sizeof(flash_record_t) - 8));//calculate crc value except flag_crc(uint64_t)
            usr_data = (uint64_t*)&flash_r_data;
            /* Program the user Flash area word by word
            (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
            record_num = sizeof(flash_record_t) / sizeof(uint64_t);
            if ((sizeof(flash_record_t)%sizeof(uint64_t)) != 0)
            {
                record_num++;
            }
            LOGD("update usr data:flag_crc=%lld", flash_r_data.flag_crc);
            break;
        case DATA_CHANGE_FREQUENT:
            if (1 == sessionDataFlag)
            {
                offsetPage = ((data_change_frequent.g_sessionWritePos%100)/25);
                LOGD("update CHANGE_FREQ data:WritePos=%d!", data_change_frequent.g_sessionWritePos);
            }
            else
            {
                offsetPage = (data_change_frequent.serialNum%4);
                LOGD("update CHANGE_FREQ data:serialNum=%d!", data_change_frequent.serialNum);
            }
            StartAddr = FLASH_DATA_CHANGE_FREQUENT_START_ADDR + offsetPage*FLASH_PAGE_SIZE;
            EndAddr = FLASH_DATA_CHANGE_FREQUENT_END_ADDR + offsetPage*FLASH_PAGE_SIZE;
            __disable_irq();
            icache_disable();
            memcpy((void*)&data_change_frequent_temp, (void *)StartAddr, sizeof(data_change_frequent_t));
            icache_enable();
            __enable_irq();

            if (data_change_frequent_temp.crc != CrcCheck((uint8_t *)&data_change_frequent_temp, (sizeof(data_change_frequent_t) - 8)))
            {
                LOGD("update CHANGE_FREQ data:crc error!temp.crc=%lld",data_change_frequent_temp.crc);
                memset((void*)&data_change_frequent_temp, 0, sizeof(data_change_frequent_t));
            }

            if (0 == sessionDataFlag)
            {
                memcpy((void*)data_change_frequent.session, (void *)data_change_frequent_temp.session, sizeof(session_t)*25);
                data_change_frequent.serialNum++;
                data_change_frequent.programeNum = data_change_frequent_temp.programeNum + 1;
                data_change_frequent.crc = CrcCheck((uint8_t *)&data_change_frequent, (sizeof(data_change_frequent_t) - 8));
                usr_data = (uint64_t*)&data_change_frequent;
                LOGD("update CHANGE_FREQ data:programeNum=%d, crc=%lld", data_change_frequent.programeNum, data_change_frequent.crc);
            }
            else
            {
                pos_temp = ((data_change_frequent.g_sessionWritePos%100)/25)*25;
                memcpy((void*)data_change_frequent_temp.session, (void *)&g_session[pos_temp], sizeof(session_t)*25);

                /*LOGD("write pos = %d,session:%d,%d,%d,%d,%d\n\r", data_change_frequent.g_sessionWritePos,
                        data_change_frequent_temp.session[data_change_frequent.g_sessionWritePos%25].num[session_duration],
                        data_change_frequent_temp.session[data_change_frequent.g_sessionWritePos%25].num[max_susceptor_temp1_session],
                        data_change_frequent_temp.session[data_change_frequent.g_sessionWritePos%25].num[max_susceptor_temp2_session],
                        data_change_frequent_temp.session[data_change_frequent.g_sessionWritePos%25].num[max_bat_temp_session],
                        data_change_frequent_temp.session[data_change_frequent.g_sessionWritePos%25].num[max_cold_junc_temp_session]);*/

                if (data_change_frequent.g_sessionNum < 100)
                {
                    data_change_frequent.g_sessionNum++;
                }
                data_change_frequent.g_sessionWritePos++;
                data_change_frequent_temp.programeNum++;
                data_change_frequent_temp.g_sessionNum = data_change_frequent.g_sessionNum;
                data_change_frequent_temp.g_sessionWritePos = data_change_frequent.g_sessionWritePos;
                data_change_frequent_temp.crc = CrcCheck((uint8_t *)&data_change_frequent_temp, (sizeof(data_change_frequent_t) - 8));
                usr_data = (uint64_t*)&data_change_frequent_temp;
                LOGD("update CHANGE_FREQ data:programeNum=%d, crc=%llx", data_change_frequent_temp.programeNum, data_change_frequent_temp.crc);
            }
            /* Program the user Flash area word by word
            (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
            record_num = sizeof(data_change_frequent_t) / sizeof(uint64_t);
            if ((sizeof(data_change_frequent_t)%sizeof(uint64_t)) != 0)
            {
                record_num++;
            }
            break;
        default:
            break;
    }

    /* Unlock the Flash to enable the flash control register access *************/
    HWI_FLASH_Unlock();

    /* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

    /* Get the 1st page to erase */
    FirstPage = GetPage(StartAddr);//3

    /* Get the number of pages to erase from 1st page */
    NbOfPages = GetPage(EndAddr) - FirstPage + 1;

    /* Fill EraseInit structure*/
    EraseInitStruct.m_typeErase         = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.m_page              = FirstPage;
    EraseInitStruct.m_numberPages       = NbOfPages;
    //EraseInitStruct.m_numberPages       = 1;

    /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
    you have to make sure that these data are rewritten before they are accessed during code
    execution. If this cannot be done safely, it is recommended to flush the caches by setting the
    DCRST and ICRST bits in the FLASH_CR register. */
    HWI_StatusTypeDef status = HWI_FLASH_Erase(&EraseInitStruct, &PageError);
    if(status != HWI_OK)
    {
        LOG_NOW("status:%d,PageError:0x%x,HAL_FLASH_GetError:%x",status,PageError,HWI_FLASH_GetError());
    }



    Flag_Address = StartAddr;
    while (record_num --)
    {
        if (HWI_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,Flag_Address, *usr_data) == HWI_OK)
        {
            Flag_Address = Flag_Address + 8;
            usr_data = usr_data + 1;
        }
        else
        {
            LOG_NOW("Flash write error\r\n");
            Error_Handler();
        }
    }

    /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    HWI_FLASH_Lock();
}

/*************************************************************************************************
  * @brief    : Erase frequently changing data ( session data )
  * @param1   : The number of erase flash page
  * @return   : None
*************************************************************************************************/
void erase_frquent_data_flash(uint8_t pageNum)
{
    uint32_t FirstPage = 0, NbOfPages = 0;
    uint32_t PageError = 0;
    uint32_t StartAddr = 0;
    uint32_t EndAddr = 0;
    LOGD("erase_frquent_data_flash %d", pageNum);
    if (pageNum < FREQUENT_DATA_FLASH_PAGES_NUM)
    {
        StartAddr = FLASH_DATA_CHANGE_FREQUENT_START_ADDR + pageNum*FLASH_PAGE_SIZE;
        EndAddr = FLASH_DATA_CHANGE_FREQUENT_END_ADDR + pageNum*FLASH_PAGE_SIZE;
    }
    else if (FREQUENT_DATA_FLASH_ALL_PAGE == pageNum)
    {
        StartAddr = FLASH_DATA_CHANGE_FREQUENT_START_ADDR;
        EndAddr = FLASH_DATA_CHANGE_FREQUENT_END_ADDR + 3*FLASH_PAGE_SIZE;
        memset((void*)&data_change_frequent, 0, sizeof(data_change_frequent_t));
    }
    else
    {
        return;
    }


    /* Unlock the Flash to enable the flash control register access *************/
    HWI_FLASH_Unlock();

    /* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

    /* Get the 1st page to erase */
    FirstPage = GetPage(StartAddr);//3

    /* Get the number of pages to erase from 1st page */
    NbOfPages = GetPage(EndAddr) - FirstPage + 1;

    /* Fill EraseInit structure*/
    EraseInitStruct.m_typeErase         = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.m_page              = FirstPage;
    EraseInitStruct.m_numberPages       = NbOfPages;
    //EraseInitStruct.m_numberPages       = 1;

    /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
    you have to make sure that these data are rewritten before they are accessed during code
    execution. If this cannot be done safely, it is recommended to flush the caches by setting the
    DCRST and ICRST bits in the FLASH_CR register. */
    HWI_StatusTypeDef status = HWI_FLASH_Erase(&EraseInitStruct, &PageError);
    if(status != HWI_OK)
    {
        LOG_NOW("status:%d,PageError:0x%x,HAL_FLASH_GetError:%x",status,PageError,HWI_FLASH_GetError());
    }

    /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    HWI_FLASH_Lock();
}
void erase_data_flash(void)
{
    uint32_t FirstPage = 0, NbOfPages = 0;
    uint32_t PageError = 0;
    uint32_t StartAddr = 0;
    uint32_t EndAddr = 0;

    StartAddr = FLASH_USER_START_ADDR;
    EndAddr = FLASH_DATA_CHANGE_FREQUENT_END_ADDR + 3*FLASH_PAGE_SIZE;
    memset((void*)&data_change_frequent, 0, sizeof(data_change_frequent_t));

    /* Unlock the Flash to enable the flash control register access *************/
    HWI_FLASH_Unlock();

    /* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

    /* Get the 1st page to erase */
    FirstPage = GetPage(StartAddr);//3

    /* Get the number of pages to erase from 1st page */
    NbOfPages = GetPage(EndAddr) - FirstPage + 1;

    /* Fill EraseInit structure*/
    EraseInitStruct.m_typeErase         = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.m_page              = FirstPage;
    EraseInitStruct.m_numberPages       = NbOfPages;
    //EraseInitStruct.m_numberPages       = 1;

    /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
    you have to make sure that these data are rewritten before they are accessed during code
    execution. If this cannot be done safely, it is recommended to flush the caches by setting the
    DCRST and ICRST bits in the FLASH_CR register. */
    HWI_StatusTypeDef status = HWI_FLASH_Erase(&EraseInitStruct, &PageError);
    if(status != HWI_OK)
    {
        LOG_NOW("status:%d,PageError:0x%x,HAL_FLASH_GetError:%x",status,PageError,HWI_FLASH_GetError());
    }

    /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    HWI_FLASH_Lock();
}

/*************************************************************************************************
  * @brief    : Save life cycle data to MCU RAM
  * @param1   : None
  * @return   : None
*************************************************************************************************/
void record_life_cycle_task(void)
{
    int32_t temp = 0;

    temp = (int32_t)app_GetVbusVolt();
    if (temp > data_change_frequent.lifeCycleData[max_bat_chg_vol])
    {
        data_change_frequent.lifeCycleData[max_bat_chg_vol] = temp;
    }

    temp = (int32_t)(dev_get_adc_result()->i_sense * 1000);
    if (temp < data_change_frequent.lifeCycleData[max_bat_chg_current])
    {
        data_change_frequent.lifeCycleData[max_bat_chg_current] = temp;
    }

    temp = (int32_t)dev_get_adc_result()->zone1_temp;
    if (temp > data_change_frequent.lifeCycleData[max_susceptor_temp1_life])
    {
        data_change_frequent.lifeCycleData[max_susceptor_temp1_life] = temp;
    }

    temp = (int32_t)dev_get_adc_result()->zone2_temp;
    if (temp > data_change_frequent.lifeCycleData[max_susceptor_temp2_life])
    {
        data_change_frequent.lifeCycleData[max_susceptor_temp2_life] = temp;
    }

    temp = (int32_t)dev_get_adc_result()->bat_temp;
    if (temp > data_change_frequent.lifeCycleData[max_bat_temp_life])
    {
        data_change_frequent.lifeCycleData[max_bat_temp_life] = temp;
    }
}

/*************************************************************************************************
  * @brief    : Test error code data writing and reading
  * @param1   : None
  * @return   : None
*************************************************************************************************/
void test_error_code_read_write(void)
{
    static uint32_t flag = 0;
    if (flag>= 100)
    {
        return;
    }
    if (0 == flag%2)
    {
        write_error_occur_num_to_ram(flt_de_bat_hot_40_pre_ses, data_change_frequent.serialNum%4);
        update_data_flash(DATA_CHANGE_FREQUENT, NONE_SESSION_DATA);
        LOGD("write to flash,serialNum=%d", data_change_frequent.serialNum);
        //LOGD("data_change_frequent.errorCodeData[flt_de_bat_hot1]=%d\n\r", data_change_frequent.errorCodeData[flt_de_bat_hot_40_pre_ses]);
        //LOGD("data_change_frequent.crc=%llx\n\r", data_change_frequent.crc);
    }
    else //if (1 == flag%2)
    {
        data_change_frequent_t* ptr = get_data_change_frequent_from_flash();
        LOGD("read from flash,programeNum=%d", data_change_frequent.programeNum);
    }
    flag++;
}

/*************************************************************************************************
  * @brief    : Test life cycle data writing and reading
  * @param1   : None
  * @return   : None
*************************************************************************************************/
void test_lifeCycle_read_write(void)
{
    static uint32_t flag = 0;
    if (flag>= 100)
    {
        return;
    }
    if (0 == flag%2)
    {
        LOGD("write to flash\n\r");
        write_life_cycle_value_to_ram(max_bat_chg_vol, 0);
        write_life_cycle_value_to_ram(max_bat_chg_current, 1);
        write_life_cycle_value_to_ram(max_susceptor_temp1_life, 2);
        write_life_cycle_value_to_ram(max_susceptor_temp2_life, 3);
        write_life_cycle_value_to_ram(max_bat_temp_life, 4);
        write_life_cycle_value_to_ram(max_chg_time, 5);
        write_life_cycle_value_to_ram(total_chg_time, 6);
        write_life_cycle_value_to_ram(chag_times, 7);
        update_data_flash(DATA_CHANGE_FREQUENT, NONE_SESSION_DATA);

    }
    else //if (1 == flag%2)
    {
        LOGD("read from flash\n\r");
        data_change_frequent_t* ptr = get_data_change_frequent_from_flash();
        LOGD("flt_de_bat_hot1=0x%x\n\r", ptr->errorCodeData[flt_de_bat_hot_40_pre_ses]);
        LOGD("lifecycle:%d,%d,%d,%d,%d,%d,%d,%d\n\r",
                data_change_frequent.lifeCycleData[max_bat_chg_vol],
                data_change_frequent.lifeCycleData[max_bat_chg_current],
                data_change_frequent.lifeCycleData[max_susceptor_temp1_life],
                data_change_frequent.lifeCycleData[max_susceptor_temp2_life],
                data_change_frequent.lifeCycleData[max_bat_temp_life],
                data_change_frequent.lifeCycleData[max_chg_time],
                data_change_frequent.lifeCycleData[total_chg_time],
                data_change_frequent.lifeCycleData[chag_times]);
    }
    flag++;
}

/*************************************************************************************************
  * @brief    : Test 100 sessions data writing and reading
  * @param1   : None
  * @return   : None
*************************************************************************************************/
void test_100sessions_read_write(void)
{
    static uint32_t flag = 0;
    //uint32_t addr = 0;
    //uint8_t offsetPages = 0;
    if (flag>= 200)
    {
        return;
    }
    if (0 == flag%2)
    {
        LOGD("write to flash,flag=%d\n\r", flag);
        write_session_to_ram(session_duration, flag);
        write_session_to_ram(max_susceptor_temp1_session, flag);
        write_session_to_ram(max_susceptor_temp2_session, flag);
        write_session_to_ram(max_bat_temp_session, flag);
        write_session_to_ram(max_cold_junc_temp_session, flag);

        LOGD("write pos = %d,session:%f,%f,%f,%f,%f\n\r", data_change_frequent.g_sessionWritePos,
                g_session[data_change_frequent.g_sessionWritePos%100].num[session_duration],
                g_session[data_change_frequent.g_sessionWritePos%100].num[max_susceptor_temp1_session],
                g_session[data_change_frequent.g_sessionWritePos%100].num[max_susceptor_temp2_session],
                g_session[data_change_frequent.g_sessionWritePos%100].num[max_bat_temp_session],
                g_session[data_change_frequent.g_sessionWritePos%100].num[max_cold_junc_temp_session]);

        update_data_flash(DATA_CHANGE_FREQUENT, SESSION_DATA);
    }
    else //if (1 == flag%2)
    {
        LOGD("read from flash\n\r");
        data_change_frequent_t* ptr = get_data_change_frequent_from_flash();
        LOGD("flt_de_bat_hot1=0x%x\n\r", ptr->errorCodeData[flt_de_bat_hot_40_pre_ses]);
        LOGD("lifecycle:%d,%d,%d,%d,%d,%d,%d,%d\n\r",
                data_change_frequent.lifeCycleData[max_bat_chg_vol],
                data_change_frequent.lifeCycleData[max_bat_chg_current],
                data_change_frequent.lifeCycleData[max_susceptor_temp1_life],
                data_change_frequent.lifeCycleData[max_susceptor_temp2_life],
                data_change_frequent.lifeCycleData[max_bat_temp_life],
                data_change_frequent.lifeCycleData[max_chg_time],
                data_change_frequent.lifeCycleData[total_chg_time],
                data_change_frequent.lifeCycleData[chag_times]);

        /*offsetPages = ((data_change_frequent.g_sessionWritePos-1)%100)/25;
        addr = FLASH_DATA_CHANGE_FREQUENT_START_ADDR + offsetPages*FLASH_PAGE_SIZE;
        memcpy((void*)&data_change_frequent_temp, (void *)addr, sizeof(data_change_frequent_t));
        LOGD("read pos = %d,session:%d,%d,%d,%d,%d\n\r", (data_change_frequent.g_sessionWritePos-1),
                data_change_frequent_temp.session[(data_change_frequent.g_sessionWritePos-1)%25].num[session_duration],
                data_change_frequent_temp.session[(data_change_frequent.g_sessionWritePos-1)%25].num[max_susceptor_temp1_session],
                data_change_frequent_temp.session[(data_change_frequent.g_sessionWritePos-1)%25].num[max_susceptor_temp2_session],
                data_change_frequent_temp.session[(data_change_frequent.g_sessionWritePos-1)%25].num[max_bat_temp_session],
                data_change_frequent_temp.session[(data_change_frequent.g_sessionWritePos-1)%25].num[max_cold_junc_temp_session]);*/
        LOGD("write pos = %d,session:%f,%f,%f,%f,%f\n\r", data_change_frequent.g_sessionWritePos,
                g_session[(data_change_frequent.g_sessionWritePos -1)%100].num[session_duration],
                g_session[(data_change_frequent.g_sessionWritePos -1)%100].num[max_susceptor_temp1_session],
                g_session[(data_change_frequent.g_sessionWritePos -1)%100].num[max_susceptor_temp2_session],
                g_session[(data_change_frequent.g_sessionWritePos -1)%100].num[max_bat_temp_session],
                g_session[(data_change_frequent.g_sessionWritePos -1)%100].num[max_cold_junc_temp_session]);
    }
    flag++;
}


