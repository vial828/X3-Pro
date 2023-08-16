
#include "HWI_Hal.h"
#include "HWI_systick.h"
#include "kernel.h"

volatile static uint32_t tick=0;




void hwi_SysTick_Config(void)
{
    /* setup systick timer for 1000Hz interrupts */
    if(SysTick_Config(SystemCoreClock / 1000U)){
        /* capture error */
        while(1){
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

void hwi_SysTick_Handler(void)
{

    tick++;
//	TimeEventControl();

	
}

uint32_t hwi_SysTick_GetTick(void)
{
    return tick;
}

