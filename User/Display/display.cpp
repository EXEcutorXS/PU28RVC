/******************************************************************************
* ООО Теплостар
* Самара
* 
* Программисты: Клюев А.А.
* 
* 08.08.2018
* Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "display.h"
#include "core.h"
#include "sleep.h"
#include "main.h"

Display display;
//-----------------------------------------------------
Display::Display(void)
{
    this->setTimer(60 * 1000);
    this->setup.resolution = this->RESOLUTION_16;
}
//-----------------------------------------------------
void Display::initialize(void)
{
    this->initPeriph();
//--------------------------------Reset LCD Driver-----------------------------------
    DISP_power_on;
    core.delayMs(100);
    DISP_reset_off;
    core.delayMs(10);
    DISP_reset_on;
    core.delayMs(100);
    DISP_reset_off;
    core.delayMs(500);
    this->writeCommand(0x11);
    core.delayMs(120);
//--------------------------------Display and color format setting-------------------
    this->writeCommand(0x36);
    this->writeData(0x00);
    this->writeCommand(0x3a);
    this->writeData(0x05);
    
//--------------------------------ST7789S Frame rate setting-------------------------
    this->writeCommand(0xb2);
    this->writeData(0x0c);
    this->writeData(0x0c);
    this->writeData(0x00);
    this->writeData(0x33);
    this->writeData(0x33);
    this->writeCommand(0xb7);
    this->writeData(0x35);
    
//---------------------------------ST7789S Power setting-----------------------------
    this->writeCommand(0xbb);
    this->writeData(0x35);
    this->writeCommand(0xc0);
    this->writeData(0x2c);
    this->writeCommand(0xc2);
    this->writeData(0x01);
    this->writeCommand(0xc3);
    this->writeData(0x13);
    this->writeCommand(0xc4);
    this->writeData(0x20);
    this->writeCommand(0xc6);
    this->writeData(0x0f);
    this->writeCommand(0xca);
    this->writeData(0x0f);
    this->writeCommand(0xc8);
    this->writeData(0x08);
    this->writeCommand(0x55);
    this->writeData(0x90);
    this->writeCommand(0xd0);
    this->writeData(0xa4);
    this->writeData(0xa1);
    
   
    this->writeCommand(ST7789H2_PIXFMT);     // 
    if (this->setup.resolution == this->RESOLUTION_12) this->writeData(0x03);   // 0x03-12bit/pixel, 0x05-16bit/pixel, 0x06-18bit/pixel
    if (this->setup.resolution == this->RESOLUTION_16) this->writeData(0x05);   // 0x03-12bit/pixel, 0x05-16bit/pixel, 0x06-18bit/pixel
    if (this->setup.resolution == this->RESOLUTION_18) this->writeData(0x06);   // 0x03-12bit/pixel, 0x05-16bit/pixel, 0x06-18bit/pixel
    this->writeCommand(ST7789H2_INVON);      // 
    this->writeCommand(ST7789H2_DISPOFF);    // 
    
}
//-----------------------------------------------------
void Display::initPeriph(void)
{
    spi_parameter_struct spi_init_struct;
    timer_parameter_struct timer_initparam;
    timer_oc_parameter_struct timer_ocinitparam;
    
    rcu_periph_clock_enable(RCU_AF);
    // Enable the GPIO Clock
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    // Enable the SPI Clock
    rcu_periph_clock_enable(RCU_SPI0);
    //Включаем тактирование таймера
    rcu_periph_clock_enable(RCU_TIMER1);
    gpio_pin_remap_config (GPIO_TIMER1_PARTIAL_REMAP0, ENABLE);
    // Configure the Led pin
    gpio_init(GPIOA,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_15);
    // Configure the Enable pin
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_MAX, GPIO_PIN_1);
    // Configure the Reset pin
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_MAX, GPIO_PIN_11);
    // Configure the CS pin
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_MAX, GPIO_PIN_0);
    // Configure the SPI1 pin
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_5 | GPIO_PIN_7);
    // Configure the SPI1
    spi_init_struct.trans_mode           = SPI_TRANSMODE_BDTRANSMIT;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_16BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_4;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_init_struct);
    
    spi_enable(SPI0);
    
    // Configure the TIMER1
    timer_deinit(TIMER1);
    timer_initparam.prescaler         = (uint16_t)(120) - 1;
    timer_initparam.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initparam.counterdirection  = TIMER_COUNTER_UP;
    timer_initparam.period            = (uint16_t)100;
    timer_initparam.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initparam.repetitioncounter = 0;
    timer_init(TIMER1,&timer_initparam);
    
    timer_auto_reload_shadow_disable(TIMER1);
    timer_primary_output_config(TIMER1,DISABLE);
    timer_channel_output_shadow_config(TIMER1,TIMER_CH_0,TIMER_OC_SHADOW_DISABLE);
    timer_ocinitparam.outputstate  = TIMER_CCX_ENABLE;
    timer_ocinitparam.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocinitparam.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocinitparam.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocinitparam.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocinitparam.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER1,TIMER_CH_0,&timer_ocinitparam);
    timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_0,0);
    timer_channel_output_mode_config(TIMER1,TIMER_CH_0,TIMER_OC_MODE_PWM0);
    timer_channel_output_fast_config(TIMER1, TIMER_CH_0, TIMER_OC_FAST_DISABLE);
    timer_master_output_trigger_source_select(TIMER1, TIMER_TRI_OUT_SRC_RESET);
    timer_master_slave_mode_config(TIMER1,TIMER_MASTER_SLAVE_MODE_DISABLE);
    
    timer_enable(TIMER1);
}
//-----------------------------------------------------
void Display::setResolution(uint8_t resolution)
{
    this->setup.resolution = resolution;
    this->writeCommand(ST7789H2_PIXFMT);
    if (this->setup.resolution == this->RESOLUTION_12) this->writeData(0x03);
    if (this->setup.resolution == this->RESOLUTION_16) this->writeData(0x05);
    if (this->setup.resolution == this->RESOLUTION_18) this->writeData(0x06);
}
//-----------------------------------------------------
void Display::setLight(uint16_t percent)
{
    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_0, percent);
}
//-----------------------------------------------------
void Display::writeCommand(uint8_t cmd)
{
    if (sleep.isLowPower) sleep.setLowPower(false);
    DISP_cs_on;
    // ждем освобождения SPI
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TBE) == RESET);
    spi_i2s_data_transmit(SPI0, cmd<<7);
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
    DISP_cs_off;
}
//-----------------------------------------------------
void Display::writeData(uint8_t data)
{
    if (sleep.isLowPower) sleep.setLowPower(false);
    DISP_cs_on;
    // ждем освобождения SPI
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TBE) == RESET);
    spi_i2s_data_transmit(SPI0, (0x100 + data)<<7);
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
    DISP_cs_off;
}
//-----------------------------------------------------
void Display::writeDataFast(uint8_t data)
{
    if (sleep.isLowPower) sleep.setLowPower(false);
    // ждем освобождения SPI
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TBE) == RESET);
    spi_i2s_data_transmit(SPI0, (0x100 + data)<<7);
    //while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
}
//-----------------------------------------------------
void Display::writeColor(uint32_t color)
{
        this->writeData(((color>>13)&0x07) | ((color>>16)&0xF8));
        this->writeData(((color>>3)&0x1F) | ((color>>5)&0xE0));
}
//-----------------------------------------------------
void Display::writeColorFast(uint32_t color)
{
    if (sleep.isLowPower) sleep.setLowPower(false);

        writeData(0x100 + (((color>>13)&0x07) | ((color>>16)&0xF8)));
        writeData(0x100 + (((color>>3)&0x1F) | ((color>>5)&0xE0)));

}
//-----------------------------------------------------
void Display::setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint32_t xa, ya;
    
    x = this->WIDTH-x;
    xa = ((uint32_t)y << 16) | (y+h-1);
    ya = ((uint32_t)(x-w) << 16) | (x-1);
    this->writeCommand(ST7789H2_CASET); // Column addr set
    this->writeData((xa>>24)&0xFF);
    this->writeData((xa>>16)&0xFF);
    this->writeData((xa>>8)&0xFF);
    this->writeData(xa&0xFF);
    this->writeCommand(ST7789H2_PASET); // Row addr set
    this->writeData((ya>>24)&0xFF);
    this->writeData((ya>>16)&0xFF);
    this->writeData((ya>>8)&0xFF);
    this->writeData(ya&0xFF);
    this->writeCommand(ST7789H2_RAMWR); // write to RAM
}
//-----------------------------------------------------
void Display::test(uint16_t data)
{
    this->writeCommand(ST7789H2_VSCRDEF);
    // Top fixed area
    this->writeData(0x00);  // H
    this->writeData(35);  // L
    // Scroll area
    this->writeData(250>>8);  // H
    this->writeData(250&0xFF);  // L
    // Bottom fixed area
    this->writeData(0x00);  // H
    this->writeData(35);  // L
    
    this->writeCommand(ST7789H2_VSCRSADD);
    // VSP сдвиг на кол-во пикселей
    this->writeData(data>>8);
    this->writeData(data&0xFF);
}
//-----------------------------------------------------
void Display::turnOff(void)
{
    this->writeCommand(ST7789H2_DISPOFF);
    this->writeCommand(ST7789H2_SLPIN);
}
//-----------------------------------------------------
void Display::switchOn(void)
{
    this->writeCommand(ST7789H2_SLPOUT);
    this->writeCommand(ST7789H2_DISPON);
}

//-----------------------------------------------------
void Display::setTimer(uint32_t value)
{
    this->tickTimer = core.getTick() + value;
}
//-----------------------------------------------------
uint32_t Display::getTimer(void)
{
    int32_t i;
    
    i = this->tickTimer - core.getTick();
    if (i < 0) i = 0;
    return i;
}
//-----------------------------------------------------
void Display::resetTimer(void)
{
    this->tickTimer = core.getTick();
}
//-----------------------------------------------------
