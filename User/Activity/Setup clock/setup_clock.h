/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SETUP_CLOCK_H
#define __SETUP_CLOCK_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Classes ------------------------------------------------------------------*/
class SetupClock{
    public:
        SetupClock(void);
        void viewScreen(void);
        uint8_t handler(void);
        void setPosition(bool is12h);
        void viewWeek(bool isFirst);
        void viewHour(bool isFirst);
        void viewMinute(bool isFirst);
        void viewCorrection(bool isFirst);
    
    private:
        uint8_t weekDayOld,
                hourOld,
                minuteOld;
        int16_t calibrationValueOld;
        bool isChangeCorrection;
        static const uint32_t COLOR = 0x404040;
        uint16_t WEEKNAME_X,
                WEEKNAME_Y,
                HOUR_X,
                HOUR_Y,
                MINUTE_X,
                MINUTE_Y,
                AM_PM_X,
                AM_PM_Y;
        
};
extern SetupClock setupClock;

/* Info ------------------------------------------------------------------*/


#endif /* __SETUP_CLOCK_H */
