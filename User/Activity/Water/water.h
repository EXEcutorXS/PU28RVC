/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WATER_H
#define __WATER_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"

/* Defines ------------------------------------------------------------------*/
#define MIN_VALUE  60
#define MAX_VALUE  90
#define START_VALUE  80

/* Classes ------------------------------------------------------------------*/
class Water
{
    public:
        Water(void);
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
extern Water water;

/* Info ------------------------------------------------------------------*/

#endif /* __WATER_H */
