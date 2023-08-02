/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CORE_H
#define __CORE_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Defines ------------------------------------------------------------------*/
#define MAIN_PROGRAM_START_ADDRESS  0x0800C000
#define NVIC_VectTab_RAM            0x20000000
#define BOOT_FLAG_SHIFT             0xA000

/* Classes ------------------------------------------------------------------*/
class Core
{
    public:
        Core(void);
        void initialize(void);
        void delayUs(uint32_t delay);
        void delayMs(uint32_t delay);
        void protectedFlash(void);
        void remapTable(void);
        void incTick(void);
        uint32_t getTick(void);
        void setTimer(uint32_t value);
        uint32_t getTimer(void);
        void resetTimer(void);
        float celToFar(float cel);
        float farToCel(float far);
        void packFloat(uint8_t *dst, float val);
        uint16_t getDelta(int16_t valA, int16_t valB);
        void paramToProtocol(volatile uint8_t* array, volatile uint8_t group, volatile uint8_t* name, volatile uint8_t type, volatile uint32_t addr, volatile uint8_t* val);
    
        uint32_t searchAddress(uint8_t address);
        void readSection(void);
        void readCondition(void);
        void moveAddress(uint32_t address);
        void handler(void);
        void checkWarning(void);
				
				void ClassInit(void* address,uint32_t size);
    
        uint8_t command;
        uint8_t faultCode;//ToDelete  временная переменная с кодом неисправности
        uint8_t warningCode;//ToDelete  временная переменная с кодом предупреждения о неверных данных циклограммы
        uint16_t flameBreakBorder;//ToDelete  временная переменная с границей срыва пламени
        uint8_t stage, phase;//ToDelete
        
        int16_t calibrationValue;
    
    private:
        uint32_t msTick;
        uint32_t tickTimer;
        
        uint32_t headerAddress;
        uint32_t dataAddress;
        uint32_t timePhase;
};
extern Core core;

/* Info ------------------------------------------------------------------*/


#endif /* __CORE_H */
