/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RADIATOR_H
#define __RADIATOR_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"

/* Defines ------------------------------------------------------------------*/
#define MIN_VALUE  60
#define MAX_VALUE  90
#define START_VALUE  80

/* Classes ------------------------------------------------------------------*/
class Radiator
{
    public:
        Radiator(void);
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
extern Radiator radiator;

/* Info ------------------------------------------------------------------*/

#endif /* __RADIATOR_H */
