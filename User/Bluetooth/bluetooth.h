/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"
#include "main.h"

/* Defines ------------------------------------------------------------------*/
#define CLK_UP          gpio_bit_set(GPIOB,  GPIO_PIN_13)
#define CLK_DOWN        gpio_bit_reset(GPIOB,  GPIO_PIN_13)

#define MOSI_UP         gpio_bit_set(GPIOB,  GPIO_PIN_14)
#define MOSI_DOWN       gpio_bit_reset(GPIOB,  GPIO_PIN_14)

#define CS_UP           gpio_bit_set(GPIOA,  GPIO_PIN_8)
#define CS_DOWN         gpio_bit_reset(GPIOA,  GPIO_PIN_8)

#define MISO_STATE      (gpio_input_bit_get(GPIOB, GPIO_PIN_15) == SET)

/* Classes ------------------------------------------------------------------*/
class Bluetooth
{
    public:
        Bluetooth(void);
        void initSpi();
        void initialize(char *dev_name);
        void handler(void);
        void sendArray(uint8_t* buf);
        void readBleData(uint8_t *buf);
        void delay(void);
        void setTimer(uint32_t value);
        uint32_t getTimer(void);
        void resetTimer(void);
        void extiIrqHandler(void);
        
        bool isConnected;
        bool isCommand;
        uint8_t bufRx[32];
    
        uint8_t byte_out[20];
        uint8_t byte_out_point;
        uint8_t byte_in[20];
        uint8_t byte_in_point;
    
        uint32_t tickTimer;
        
        static const int TIMER_SEND = 500;
        
        static const int TYPE_SETUP = 1;
        static const int TYPE_STATUS = 2;
        static const int TYPE_WORK = 3;
        static const int TYPE_FIRMWARE = 4;
        static const int TYPE_ID = 5;
        static const int TYPE_TIMERS = 6;
        static const int TYPE_TIME = 7;
        static const int TYPE_EMPTY = 8;
        static const int TYPE_CONNECTION_STATUS = 9;
        static const int TYPE_KEY = 10;
        
    private:
        uint8_t commandCount;
};
extern Bluetooth bluetooth;

#endif /* __BLUETOOTH_H */
