/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __START_TIMER_H
#define __START_TIMER_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Classes ------------------------------------------------------------------*/
class StartTimer
{
    public:
        StartTimer(void);
        void viewButton(void);
        void check(void);
        uint8_t viewScreen(uint8_t mode);
        void viewScreenChange(void);
        void setTimer(uint32_t value);
        uint32_t getTimer(void);
        void resetTimer(void);
    
        bool isChange;
        uint32_t tickTimer;
        bool isCommandRun;
        uint8_t touch, mode;
        char isOn, isUnlimited;
        uint8_t numb;
        int day[3], hour[3], minute[3], on[3];
        char isDay, isHour, isMinute;
    
        static const int TIMER_X = 0;
        static const int TIMER_Y = 0;
        static const int TIMER_SIZE = 60;
        static const int TIMER_STEP = 408;
        static const int EXIT_X = 0;
        static const int EXIT_Y = 0;
        static const int EXIT_SIZE = 60;
        static const int EXIT_STEP = 0;
        static const int NEXT_X = 270;
        static const int NEXT_Y = 0;
        static const int NEXT_SIZE =  60;
        static const int NEXT_STEP = 0;
        
        static const int MO_X = 203;        static const int MO_Y = 14;
        static const int TU_X = 250;        static const int TU_Y = 90;
        static const int WE_X = 230;        static const int WE_Y = 175;
        static const int TH_X = 135;        static const int TH_Y = 210;
        static const int FR_X = 47;         static const int FR_Y = 152;
        static const int SA_X = 45;         static const int SA_Y = 68;
        static const int SU_X = 105;        static const int SU_Y = 7;
        static const int AM_X = 145;        static const int AM_Y = 140;
        
        static const int SELECTOR_STEP = 408;
        static const int COLOR_ON = 0xFFFFFF;
        static const int COLOR_OFF = 0xC0C0C0;
    
    private:
        uint8_t on_old;
};
extern StartTimer startTimer;

/* Info ------------------------------------------------------------------*/


#endif /* __START_TIMER_H */
