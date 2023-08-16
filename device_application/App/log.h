#ifndef __LOG_H
#define __LOG_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "HWI_Hal.h"
#include "uart.h"
#include "comm.h"

#define LOG_ALL			     	(0x7f)
#define LOG_NULL			    (0x00)
#define LOG_FCT     			(1u << 0)
#define LOG_DEBUG         (1u << 1)
#define LOG_INFO     			(1u << 2)
#define LOG_ERR     		  (1u << 3)

#define ONE_LOG_SIZE       256

void LogSuspend(void);
void LogResume(void);

//#define LOG_ASSERT     		(1<<4)
//#define LOG_CUSTOM        (1<<5)

extern uint8_t log_mask;
extern uint8_t uart_tx_enalbe_flag;

#define LOG_NOW(FMT,ARGS...)     						\
do{    																						\
			if(uart_tx_enalbe_flag)												\
			{																						\
					log_print(LOG_INFO,FMT, ##ARGS);                 \
					while(UartStatusCheck() != SET);         \
					comm_send_proc();												\
					while(UartStatusCheck() != SET);				\
			}																						\
}while(0)

#define ASSERT(EXP)           do{                                                                     \
                                if(!(EXP))                                                            \
                                {                                                                     \
                                    LOG_NOW("assert! %s %d (%s)\n", __FILE__, __LINE__, #EXP); \
																		while(1);   \
                                }                                                                     \
                              }while(0)

#define LOGF(FMT,ARGS...)     do{														\
																			if(log_mask & LOG_FCT)					\
																			log_print(LOG_FCT,FMT,##ARGS);		  \
																	 }while(0)

#define LOGD(FMT,ARGS...)     do{														\
																			if(log_mask & LOG_DEBUG)					\
																			log_print(LOG_DEBUG,FMT,##ARGS);		  \
																	 }while(0)

#define LOGI(FMT,ARGS...)     do{														\
																			if(log_mask & LOG_INFO)					\
																			log_print(LOG_INFO,FMT,##ARGS);		  \
																	 }while(0)


#define LOGE(FMT,ARGS...)     do{														\
																			if(log_mask & LOG_ERR)					\
																			log_print(LOG_ERR,FMT,##ARGS);		  \
																	 }while(0)

/*#define LOGC(FMT,ARGS...)     do{														\
																			if(log_mask & LOG_CUSTOM)					\
																			log_print(LOG_CUSTOM,FMT,##ARGS);		  \
																	 }while(0)	*/																 





/****************function****************/
//void log_print(const char *fmt, ...);
void log_print(uint8_t level, const char *fmt, ...);
																	 
#define  Error_Handler()      do																												\
															{																													\
																  while(UartStatusCheck() != SET);											\
																	log_print(LOG_ERR, "error hander :%s ,%d",__FILE__, __LINE__); \
																	while(UartStatusCheck() != SET);         \
																	comm_send_proc();												\
																	while(UartStatusCheck() != SET);				\
																	while(1);																							\
															}while(0)																	 

#ifdef __cplusplus
}
#endif

#endif
