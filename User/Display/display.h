/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DISPLAY_H
#define __DISPLAY_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Defines ------------------------------------------------------------------*/
#define DISP_power_on	        gpio_bit_set(GPIOB, GPIO_PIN_1)
#define DISP_power_off	        gpio_bit_reset(GPIOB, GPIO_PIN_1)

#define DISP_reset_off	        gpio_bit_set(GPIOB, GPIO_PIN_11)
#define DISP_reset_on	        gpio_bit_reset(GPIOB, GPIO_PIN_11)

#define DISP_led_on	            gpio_bit_set(GPIOA, GPIO_PIN_3)
#define DISP_led_off            gpio_bit_reset(GPIOA, GPIO_PIN_3)

#define DISP_cs_off	            gpio_bit_set(GPIOB, GPIO_PIN_0)
#define DISP_cs_on	            gpio_bit_reset(GPIOB, GPIO_PIN_0)

#define ST7789H2_NOP        0x00
#define ST7789H2_SWRESET    0x01
#define ST7789H2_RDDID      0x04
#define ST7789H2_RDDST      0x09

#define ST7789H2_SLPIN      0x10
#define ST7789H2_SLPOUT     0x11
#define ST7789H2_PTLON      0x12
#define ST7789H2_NORON      0x13

#define ST7789H2_RDMODE     0x0A
#define ST7789H2_RDMADCTL   0x0B
#define ST7789H2_RDPIXFMT   0x0C
#define ST7789H2_RDIMGFMT   0x0D
#define ST7789H2_RDSELFDIAG 0x0F

#define ST7789H2_INVOFF     0x20
#define ST7789H2_INVON      0x21
#define ST7789H2_GAMMASET   0x26
#define ST7789H2_DISPOFF    0x28
#define ST7789H2_DISPON     0x29

#define ST7789H2_CASET      0x2A
#define ST7789H2_PASET      0x2B
#define ST7789H2_RAMWR      0x2C
#define ST7789H2_RAMRD      0x2E

#define ST7789H2_PTLAR      0x30
#define ST7789H2_VSCRDEF    0x33
#define ST7789H2_MADCTL     0x36
#define ST7789H2_VSCRSADD   0x37
#define ST7789H2_IDMOFF     0x38
#define ST7789H2_IDMON      0x39
#define ST7789H2_PIXFMT     0x3A

#define ST7789H2_FRMCTR1    0xB1
#define ST7789H2_FRMCTR2    0xB2
#define ST7789H2_FRMCTR3    0xB3
#define ST7789H2_INVCTR     0xB4
#define ST7789H2_PARCTRL    0xB5
#define ST7789H2_DFUNCTR    0xB6

#define ST7789H2_PWCTR1     0xC0
#define ST7789H2_PWCTR2     0xC1
#define ST7789H2_PWCTR3     0xC2
#define ST7789H2_PWCTR4     0xC3
#define ST7789H2_PWCTR5     0xC4
#define ST7789H2_VMCTR1     0xC5
#define ST7789H2_VMCTR2     0xC7

#define ST7789H2_RDID1      0xDA
#define ST7789H2_RDID2      0xDB
#define ST7789H2_RDID3      0xDC
#define ST7789H2_RDID4      0xDD

#define ST7789H2_GMCTRP1    0xE0
#define ST7789H2_GMCTRN1    0xE1

/* Classes ------------------------------------------------------------------*/
class Display
{
    public:
        Display(void);
        void initialize(void);
        void initPeriph(void);
        void setResolution(uint8_t resolution);
        void setLight (uint16_t percent);
        void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
        void writeCommand(uint8_t cmd);
        void writeData(uint8_t data);
        void writeDataFast(uint8_t data);
        void writeColor(uint32_t color);
        void writeColorFast(uint32_t color);
        void test(uint16_t data);
        void turnOff(void);
        void switchOn(void);
        void setTimer(uint32_t value);
        uint32_t getTimer(void);
        void resetTimer(void);
        
        uint32_t tickTimer;
        volatile uint8_t isDispOk;
        
        struct Setup{
            int brightness, timeout;
            char viewClock;
            char displayHeater, displayEngine, displaySleep;
            char language;
            char resolution;
            char celsius;
            char fahrenheit;
            char h12;
        }; Setup setup;
        
        static const int RESOLUTION_12 = 1;
        static const int RESOLUTION_16 = 2;
        static const int RESOLUTION_18 = 3;
        
        static const int WIDTH = 320;
        static const int HEIGHT = 240;
        static const int CLEAR = 0x123456;
        static const int BLACK = 0x000000;
        static const int NAVY = 0x000080;
        static const int DARKGREEN = 0x008000;
        static const int DARKCYAN = 0x008080;
        static const int MAROON = 0x800000;
        static const int PURPLE = 0x800080;
        static const int OLIVE = 0x808000;
        static const int LIGHTGREY = 0xC0C0C0;
        static const int DARKGREY = 0x808080;
        static const int BLUE = 0x0000FF;
        static const int GREEN = 0x00FF00;
        static const int CYAN = 0x00FFFF;
        static const int RED = 0xFF0000;
        static const int MAGENTA = 0xFF00FF;
        static const int YELLOW = 0xFFFF00;
        static const int WHITE = 0xFFFFFF;
        static const int ORANGE = 0xFFA500;
        static const int GREENYELLOW = 0xADFF2F;
        
        static const int COLOR_BACK = BLACK;
        static const int COLOR_TEXT = 0xD0D0D0;
        static const int COLOR_TEMP = WHITE;
		static const int COLOR_TEMP_RVC = 0xB0FFB0;
        static const int COLOR_ERROR = ORANGE; // RED LIGHTGREY
        static const int COLOR_MESSAGE = LIGHTGREY;
    
    private:
        uint8_t pixelCount;
        uint32_t pixelPrevious;
};
extern Display display;

#endif /* __DISPLAY_H */
