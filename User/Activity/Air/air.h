/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AIR_H
#define __AIR_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"

/* Defines ------------------------------------------------------------------*/
#define MIN_VALUE  60
#define MAX_VALUE  90
#define START_VALUE  80

/* Classes ------------------------------------------------------------------*/
class Air
{
    public:
        Air(void);
        void viewScreen(void);
        uint8_t viewHandler(void);
        void viewMode(void);
        void viewTemperature(void);
        uint8_t sensorCheck(void);
        void buttonPress(void);
        void buttonCheck(uint8_t mode);
        
        bool isOn;
        uint8_t value;
        
    private:
        bool isOnOld;
        
};
extern Air air;

/* Info ------------------------------------------------------------------*/

#endif /* __AIR_H */
