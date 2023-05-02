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
        void lockMemory(bool isLock);
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
        uint8_t volatileWriteEnable(void);
        uint8_t writeDisable(void);
        uint8_t globalUnlock(void);
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
    W25_VOLATILE_WRITE_ENABLE = 0x50,
    
    W25_GLOBAL_BLOCK_UNLOCK = 0x98,
     
    W25_READ_STATUS_1 = 0x05,
    W25_READ_STATUS_2 = 0x35,
    W25_READ_STATUS_3 = 0x15,
    
    W25_SECTOL_LOCK = 0x36, 
    W25_SECTOL_UNLOCK = 0x39, 
    
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
    uint8_t top_bot_protect : 1;
    uint8_t sector_protect : 1;
    uint8_t status_reg_protect0 : 1;
} STATUS_REG1_STRUCT_t;
 
typedef union {
    uint8_t all;
    STATUS_REG1_STRUCT_t bit;
} Status_reg_1_t;


typedef struct {
    uint8_t status_reg_protect1 : 1;
    uint8_t quad_enable : 1;
    uint8_t reserved : 1;
    uint8_t security_reg_lock : 3;
    uint8_t complement_protect : 1;
    uint8_t suspend_status : 1;
} STATUS_REG2_STRUCT_t;

typedef union {
    uint8_t all;
    STATUS_REG2_STRUCT_t bit;
} Status_reg_2_t;


typedef struct {
    uint8_t reserved0 : 1;
    uint8_t reserved1 : 1;
    uint8_t write_protect_selection : 1;
    uint8_t reserved3 : 1;
    uint8_t reserved4 : 1;
    uint8_t output_driver_strength : 2;
    uint8_t hold_or_reset_function : 1;
} STATUS_REG3_STRUCT_t;

typedef union {
    uint8_t all;
    STATUS_REG3_STRUCT_t bit;
} Status_reg_3_t;

//-----------------------------------------------------
// Структура хранимых изображений в памяти:
// --------Заголовок----------
// 0 - 0xCD
// 1 - 0xBA
// 2 - 0x00
// 3 - Width (L)            размер данных = Width x Height x 3
// 4 - Width (H)
// 5 - Height (L)
// 6 - Height (H)
// 7 - Reserved
// 8 - Reserved
// 9 - Reserved
// 10 - Reserved
// ---------Данные---------
// 11 - Data (R)
// 12 - Data (G)
// 13 - Data (B)
// 14 - ...

//-----------------------------------------------------
// Структура хранимых шрифтов в памяти:
// --------Заголовок----------
// 0 - 0xCD
// 1 - 0xBA
// 2 - 0x00
// 3 - Width (L)            размер данных = Width x Height x 3
// 4 - Width (H)
// 5 - Height (L)
// 6 - Height (H)
// 7 - Reserved
// 8 - Reserved
// 9 - Reserved
// 10 - Reserved
// ---------Данные---------
// 11 - Data1 (L)
// 12 - Data1 (H)
// 13 - Data2 (L)
// 14 - ...

//-----------------------------------------------------
// Структура хранимых строк в памяти:
// --------Заголовок----------
// 0 - 0xCD
// 1 - 0xBA
// 2 - 0x00
// 3 - Width (L)            размер данных = Width x Height x 3
// 4 - Width (H)
// 5 - Height (L)
// 6 - Height (H)
// 7 - Reserved
// 8 - Reserved
// 9 - Reserved
// 10 - Reserved
// ---------Данные---------
// 11 - Stroke1 (L)
// 12 - Stroke1 (H)
// 13 - Stroke1 (L)
// 14 - Stroke1 (H)
// 15 - 0x00
// 16 - Stroke2 (L)
// 17 - Stroke2 (H)
// 18 - ...

//-----------------------------------------------------
// Структура хранимых прошивок в памяти:
// --------Заголовок----------
// 0 - 0xCD
// 1 - 0xBA
// 2 - 0x01
// 3 - Length (L)           размер данных (всех описаний и прошивки)
// 4 - Length (M)
// 5 - Length (H)
// 6 - Reserved
// 7 - Reserved
// 8 - Reserved
// 9 - Reserved
// 10 - Reserved
// ---------Данные---------
// 11 - Length (L)          размер данных описания (на всех языках)
// 12 - Length (H)
// 13 - Language number     номер языка (0-eng, 1-rus, 2-cz...)
// 14 - Stroke1 (L)         описание
// 15 - Stroke1 (H)
// 16 - Stroke1 (L)
// 17 - Stroke1 (H)
// 18 - ...
// 19 - 0x00
// xx - Language number     номер языка (0-eng, 1-rus, 2-cz...)
// xx - Stroke2 (L)         описание
// xx - Stroke2 (H)
// xx - Stroke2 (L)
// xx - Stroke2 (H)
// xx - ...
// xx - 0x00
// xx - Address (L)         адрес загрузки в память, со смещением от загрузчика
// xx - Address (M) 
// xx - Address (M) 
// xx - Address (H)
// xx - CRC (L)             контрольная сумма
// xx - CRC (H)
// xx - Version 1           версия ПО
// xx - Version 2
// xx - Version 3
// xx - Version 4
// xx - Type device         тип устройства (0-пульт, 1- подогреватель)
// xx - Data                прошивка
// xx - Data
// xx - ...

//-----------------------------------------------------




#endif /* __MEMORY_H */
