#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include <stdlib.h>
#include "gd32w51x.h"

#define RED             (uint16_t)0xF800
#define GREEN           (uint16_t)0x07E0
#define BLUE            (uint16_t)0x001F
#define WHITE           (uint16_t)0xFFFF
#define BLACK           (uint16_t)0x0000
#define YELLOW          (uint16_t)0xFFE0

/* grays */
#define GRAY0           (uint16_t)0xEF7D
#define GRAY1           (uint16_t)0x8410
#define GRAY2           (uint16_t)0x4208

/* PB12 tft cs */
#define LCD_CS_SET      ((uint32_t)(GPIO_BOP(GPIOB) = GPIO_PIN_12))
#define LCD_CS_CLR      ((uint32_t)(GPIO_BC(GPIOB) = GPIO_PIN_12))

/* PB11 tft rs/dc */
#define LCD_RS_SET      ((uint32_t)(GPIO_BOP(GPIOB) = GPIO_PIN_3))
#define LCD_RS_CLR      ((uint32_t)(GPIO_BC(GPIOB) = GPIO_PIN_3))

/* PB10 tft rst */
#define LCD_RST_SET     ((uint32_t)(GPIO_BOP(GPIOC) = GPIO_PIN_7))
#define LCD_RST_CLR     ((uint32_t)(GPIO_BC(GPIOC) = GPIO_PIN_7))

/* AVDDEN PB4 config*/
#define LCD_AVDDEN_SET      ((uint32_t)(GPIO_BOP(GPIOB) = GPIO_PIN_4))
#define LCD_AVDDEN_CLR      ((uint32_t)(GPIO_BC(GPIOB) = GPIO_PIN_4))

/* 2V8_SW PA5  config */
#define EN_2V8_SET     ((uint32_t)(GPIO_BOP(GPIOA) = GPIO_PIN_5))
#define EN_2V8_CLR     ((uint32_t)(GPIO_BC(GPIOA) = GPIO_PIN_5))

#define  HBM_ENABLE   0x0
#define  HBM_DISABLE  0x02

/* initialize the lcd */
void dev_oled_init(void);
/* set lcd display region */
void dev_oled_set_region(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);
/* set the start display point of lcd */
void dev_oled_set_xy(uint16_t x, uint16_t y);
/* clear the lcd */
void lcd_clear(const unsigned char* data);
void dis_pic(const unsigned char* data,uint32_t size, uint32_t begin);
void oled_dis_demo(void);
void dev_dma_config(void);
void dev_oled_reinit(uint8_t brightness,uint8_t HBM_state);
void dev_oled_power_off(void);
#endif /* LCD_DRIVER_H */



