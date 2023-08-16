/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "HWI_Hal.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dev_uart.h"

#include "iap_config.h"

#include "iap.h"

#include "ymodem.h"

#include "securityBoot.h"
#include "flash.h"
#include "dev_flash_ex.h"
#include "version.h"
//#include "test_flash.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t signature[SIGNATURE_LENGTH] = {0x00};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int8_t status = 0;

void BKP_Config(void)
{
#if 0
  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_RTC);

  /* Enables the PWR Clock and Enables access to the backup domain */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  LL_PWR_EnableBkUpAccess();
  #endif
}



void boot_version_check(void)
{
    boot_record_t * p_boot_record = get_boot_record_from_flash();
    uint8_t len = strlen(VERSION);

    if(len <= 32)
    {
        if(strlen((char*)p_boot_record->bootloader_version) == len &&
            strncmp((char*)p_boot_record->bootloader_version, VERSION, len) == 0)
        {
            return ;
        }
        else{
            memset(p_boot_record->bootloader_version, 0, sizeof(p_boot_record->bootloader_version));
            memcpy(p_boot_record->bootloader_version, VERSION, len);
            update_boot_flash();
        }
    }
}
/* USER CODE END 0 */




/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
		hwi_HAL_Init();
  /* USER CODE BEGIN 1 */
#ifdef RDP2
    Flash_EnableRDP_WRP_Pages(FLASH_BASE, DATA_START_ADDR - 1);
#endif
    BKP_Config();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/
    uint32_t time_1s = 0;


    boot_version_check();
    File_Size = *(__IO uint32_t*)(FILE_SIZE_ADDR);
    if(File_Size != 0 && File_Size != 0xffffffff)
    {
        if(IAP_ReadFlag() != UPDATE_FLAG_DATA)
        {
            if (hwi_ReadBackupRegister(BKP_DR4) != 0xCC){
                memcpy(signature, (uint8_t*)SIGNATURE_ADDR, 32);
                securityboot();
                hwi_WriteBackupRegister(BKP_DR4,0xCC);
            }
            IAP_JumpToApp();
        }
    }


  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* Tick clock   */
  hwi_SysTick_Config();



  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */
    Dev_UART_Init(1);
 //   hwi_IWDG_Init(); debug lgf
    vd_g_CommandExp_PackSendData(0xAB);

    time_1s = hwi_SysTick_GetTick();
    gUartRcvDataFlag_time = 0;


    /* Initialize all configured peripherals */

  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
        status = IAP_Update();

        if((hwi_SysTick_GetTick() - gUartRcvDataFlag_time) > 60 * 1000)
        {
            hwi_SOFT_RESET();
        }

        if((hwi_SysTick_GetTick() - time_1s) > 1000)
        {
            time_1s = hwi_SysTick_GetTick();
            hwi_FeedIwdg();
        }
        if(status == 1)
        {
            securitybootAfterUpdate();
            hwi_WriteBackupRegister(BKP_DR4,0xCC);
            JumpPreparation();
            hwi_HAL_Delay(500);

            hwi_SOFT_RESET();
        }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
#if 0
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2)
  {
    Error_Handler();
  };

  /* HSI configuration and activation */
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() != 1)
  {
  };

  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_1, 8, LL_RCC_PLLR_DIV_2);
  LL_RCC_PLL_Enable();
  LL_RCC_PLL_EnableDomain_SYS();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  };

  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  /* Sysclk activation on the main PLL */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  };

  /* Set APB1 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

  LL_Init1msTick(64000000);

  LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(64000000);

  LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
  #endif

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
