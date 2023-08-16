#include "rtc.h"
#include "log.h"

//RTC_HandleTypeDef hrtc;
//RTC_DateTypeDef rtc_date;
//RTC_TimeTypeDef rtc_time;

void RtcEnableDomainRegisterInit(void)
{
    hwi_RtcEnableDomainRegisterInit();
}


//void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
//{
//
//}


void WriteBackupRegister(BKP_DRx_enum BKP_DRx,uint32_t data)
{
	hwi_WriteBackupRegister(BKP_DRx, data);
}

uint32_t ReadBackupRegister(BKP_DRx_enum BKP_DRx)
{
    return hwi_ReadBackupRegister(BKP_DRx);
}
