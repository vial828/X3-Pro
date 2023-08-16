#include "HWI_trng.h"
//#include "log.h"
#include "HWI_Hal.h"

static ErrStatus trng_ready_check(void);

/*************************************************************************************************
 * @brief   :get random seed by XOR-ing adc value of floating pin and current tick
 * @parm    :void
 * @return  :random seed for rand()
*************************************************************************************************/
uint32_t hwi_trng_generate(void)
{
    uint32_t seed=0;

	if(SUCCESS == trng_ready_check()){
		seed = trng_get_true_random_data();
	}
			
    return seed;
}

void hwi_TRNG_Init(void)
{
    ErrStatus reval = SUCCESS;
    
    /* TRNG module clock enable */
    rcu_periph_clock_enable(RCU_TRNG);
    
    /* TRNG registers reset */
    trng_deinit();
    trng_enable();
    /* check TRNG work status */
    trng_ready_check();
}

/*!
    \brief      check whether the TRNG module is ready
    \param[in]  none
    \param[out] none
    \retval     ErrStatus: SUCCESS or ERROR
*/
static ErrStatus trng_ready_check(void)
{
    uint32_t timeout = 0;
    FlagStatus trng_flag = RESET;
    ErrStatus reval = SUCCESS;
    
    /* check wherther the random data is valid */
    do{
        timeout++;
        trng_flag = trng_flag_get(TRNG_FLAG_DRDY);
    }while((RESET == trng_flag) &&(0xFFFF > timeout));
    
    if(RESET == trng_flag){
        /* ready check timeout */
//        printf("Error: TRNG can't ready \r\n");
        trng_flag = trng_flag_get(TRNG_FLAG_CECS);
//        printf("Clock error current status: %d \r\n", trng_flag);
        trng_flag = trng_flag_get(TRNG_FLAG_SECS);
//        printf("Seed error current status: %d \r\n", trng_flag);  
        reval = ERROR;
    }
    
    /* return check status */
    return reval;
}



