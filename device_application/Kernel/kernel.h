/******************************************
 * kernel.h
 *
 *  Created on: 2019Äê11ÔÂ08ÈÕ
 *      Author: Administrator
 ******************************************/
#include "message.h"
#include "stddef.h"
#include "comm.h"
#include "log.h"
#include "FreeRTOS.h"
#include "task.h"


#ifndef KERNEL_KERNEL_H_
#define KERNEL_KERNEL_H_

//#define NICO  
//#define TEST_ADC
//#define TEST_PERFORMANCE


#define MARK_TICK(markTick)      do{  \
																		static uint8_t isBegin = 0;  \
																		if(!isBegin){                \
																			markTick = GetTick();      \
																			isBegin = 1;               \
																		}                            \
																 }while(0)                      \

#define WAIT_ONECE(time)					do{                                  \
																			static uint32_t markTick = 0;    \
																			static uint8_t  hasFirst = 0;    \
																			MARK_TICK(markTick);		           \
																			if(!hasFirst){                   \
																				if(TicsSince(markTick)>=time){ \
																						hasFirst = 1;              \
																				}else{												 \
																					return;											 \
																				}															 \
																		  }                                \
																	}while(0)														 \

#define EXECUTE_ONECE_AFTER_TIME(func, time)		do{                                  \
																										static uint32_t markTick = 0;    \
																										static uint8_t  hasFirst = 0;    \
																										MARK_TICK(markTick);	           \
																										if(!hasFirst){                   \
																											if(TicsSince(markTick)>=time){ \
																													func();                    \
																													hasFirst = 1;              \
																											}else{												 \
																												return;											 \
																											}															 \
																										}                                \
																								}while(0)														 \




uint32_t GetTick(void);
uint32_t TicsSince( uint32_t mark);
void printAllTaskStatus(void);
void monitor_func(void *para);                                                                                                

#endif /* KERNEL_KERNEL_H_ */
