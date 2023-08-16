#include "HWI_hal.h"
#include "HWI_wdg.h"

void hwi_IWDG_Init(void)
{
	dbg_periph_enable(DBG_FWDGT_HOLD);
	
  rcu_osci_on(RCU_IRC32K);
  rcu_osci_stab_wait(RCU_IRC32K);
  fwdgt_config(WDT_TIME_OUT_MS,FWDGT_PSC_DIV32);
  
  fwdgt_enable();
	
	fwdgt_counter_reload();
}

void hwi_FeedIwdg(void)
{
  //fwdgt_counter_reload(); debug lgf
}
