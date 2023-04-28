/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __START_TIMERS_H
#define __START_TIMERS_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Classes ------------------------------------------------------------------*/
class StartTimers
{
    public:
        StartTimers(void);
        void viewScreen(void);
        uint8_t handler(void);
        
    private:
        
};
extern StartTimers startTimers;

/* Info ------------------------------------------------------------------*/


#endif /* __START_TIMERS_H */
