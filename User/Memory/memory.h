/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MEMORY_H
#define __MEMORY_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Classes ------------------------------------------------------------------*/
class Memory
{
    public:
        uint8_t initializeSpi(void);
        uint8_t initializeQuad(void);
        void initSpi(void);
        void quadOutInit(void);
        void quadInInit(void);
        uint8_t spiSend(uint8_t data);
        void quadSend(uint8_t data);
        uint8_t quadReceiv(void);
        uint8_t readPage(uint8_t* data, uint32_t addr, uint32_t len);
        uint8_t writePage(uint8_t* data, uint32_t addr, uint8_t len);
        uint8_t writeByte(uint8_t data, uint32_t addr, uint32_t len);
        void blockErase(uint32_t addr);
        void chipErase(void);
        uint32_t searchEnd(uint32_t* N);
        uint32_t searchImage(uint32_t N);
        void lastErase(void);
        uint8_t ident(void);
        uint8_t powerDown(void);
        uint8_t powerUp(void);
        int32_t calcCRC(void);
    
    private:
        void reset(void);
        uint8_t writeEnable(void);
        uint8_t write(uint8_t* data, uint8_t len);
        uint8_t read(uint8_t* data, uint8_t len);
        uint8_t readStatus(void);
        uint8_t writeZero(uint32_t addr, uint8_t len);
        void fillPage(void);
    
        uint8_t writePoint;
        uint8_t writeBuf[21];
        uint8_t isQuadMode;
};
extern Memory memory;

/* Defines ------------------------------------------------------------------*/
#define MEM_CS_OFF	            gpio_bit_set(GPIOC, GPIO_PIN_13)
#define MEM_CS_ON	            gpio_bit_reset(GPIOC, GPIO_PIN_13)

#define MEM_RST_OFF	            gpio_bit_set(GPIOB, GPIO_PIN_9)
#define MEM_RST_ON	            gpio_bit_reset(GPIOB, GPIO_PIN_9)

#define MEM_WP_OFF	            gpio_bit_set(GPIOC, GPIO_PIN_15)
#define MEM_WP_ON	            gpio_bit_reset(GPIOC, GPIO_PIN_15)

#define MEM_DI_OFF	            gpio_bit_set(GPIOB, GPIO_PIN_3)
#define MEM_DI_ON	            gpio_bit_reset(GPIOB, GPIO_PIN_3)

#define MEM_CLK_OFF	            gpio_bit_set(GPIOB, GPIO_PIN_4)
#define MEM_CLK_ON	            gpio_bit_reset(GPIOB, GPIO_PIN_4)

#define MEM_DO_READ             gpio_input_bit_get(GPIOC, GPIO_PIN_14)

#define MEM_IO0_OFF	            gpio_bit_set(GPIOB, GPIO_PIN_3)
#define MEM_IO0_ON	            gpio_bit_reset(GPIOB, GPIO_PIN_3)

#define MEM_IO1_OFF	            gpio_bit_set(GPIOC, GPIO_PIN_14)
#define MEM_IO1_ON	            gpio_bit_reset(GPIOC, GPIO_PIN_14)

#define MEM_IO2_OFF	            gpio_bit_set(GPIOC, GPIO_PIN_15)
#define MEM_IO2_ON	            gpio_bit_reset(GPIOC, GPIO_PIN_15)

#define MEM_IO3_OFF	            gpio_bit_set(GPIOB, GPIO_PIN_9)
#define MEM_IO3_ON	            gpio_bit_reset(GPIOB, GPIO_PIN_9)

#define MEM_IO0_READ            gpio_input_bit_get(GPIOB, GPIO_PIN_3)
#define MEM_IO1_READ            gpio_input_bit_get(GPIOC, GPIO_PIN_14)
#define MEM_IO2_READ            gpio_input_bit_get(GPIOC, GPIO_PIN_15)
#define MEM_IO3_READ            gpio_input_bit_get(GPIOB, GPIO_PIN_9)

#define MEM_OK                   0
#define MEM_IDENT_ERROR          1

//Команды можно определить при помощи define или же при помощи перечисления.
typedef enum {
    W25_WRITE_DISABLE = 0x04,
    W25_WRITE_ENABLE = 0x06,
     
    W25_READ_STATUS_1 = 0x05,
    W25_READ_STATUS_2 = 0x35,
    W25_READ_STATUS_3 = 0x15,
     
    W25_WRITE_STATUS_1 = 0x01,
    W25_WRITE_STATUS_2 = 0x31,
    W25_WRITE_STATUS_3 = 0x11,
     
    W25_BLOCK_ERASE = 0xd8,
    W25_CHIP_ERASE = 0xc7, //0x60
     
    W25_GET_DEV_ID = 0x90,
    W25_GET_JEDEC_ID = 0x9f,
     
    W25_ENABLE_RESET = 0x66,
    W25_RESET = 0x99,
     
    W25_PAGE_PROGRAMM = 0x02,
    W25_READ = 0x03,
    
    W25_QUAD_PAGE_PROGRAMM = 0x32,
    
    W25_FAST_READ = 0x0B,
    W25_FAST_READ_QUAD = 0xEB,
    
    W25_ENTER_QPI = 0x38,
    W25_EXIT_QPI = 0xFF,
    
    W25_POWER_DOWN = 0xB9,
    W25_POWER_UP = 0xAB,
    
} W25_Command_t; 

//Я любитель использовать объединения. Поэтому написал тип описывающий этот регистр.
typedef struct {
    uint8_t busy : 1;
    uint8_t write_enable : 1;
    uint8_t block_protect : 3;
    uint8_t top_bot_ptotect : 1;
    uint8_t sector_protect : 1;
    uint8_t status_reg_protect0 : 1;
} STATUS_REG1_STRUCT_t;
 
typedef union {
    uint8_t all;
    STATUS_REG1_STRUCT_t bit;
} Status_reg_1_t;

// Структура принимаемых изображений через последовательный порт:
// 0 - 0xAA
// 1 - 0xCD
// 2 - 0xBA
// 3 - 0x00
// 4 - Width (L)
// 5 - Width (H)
// 6 - Height (L)
// 7 - Height (H)
// 8 - Reserved
// 9 - Reserved
// 10 - Reserved
// 11 - Reserved
// 
// Структура принимаемых команд для изображений через последовательный порт:
// 0 - 0xAA
// 1 - 0xCD
// 2 - 0xBA
// 3 - 0xC0
// 4 - Width (L)
// 5 - Width (H)
// 6 - Height (L)
// 7 - Height (H)
// 8 - Command
// 9 - Reserved
// 10 - Reserved
// 11 - Reserved
// 
// Структура хранимых изображений в памяти:
// 0 - 0xCD
// 1 - 0xBA
// 2 - 0x00
// 3 - Width (L)
// 4 - Width (H)
// 5 - Height (L)
// 6 - Height (H)
// 7 - Reserved
// 8 - Reserved
// 9 - Reserved
// 10 - Reserved
// 11 - Data (R)
// 12 - Data (G)
// 13 - Data (B)
// 14 - ...
// 
// Структура хранимых шрифтов в памяти:
// 0 - 0xCD
// 1 - 0xBA
// 2 - 0x01
// 3 - Language number
// 4 - Length (L)
// 5 - Length (H)
// 6 - Width
// 7 - Height
// 8 - Reserved
// 9 - Reserved
// 10 - Reserved
// 11 - Data1 (L)
// 12 - Data1 (H)
// 13 - Data2 (L)
// 14 - ...
// 
// Структура хранимых строк в памяти:
// 0 - 0xCD
// 1 - 0xBA
// 2 - 0x02
// 3 - Language number
// 4 - Length (L)
// 5 - Length (H)
// 6 - Reserved
// 7 - Reserved
// 8 - Reserved
// 9 - Reserved
// 10 - Reserved
// 11 - Stroke1
// 12 - Stroke1
// 13 - 0x00
// 14 - Stroke2
// 15 - ...
// 






#endif /* __MEMORY_H */
