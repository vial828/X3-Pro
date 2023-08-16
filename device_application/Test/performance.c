#include "kernel.h"
#include "log.h"
#include "performance.h"


#define TEST_TIME(sec)   (sec*1000)
#define SECOND   10


void test_performance(void)
{
	static uint32_t markTick = 0;
	static uint32_t cycleCnt = 0;

	MARK_TICK(markTick);
	cycleCnt++;
	if(TicsSince(markTick)>=TEST_TIME(SECOND)){
		LOGD("-------------------------------------%d loops per second\r\n",(cycleCnt/SECOND));
		cycleCnt = 0;
		markTick = GetTick();
	}
}

