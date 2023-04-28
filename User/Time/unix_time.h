/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UNIX_TIME_H
#define __UNIX_TIME_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Classes ------------------------------------------------------------------*/
class UnixTime
{
    public:
        void timerToCal (unsigned long timer);
        unsigned long calToTimer (void);
    
        int year;
        char mon;
        char mday;
        char hour;
        char min;
        char sec;
        char wday;
    
    private:
        
};
extern UnixTime unixTime;

#endif /* __UNIX_TIME_H */
