/*
    \file  dev_oled_driver.c
    \brief oled driver functions
    \version 2023-02-10
*/


#include "gd32w51x.h"
#include "dev_oled_driver.h"
//#include "lcd_data.h"
#include "HWI_Hal.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "gd25qxx.h"
#include "task.h"
#include "HWI_gpio.h"

#ifdef H_VIEW
    #define X_MAX_PIXEL         (uint16_t)320
    #define Y_MAX_PIXEL         (uint16_t)240
#else
    #define X_MAX_PIXEL         (uint16_t)240
    #define Y_MAX_PIXEL         (uint16_t)320
#endif

static uint8_t spi_write_byte(uint32_t spi_periph,uint8_t byte);
static void spi1_init(void);
static void lcd_write_index(uint8_t index);
static void lcd_write_data(uint8_t data);
static void lcd_write_data_16bit(uint8_t datah,uint8_t datal);
static void lcd_reset(void);

dma_single_data_parameter_struct dma_init_struct;
uint8_t flash_rx_buffer[76140];

/*!
    \brief      send a byte through the SPI interface and return a byte received from the SPI bus
    \param[in]  spi_periph: SPIx(x=0,1)
    \param[in]  byte: data to be send
    \param[out] none
    \retval     the value of the received byte
*/
static uint8_t spi_write_byte(uint32_t spi_periph,uint8_t byte)
{
    while(RESET == (SPI_STAT(spi_periph)&SPI_FLAG_TBE));
    SPI_DATA(spi_periph) = byte;

    while(RESET == (SPI_STAT(spi_periph)&SPI_FLAG_RBNE));
    return(SPI_DATA(spi_periph));
} 

/*!
    \brief      initialize SPI1
*/
static void spi1_init(void)
{
    spi_parameter_struct spi_init_struct;
    rcu_periph_clock_enable(RCU_SPI1);

    /* SPI1 parameter config */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_2;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;

    spi_init(SPI1, &spi_init_struct);
    spi_enable(SPI1);
}
//extern const unsigned char gImage_screen[153600] ;
void dev_dma_config(void)
{
    dma_single_data_para_struct_init(&dma_init_struct);
    rcu_periph_clock_enable(RCU_DMA0);
    /* SPI1 transmit dma config: DMA1,DMA_CH3*/
    dma_deinit(DMA0, DMA_CH4);
    dma_init_struct.periph_addr         = (uint32_t)&SPI_DATA(SPI1);
 //   dma_init_struct.memory0_addr        = (uint32_t)gImage_screen;
    dma_init_struct.direction           = DMA_MEMORY_TO_PERIPH;
    dma_init_struct.priority            = DMA_PRIORITY_LOW;
    dma_init_struct.periph_memory_width = DMA_MEMORY_WIDTH_8BIT;
  //  dma_init_struct.number              = 38;
    dma_init_struct.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.circular_mode       = DMA_CIRCULAR_MODE_DISABLE;
    dma_single_data_mode_init(DMA0, DMA_CH4, &dma_init_struct);
    /* connect DMA1_CH3 to SPI1 TX */
    dma_channel_subperipheral_select(DMA0, DMA_CH4, DMA_SUBPERI0);
}

/*!
    \brief      write the register address
    \param[in]  index: the value of register address to be written
*/
static void lcd_write_index(uint8_t index)
{
    LCD_RS_CLR;
    spi_write_byte(SPI1,index);
}

/*!
    \brief      write the register data
    \param[in]  data: the value of register data to be written
*/
static void lcd_write_data(uint8_t data)
{
    LCD_RS_SET;
    spi_write_byte(SPI1,data);
}

/*!
    \brief      write the register data(an unsigned 16-bit data)
    \param[in]  datah: the high 8bit of register data to be written
    \param[in]  datal: the low 8bit of register data to be written
*/
static void lcd_write_data_16bit(uint8_t datah,uint8_t datal)
{
    lcd_write_data(datah);
    lcd_write_data(datal);
}

/*!
    \brief      reset the lcd
*/
static void lcd_reset(void)
{
    LCD_RST_CLR;
    hwi_delay_ms(100);
    LCD_RST_SET;
    hwi_delay_ms(50);
}

/*!
    \brief      initialize the lcd
*/
	    //*********************
	    // Panel:Truly 0.91" 90x282
	    // Purpose: 1st Light-on
	    // Ver.:
	    // V1.0 : First version
	    //*********************

	    //IC WRITE
static void init_oled()
{
	    lcd_write_index(0xFE);lcd_write_data(0x01);
	    lcd_write_index(0x05);lcd_write_data(0x20); //128RGB, CRL=2b'00 (COG on bottom)
	    lcd_write_index(0x06);lcd_write_data(0x47); //NL=284
	    lcd_write_index(0x0E);lcd_write_data(0x83); //Frequency=Hsyncx2 ; AVDD=6.2V at normal
	    lcd_write_index(0x0F);lcd_write_data(0x83); //Frequency=Hsyncx2 ; AVDD=6.2V at idle
	    lcd_write_index(0x11);lcd_write_data(0xC2); //VCL=-3.3V, VCL pump frequency set Hsync x 6 at normal
	    lcd_write_index(0x12);lcd_write_data(0xC2); //VCL=-3.3V, VCL pump frequency set Hsync x 6 at idle
	    lcd_write_index(0x18);lcd_write_data(0x66); //VGHR=6V
	    lcd_write_index(0x19);lcd_write_data(0x88); //VGLR=-6V
	    lcd_write_index(0x1D);lcd_write_data(0x01); //SW EQ rising time set 125ns
	    lcd_write_index(0x1E);lcd_write_data(0x01); //SW EQ falling time set 125ns
	    lcd_write_index(0x1F);lcd_write_data(0x01); //VSR EQ rising time set 125ns
	    lcd_write_index(0x20);lcd_write_data(0x01); //VSR EQ falling time set 125ns

	    lcd_write_index(0x25);lcd_write_data(0x01); //PSEL=2b'00
	    lcd_write_index(0x26);lcd_write_data(0xC0); //1C0'h=448'd, 448x0.125=56us
	    lcd_write_index(0x27);lcd_write_data(0x08); //VBP=8H
	    lcd_write_index(0x28);lcd_write_data(0x08); //VFP=8H
	    lcd_write_index(0x29);lcd_write_data(0x01); //Disable skip frame function
	    lcd_write_index(0x2A);lcd_write_data(0x01); //PSEL=2b'00
	    lcd_write_index(0x2B);lcd_write_data(0xC0); //1C0'h=448'd, 448x0.125=56us
	    lcd_write_index(0x2D);lcd_write_data(0x08); //VBP=8H
	    lcd_write_index(0x2F);lcd_write_data(0x08); //VFP=8H
	    lcd_write_index(0x30);lcd_write_data(0x43); //AOD, 60/4=15Hz

	    lcd_write_index(0x37);lcd_write_data(0x0C); //Pre-charge=VGSP
	    lcd_write_index(0x3A);lcd_write_data(0x01); //T1_SD=0.125us
	    lcd_write_index(0x3B);lcd_write_data(0x00); //TP_SD=0us
	    lcd_write_index(0x3D);lcd_write_data(0x08); //TH_SD=1us
	    lcd_write_index(0x3F);lcd_write_data(0x20); //TSW_SD=4us
	    lcd_write_index(0x40);lcd_write_data(0x08); //THSW_SD=1us
	    lcd_write_index(0x41);lcd_write_data(0x03); //THSD_SD=0.375us
	    lcd_write_index(0x49);lcd_write_data(0xC7); //SD control
	    lcd_write_index(0x5B);lcd_write_data(0x10); //Enable VREFN
	    lcd_write_index(0x62);lcd_write_data(0x1C); //VREFN=-3.3V
	    lcd_write_index(0x63);lcd_write_data(0x1C); //VREFN=-3.3V
	    lcd_write_index(0x66);lcd_write_data(0x90); //AOD internal power
	    lcd_write_index(0x69);lcd_write_data(0x82); //NOR external power
	    lcd_write_index(0x6A);lcd_write_data(0x0B); //OVSS=-3.3V, SGM38045 11 pulse
	    lcd_write_index(0x6B);lcd_write_data(0x06); //OVDD=3.3V, SGM38045 6 pulse
	    lcd_write_index(0x6D);lcd_write_data(0x19); //VGMP/VGSP off on skip frame
	    lcd_write_index(0x70);lcd_write_data(0xA8); //On blanking 1: SD to GND; other region: SD to VGMP
	    lcd_write_index(0x72);lcd_write_data(0x0B); //AOD OVDD=3.3V
	    lcd_write_index(0x73);lcd_write_data(0x1E); //AOD OVSS=-3.3V
	    lcd_write_index(0x74);lcd_write_data(0x0C); //OVDD power from VOUT; SD power from AVDD
	    lcd_write_index(0x84);lcd_write_data(0x10); //OVSS delay OVDD  **
	    lcd_write_index(0x8A);lcd_write_data(0x11); //VGH=AVDD ; VGL=VCL-VCI
	    lcd_write_index(0x8E);lcd_write_data(0x18); //Enable VOUT
	    lcd_write_index(0x8F);lcd_write_data(0xC2); //Same as 1140h/1240h
	    lcd_write_index(0x92);lcd_write_data(0x09); //VOUT=5.6V at normal
	    lcd_write_index(0x93);lcd_write_data(0x09); //VOUT=5.6V at idle
	    lcd_write_index(0x99);lcd_write_data(0x10); //Normal/AOD is gamma 1, HBM is gamma2

	    lcd_write_index(0xFE);lcd_write_data(0x02);  //Gamma 1 setting
	    lcd_write_index(0xA9);lcd_write_data(0xD8); //VGMP=4.7V
	    lcd_write_index(0xAA);lcd_write_data(0x18); //VGSP=0.5V
	    lcd_write_index(0xAB);lcd_write_data(0x00);

	    lcd_write_index(0xFE);lcd_write_data(0x03);  //Gamma 2 setting
	    lcd_write_index(0xA9);lcd_write_data(0xD8); //VGMP=4.7V
	    lcd_write_index(0xAA);lcd_write_data(0x18); //VGSP=0.5V
	    lcd_write_index(0xAB);lcd_write_data(0x00);

	    //SN_STV SET
	    lcd_write_index(0xfe);lcd_write_data(0x04);
	    lcd_write_index(0x4c);lcd_write_data(0x89);
	    lcd_write_index(0x4d);lcd_write_data(0x00);
	    lcd_write_index(0x4e);lcd_write_data(0x00);
	    lcd_write_index(0x4f);lcd_write_data(0x40);
	    lcd_write_index(0x50);lcd_write_data(0x01);
	    lcd_write_index(0x51);lcd_write_data(0x01);
	    lcd_write_index(0x52);lcd_write_data(0x4e);
	    //SN_CK1 SET
	    lcd_write_index(0x00);lcd_write_data(0xcc);
	    lcd_write_index(0x01);lcd_write_data(0x00);
	    lcd_write_index(0x02);lcd_write_data(0x02);
	    lcd_write_index(0x03);lcd_write_data(0x00);
	    lcd_write_index(0x04);lcd_write_data(0x20);
	    lcd_write_index(0x05);lcd_write_data(0x07);
	    lcd_write_index(0x06);lcd_write_data(0xFA);
	    lcd_write_index(0x07);lcd_write_data(0xB8);
	    lcd_write_index(0x08);lcd_write_data(0x08);
	    //SN_CK2 SET
	    lcd_write_index(0x09);lcd_write_data(0xcc);
	    lcd_write_index(0x0a);lcd_write_data(0x00);
	    lcd_write_index(0x0b);lcd_write_data(0x02);
	    lcd_write_index(0x0c);lcd_write_data(0x00);
	    lcd_write_index(0x0d);lcd_write_data(0x20);
	    lcd_write_index(0x0e);lcd_write_data(0x06);
	    lcd_write_index(0x0f);lcd_write_data(0xFA);
	    lcd_write_index(0x10);lcd_write_data(0xB8);
	    lcd_write_index(0x11);lcd_write_data(0x08);
	    //EM_CK1 SET
	    lcd_write_index(0x12);lcd_write_data(0xcc);
	    lcd_write_index(0x13);lcd_write_data(0x00);
	    lcd_write_index(0x14);lcd_write_data(0x02);
	    lcd_write_index(0x15);lcd_write_data(0x00);
	    lcd_write_index(0x16);lcd_write_data(0x20);
	    lcd_write_index(0x17);lcd_write_data(0x07);
	    lcd_write_index(0x18);lcd_write_data(0x28);
	    lcd_write_index(0x19);lcd_write_data(0x96);
	    lcd_write_index(0x1a);lcd_write_data(0x08);
	    //EM_CK2 SET
	    lcd_write_index(0x1b);lcd_write_data(0xcc);
	    lcd_write_index(0x1c);lcd_write_data(0x00);
	    lcd_write_index(0x1d);lcd_write_data(0x02);
	    lcd_write_index(0x1e);lcd_write_data(0x00);
	    lcd_write_index(0x1f);lcd_write_data(0x20);
	    lcd_write_index(0x20);lcd_write_data(0x06);
	    lcd_write_index(0x21);lcd_write_data(0x28);
	    lcd_write_index(0x22);lcd_write_data(0x96);
	    lcd_write_index(0x23);lcd_write_data(0x08);
	    //EM_STV SET
	    lcd_write_index(0x53);lcd_write_data(0x8a);
	    lcd_write_index(0x54);lcd_write_data(0x00);
	    lcd_write_index(0x55);lcd_write_data(0x02);
	    lcd_write_index(0x56);lcd_write_data(0x05);
	    lcd_write_index(0x57);lcd_write_data(0x00);
	    lcd_write_index(0x58);lcd_write_data(0x03);
	    lcd_write_index(0x59);lcd_write_data(0x28);
	    lcd_write_index(0x5a);lcd_write_data(0x28);
	    lcd_write_index(0x5b);lcd_write_data(0x02);

	    lcd_write_index(0x78);lcd_write_data(0x0E); //Gamma OP control=1

	    lcd_write_index(0xFE);lcd_write_data(0x0C);  //GOA mapping
	    lcd_write_index(0x12);lcd_write_data(0x10);  //VSR_L1= SW4
	    lcd_write_index(0x13);lcd_write_data(0x11);  //VSR_L2= SW5
	    lcd_write_index(0x14);lcd_write_data(0x12);  //VSR_L3= SW6
	    lcd_write_index(0x15);lcd_write_data(0x19);  //VSR_L4= VGHR
	    lcd_write_index(0x16);lcd_write_data(0x19);  //VSR_L5= VGHR
	    lcd_write_index(0x17);lcd_write_data(0x19);  //VSR_L6= VGHR

	    lcd_write_index(0x18);lcd_write_data(0x01);  //VSR_L7= VSR2= SN_CK2
	    lcd_write_index(0x19);lcd_write_data(0x00);  //VSR_L8= VSR1= SN_CK1
	    lcd_write_index(0x1A);lcd_write_data(0x08);  //VSR_L9= VST= SN_STV
	    lcd_write_index(0x1B);lcd_write_data(0x19);  //VSR_L10= VGHR
	    lcd_write_index(0x1C);lcd_write_data(0x19);  //VSR_L11= VGHR
	    lcd_write_index(0x1D);lcd_write_data(0x19);  //VSR_L12= VGHR

	    lcd_write_index(0x1E);lcd_write_data(0x19);  //VSR_R1= VGHR
	    lcd_write_index(0x1F);lcd_write_data(0x19);  //VSR_R2= VGHR
	    lcd_write_index(0x20);lcd_write_data(0x19);  //VSR_R3= VGHR
	    lcd_write_index(0x21);lcd_write_data(0x0F);  //VSR_R4= SW3
	    lcd_write_index(0x22);lcd_write_data(0x0E);  //VSR_R5= SW2
	    lcd_write_index(0x23);lcd_write_data(0x0D);  //VSR_R6= SW1

	    lcd_write_index(0x24);lcd_write_data(0x19);  //VSR_R7= VGHR
	    lcd_write_index(0x25);lcd_write_data(0x19);  //VSR_R8= VGHR
	    lcd_write_index(0x26);lcd_write_data(0x19);  //VSR_R9= VGHR
	    lcd_write_index(0x27);lcd_write_data(0x03);  //VSR_R10= VSR4= EM_CK2
	    lcd_write_index(0x28);lcd_write_data(0x02);  //VSR_R11= VSR3= EM_CK1
	    lcd_write_index(0x29);lcd_write_data(0x09);  //VSR_R12= VSTE= EM_STV

	    //SW
	    //Odd frame
	    lcd_write_index(0x63);lcd_write_data(0x14);
	    lcd_write_index(0x64);lcd_write_data(0x41);
	    lcd_write_index(0x65);lcd_write_data(0x25);
	    lcd_write_index(0x66);lcd_write_data(0x52);
	    lcd_write_index(0x67);lcd_write_data(0x36);
	    lcd_write_index(0x68);lcd_write_data(0x63);

	    //Even frame
	    lcd_write_index(0x6F);lcd_write_data(0x14);
	    lcd_write_index(0x70);lcd_write_data(0x41);
	    lcd_write_index(0x71);lcd_write_data(0x25);
	    lcd_write_index(0x72);lcd_write_data(0x52);
	    lcd_write_index(0x73);lcd_write_data(0x36);
	    lcd_write_index(0x74);lcd_write_data(0x63);

	    //SD
	    //Odd frame
	    lcd_write_index(0x7B);lcd_write_data(0x14); //R1-R2
	    lcd_write_index(0x7C);lcd_write_data(0x41); //R2-R1
	    lcd_write_index(0x7D);lcd_write_data(0x25); //G1-G2
	    lcd_write_index(0x7E);lcd_write_data(0x52); //G2-G1
	    lcd_write_index(0x7F);lcd_write_data(0x36); //B1-B2
	    lcd_write_index(0x80);lcd_write_data(0x63); //B2-B1

	    //Even frame
	    lcd_write_index(0x87);lcd_write_data(0x14); //R1-R2
	    lcd_write_index(0x88);lcd_write_data(0x41); //R2-R1
	    lcd_write_index(0x89);lcd_write_data(0x25); //G1-G2
	    lcd_write_index(0x8A);lcd_write_data(0x52); //G2-G1
	    lcd_write_index(0x8B);lcd_write_data(0x36); //B1-B2
	    lcd_write_index(0x8C);lcd_write_data(0x63); //B2-B1

	    //IC WRITE
	    lcd_write_index(0xFE);lcd_write_data(0x01);
	    lcd_write_index(0x9F);lcd_write_data(0x92); //Enable new resolution, shift 18 pixel
	    lcd_write_index(0xA0);lcd_write_data(0x5A); //90RGB

	    //IC WRITE
	    //APRC
	    lcd_write_index(0xFE);lcd_write_data(0x05);
	    lcd_write_index(0x4C);lcd_write_data(0x01);
	    lcd_write_index(0x4D);lcd_write_data(0x0A);
	    lcd_write_index(0x4E);lcd_write_data(0x04);
	    lcd_write_index(0x4F);lcd_write_data(0x00);
	    lcd_write_index(0x50);lcd_write_data(0xA8);
	    lcd_write_index(0x51);lcd_write_data(0x10);
	    lcd_write_index(0x52);lcd_write_data(0x04);
	    lcd_write_index(0x53);lcd_write_data(0x41);
	    lcd_write_index(0x54);lcd_write_data(0x02);
	    lcd_write_index(0x55);lcd_write_data(0x00);
	    lcd_write_index(0x56);lcd_write_data(0x00);
	    lcd_write_index(0x57);lcd_write_data(0x20);
	    lcd_write_index(0x58);lcd_write_data(0x08);
	    lcd_write_index(0x59);lcd_write_data(0x00);
	    lcd_write_index(0x5A);lcd_write_data(0x04);
	    lcd_write_index(0x5B);lcd_write_data(0x10);
	    lcd_write_index(0x5C);lcd_write_data(0xA8);
	    lcd_write_index(0x5D);lcd_write_data(0x00);
	    lcd_write_index(0x5E);lcd_write_data(0x04);
	    lcd_write_index(0x5F);lcd_write_data(0x02);
	    lcd_write_index(0x60);lcd_write_data(0x01);
	    lcd_write_index(0x61);lcd_write_data(0x00);
	    lcd_write_index(0x62);lcd_write_data(0x00);
	    lcd_write_index(0x63);lcd_write_data(0x28);
	    lcd_write_index(0x64);lcd_write_data(0x48);
	    lcd_write_index(0x65);lcd_write_data(0x04);
	    lcd_write_index(0x66);lcd_write_data(0x0A);
	    lcd_write_index(0x67);lcd_write_data(0x48);
	    lcd_write_index(0x68);lcd_write_data(0x44);
	    lcd_write_index(0x69);lcd_write_data(0x02);
	    lcd_write_index(0x6A);lcd_write_data(0x12);
	    lcd_write_index(0x6B);lcd_write_data(0x80);
	    lcd_write_index(0x6C);lcd_write_data(0x48);
	    lcd_write_index(0x6D);lcd_write_data(0xA0);
	    lcd_write_index(0x6E);lcd_write_data(0x08);
	    lcd_write_index(0x6F);lcd_write_data(0x0C);
	    lcd_write_index(0x70);lcd_write_data(0x05);
	    lcd_write_index(0x71);lcd_write_data(0x92);
	    lcd_write_index(0x72);lcd_write_data(0x00);
	    lcd_write_index(0x73);lcd_write_data(0x10);
	    lcd_write_index(0x74);lcd_write_data(0x20);
	    lcd_write_index(0x75);lcd_write_data(0x00);
	    lcd_write_index(0x76);lcd_write_data(0x00);
	    lcd_write_index(0x77);lcd_write_data(0xE4);
	    lcd_write_index(0x78);lcd_write_data(0x00);
	    lcd_write_index(0x79);lcd_write_data(0x04);
	    lcd_write_index(0x7A);lcd_write_data(0x0A);
	    lcd_write_index(0x7B);lcd_write_data(0x01);
	    lcd_write_index(0x7C);lcd_write_data(0x08);
	    lcd_write_index(0x7D);lcd_write_data(0x00);
	    lcd_write_index(0x7E);lcd_write_data(0x24);
	    lcd_write_index(0x7F);lcd_write_data(0x4C);
	    lcd_write_index(0x80);lcd_write_data(0x04);
	    lcd_write_index(0x81);lcd_write_data(0x02);
	    lcd_write_index(0x82);lcd_write_data(0x02);
	    lcd_write_index(0x83);lcd_write_data(0x49);
	    lcd_write_index(0x84);lcd_write_data(0x02);
	    lcd_write_index(0x85);lcd_write_data(0x18);
	    lcd_write_index(0x86);lcd_write_data(0x18);
	    lcd_write_index(0x87);lcd_write_data(0x60);
	    lcd_write_index(0x88);lcd_write_data(0x88);
	    lcd_write_index(0x89);lcd_write_data(0x02);
	    lcd_write_index(0x8A);lcd_write_data(0x01);
	    lcd_write_index(0x8B);lcd_write_data(0x04);
	    lcd_write_index(0x8C);lcd_write_data(0x18);
	    lcd_write_index(0x8D);lcd_write_data(0x98);
	    lcd_write_index(0x8E);lcd_write_data(0x18);
	    lcd_write_index(0x8F);lcd_write_data(0x88);
	    lcd_write_index(0x90);lcd_write_data(0x00);
	    lcd_write_index(0x91);lcd_write_data(0x10);
	    lcd_write_index(0x92);lcd_write_data(0x20);
	    lcd_write_index(0x93);lcd_write_data(0x00);
	    lcd_write_index(0x94);lcd_write_data(0x04);
	    lcd_write_index(0x95);lcd_write_data(0x02);
	    lcd_write_index(0x96);lcd_write_data(0x00);
	    lcd_write_index(0x97);lcd_write_data(0x00);
	    lcd_write_index(0x98);lcd_write_data(0x10);
	    lcd_write_index(0x99);lcd_write_data(0x20);
	    lcd_write_index(0x9A);lcd_write_data(0x00);
	    lcd_write_index(0x9B);lcd_write_data(0x04);
	    lcd_write_index(0x9C);lcd_write_data(0x0A);
	    lcd_write_index(0x37);lcd_write_data(0x03);
            
}


void dev_oled_init(void)
{

    LCD_RST_CLR;
    //LCD_CS_CLR;
    //EN_2V8_CLR;
    LCD_AVDDEN_SET;
    hwi_delay_ms(11);
    LCD_RST_SET;
    hwi_delay_ms(11);
    LCD_CS_SET;

    hwi_oled_spi_GPIO_Init();
    //hwi_delay_ms(11);
    spi1_init();
    LCD_CS_CLR;

    init_oled();
    lcd_write_index(0xFE);
    lcd_write_data(0x00);
    lcd_write_index(0xC4);
    lcd_write_data(0x80);

    lcd_write_index(0x2A);
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0x59);

    lcd_write_index(0x2B);
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0x01);
    lcd_write_data(0x19);

    lcd_write_index(0x35);
    lcd_write_data(0x00);

    lcd_write_index(0x51);
    lcd_write_data(0xFF);

    lcd_write_index(0x11);
    LCD_CS_SET;
    hwi_delay_ms(120);
    LCD_CS_CLR;
    lcd_write_index(0x29);
    LCD_CS_SET;
    hwi_delay_ms(80);
    }

/*************************************************************************************************
  * @brief    : Reinitialize the oled
  * @param1   : oled brightness(0~255)
  * @param1   : High Bright Mode(0:exit   2:enter)
  * @return   : None
*************************************************************************************************/
void dev_oled_reinit(uint8_t brightness,uint8_t HBM_state)
{
    LCD_CS_CLR;
    hwi_delay_us(10);
    
    lcd_write_index(0xFE);
    while(spi_i2s_flag_get(SPI1,SPI_STAT_TRANS));
    lcd_write_data(0x00);
    while(spi_i2s_flag_get(SPI1,SPI_STAT_TRANS));
    
    lcd_write_index(0x51);
    while(spi_i2s_flag_get(SPI1,SPI_STAT_TRANS));
    lcd_write_data(brightness);
    while(spi_i2s_flag_get(SPI1,SPI_STAT_TRANS));
    
    lcd_write_index(0x66);
    while(spi_i2s_flag_get(SPI1,SPI_STAT_TRANS));
    lcd_write_data(HBM_state);
    while(spi_i2s_flag_get(SPI1,SPI_STAT_TRANS));

    LCD_CS_SET;
    hwi_delay_us(10);
}

void dev_oled_power_off(void)
{
    LCD_CS_CLR;
    lcd_write_index(0x28);
    lcd_write_data(0x00);
    lcd_write_index(0x10);
    lcd_write_data(0x00);
    //LCD_CS_SET;
    vTaskDelay(110);
    LCD_RST_CLR;

    LCD_RS_CLR;
    hwi_oled_spi_GPIO_DeInit();
    vTaskDelay(10);
    LCD_AVDDEN_CLR;
    vTaskDelay(15);
    //EN_2V8_SET;
}

/*!
    \brief      set lcd display region
    \param[in]  x_start: the x position of the start point
    \param[in]  y_start: the y position of the start point
    \param[in]  x_end: the x position of the end point
    \param[in]  y_end: the y position of the end point
*/
void dev_oled_set_region(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
    LCD_CS_CLR;

    /* write the register address 0x2A*/
    lcd_write_index(0x2A);
    while(spi_i2s_flag_get(SPI1,SPI_STAT_TRANS));

    lcd_write_data_16bit(x_start >> 8,x_start);
    while(spi_i2s_flag_get(SPI1,SPI_STAT_TRANS));

    lcd_write_data_16bit(x_end >> 8,x_end);
    while(spi_i2s_flag_get(SPI1,SPI_STAT_TRANS));

    /* write the register address 0x2B*/
    lcd_write_index(0x2B);
    while(spi_i2s_flag_get(SPI1,SPI_STAT_TRANS));

    lcd_write_data_16bit(y_start >> 8,y_start);
    while(spi_i2s_flag_get(SPI1,SPI_STAT_TRANS));

    lcd_write_data_16bit(y_end >> 8,y_end);
    while(spi_i2s_flag_get(SPI1,SPI_STAT_TRANS));

    /* write the register address 0x2C*/
    //lcd_write_index(0x2C);
    LCD_CS_SET;
}

/*!
    \brief      set the start display point of lcd
    \param[in]  x: the x position of the start point
    \param[in]  y: the y position of the start point
*/
void dev_oled_set_xy(uint16_t x, uint16_t y)
{
    /* write the register address 0x2A*/
    lcd_write_index(0x2A);
    lcd_write_data_16bit(x >> 8,x);

    /* write the register address 0x2B*/
    lcd_write_index(0x2B);
    lcd_write_data_16bit(y >> 8,y);

    /* write the register address 0x2C*/
    lcd_write_index(0x2C);
}


/*!
    \brief      clear the lcd
    \param[in]  data: composed of the same RGB(only one color in picture)
    \param[out] none
    \retval     none
*/

void lcd_clear(const unsigned char* data)
{
    unsigned int i,m;
    /* set lcd display region */
    dev_oled_set_region(0,0,89,281);
    LCD_CS_CLR;
    lcd_write_index(0x2C);
    while(spi_i2s_flag_get(SPI1,SPI_STAT_TRANS));
    LCD_RS_SET;

    for(i=0;i<2;i++)
    {
        dma_deinit(DMA0, DMA_CH4);
        dma_init_struct.memory0_addr = (uint32_t)&data[0];
        dma_init_struct.number = 38070;
        dma_single_data_mode_init(DMA0, DMA_CH4, &dma_init_struct);
        dma_channel_enable(DMA0, DMA_CH4);
        /* SPI DMA enable */
        spi_dma_enable(SPI1, SPI_DMA_TRANSMIT);
        while(!dma_flag_get(DMA0, DMA_CH4, DMA_FLAG_FTF));
        dma_flag_clear(DMA0, DMA_CH4, DMA_FLAG_FTF);
    }
    LCD_CS_SET;
}

void dis_pic(const unsigned char* data,uint32_t size, uint32_t begin)
{
    unsigned int i,m;
    /* set lcd display region */    
    LCD_CS_CLR;
    lcd_write_index(0x2C);
    while(spi_i2s_flag_get(SPI1,SPI_STAT_TRANS));

    LCD_RS_SET;
    for(i=0;i<1;i++)
    {
        dma_deinit(DMA0, DMA_CH4);
        dev_dma_config();
        dma_init_struct.periph_addr         = (uint32_t)&SPI_DATA(SPI1);

        dma_init_struct.memory0_addr        = (uint32_t)&data[begin];
        dma_init_struct.number              = size;
        dma_init_struct.circular_mode       = DMA_CIRCULAR_MODE_DISABLE;
        dma_single_data_mode_init(DMA0, DMA_CH4, &dma_init_struct);
        dma_channel_enable(DMA0, DMA_CH4);
        /* SPI DMA enable */
        spi_dma_enable(SPI1, SPI_DMA_TRANSMIT);
        while(!dma_flag_get(DMA0, DMA_CH4, DMA_FLAG_FTF));
        while(spi_i2s_flag_get(SPI1,SPI_STAT_TRANS));
        dma_flag_clear(DMA0, DMA_CH4, DMA_FLAG_FTF);
    }
        //while(spi_i2s_flag_get(SPI1,SPI_STAT_TRANS));
    LCD_CS_SET;
}

 unsigned char gImage_white[76140];
void oled_dis_demo(){
//    memset(&gImage_white[0],255,76140);
    //dev_oled_set_region(32,4,95,151);
//    for( int i=0;i<19;i++)
//    {
    //    lcd_set_region(100,4,109,13);
    //    lcd_init();
//        dev_oled_set_region(10,104,19,113);
//        dev_dis_pic(&gImage_red[0],300,0);

//        hwi_delay_ms(10);
    //   lcd_set_region(10,4,19,13);
        //vTaskDelay(1000);
//          lcd_clear(&gImage_logo[0]);
    //    lcd_init();
//        dev_oled_set_region(0,89,0,281);
//        dis_pic(&gImage_white[0],76140,0); 
    //    hwi_delay_ms(100);
    //    dis_pic(&gImage_Branding_Hi_08[0],28416,0);
//        spi_flash_buffer_read(&flash_rx_buffer[0],0x0,34272);//session_bg
//    dev_oled_set_region(72,10,91,29);
//                    spi_flash_buffer_read(&flash_rx_buffer[0],0x1E7E48+1200*5,1200);//gImage_session_bg_withoutcolor
//                    dis_pic(&flash_rx_buffer[0],1200,0);//  %é?
    //   dis_pic(&gImage_white[0],28416,0);  
    //    spi_flash_buffer_read(&flash_rx_buffer[0],28416*i,28416);	
        //memset(flash_rx_buffer,0x00,28416);
    //    dis_pic(&flash_rx_buffer[0],28416,0);
        //memset(flash_rx_buffer,0x00,28416);
        //vTaskDelay(1000);
//        hwi_delay_ms(10);
         //while(1);
//    }
}


