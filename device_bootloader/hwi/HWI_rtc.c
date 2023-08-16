#include "HWI_Hal.h"


void hwi_RtcEnableDomainRegisterInit(void)
{
  rcu_periph_clock_enable(RCU_PMU);
  rcu_periph_clock_enable(RCU_RTC);	
  pmu_backup_write_enable();
}

//backup data cnt 0--19(word 32bit)    
void hwi_WriteBackupRegister(BKP_DRx_enum BKP_DRx,uint32_t data)
{
  REG32((RTC) + BKP_DRx)=data;  
}

uint32_t hwi_ReadBackupRegister(BKP_DRx_enum BKP_DRx)
{
    uint32_t ret ;
    ret = REG32((RTC) +  BKP_DRx);	
    return ret;
}
