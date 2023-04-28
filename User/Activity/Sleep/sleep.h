/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SLEEP_H
#define __SLEEP_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Classes ------------------------------------------------------------------*/
class Sleep
{
    public:
        Sleep(void);
        uint8_t checkSleepMode(void);
        void viewScreen(void);
        uint8_t handler(void);
        void setLowPower(bool isEnable);
    
        bool isWakeUp;
        bool isViewClock;
        bool isLowPower;
        
    private:
        
        
        
        
        
};
extern Sleep sleep;

/* Info ------------------------------------------------------------------*/

#endif /* __SLEEP_H */
