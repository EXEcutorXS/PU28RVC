/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WORK_H
#define __WORK_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"

/* Defines ------------------------------------------------------------------*/

/* Classes ------------------------------------------------------------------*/
class Work
{
    public:
        Work(void);
        void viewScreen(void);
        uint8_t viewHandler(void);
        void viewDigClock(void);
        //void viewVoltage(void);
        void viewMode(void);
        //void viewTemperature(void);
        uint8_t sensorCheck(void);
        //void buttonPress(void);
        //void buttonCheck(uint8_t mode);
        //bool checkHeaterSetup(void);
        
        bool isFHeater;
        bool isEHeater;
        bool isChange;
        
    private:
        bool isViewSearch;
        bool isFHeaterOld, isEHeaterOld;
        uint8_t numbTouchElement;
        
};
extern Work work;

/* Info ------------------------------------------------------------------*/

#endif /* __WORK_H */
