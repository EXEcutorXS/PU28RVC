/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CLOCK_H
#define __CLOCK_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Classes ------------------------------------------------------------------*/
class Clock{
    public:
        Clock(void);
        void handler(void);
        void timeRegulate(void);
        bool isTimeOk(void);
        void timeUpdate(void);
        void initialize(void);
        void viewAnClock(uint8_t mode);
        bool viewAnTimer(uint8_t mode, int8_t *hour, int8_t *minute, uint8_t *isPm);
        void viewDigClock(uint8_t h, uint8_t m);
        void viewDigTimer(uint8_t h, uint8_t m);
        void viewCenterDigClock(char mode);
        void viewCenterDigClock(char mode, int8_t hour, int8_t minute);
        uint8_t viewAnClockSecond(void);
        uint8_t viewAnClockMinute(void);
        uint8_t viewAnClockHour(char *isPm, int8_t *weekDay, uint8_t mode);
        void viewGridClockMinute(uint16_t pos, uint8_t flag);
        void viewGridClockHour(uint16_t pos, uint8_t flag);
        void clearGridClockHour(void);
        void setColorGridClock(char mode);
        uint8_t drawHour(uint16_t pos, char flag);
        void drawMinute(uint16_t pos, uint8_t flag);
        void clearDrawMinute(uint16_t pos, uint8_t flag);
        void drawSecond(uint16_t pos, uint8_t flag);
        void clearDrawSecond(uint16_t pos, uint8_t flag);
        void sensorCheck(void);
        void setTimer(uint32_t value);
        uint32_t getTimer(void);
        void resetTimer(void);
        
        uint8_t touch;
        uint8_t mode;
        int8_t hour;
        int8_t minute;
        int8_t second;
        int8_t weekDay;
        int8_t weekDayOld;
        uint32_t tickTimer;
        uint16_t minuteX[60], minuteY[60];
        uint16_t hourX[13], hourY[13];
        int16_t positionSecond;
        int16_t positionSecondTemp;
        int16_t positionSecondTempOld;
        int16_t positionMinute;
        int16_t positionMinuteTemp;
        int16_t positionMinuteTempOld;
        int16_t positionHour;
        int16_t positionHourTemp;
        int16_t positionHourTempOld;
        char isPm;
        char isPmOld;
        char sensorClock, sensorClockOld, setupClock;
        char weekName[7][10];
        char isWork;
        
        static const int TIME_X = 0;
        static const int TIME_Y = 210;
        static const int TIME_X_SIZE = 80;
        static const int TIME_Y_SIZE = 30;
        static const int HOUR_X = 120;
        static const int HOUR_Y = 107;
        static const int HOUR_X_SIZE = 40;
        static const int HOUR_Y_SIZE = 32;
        static const int MINUTE_X = 168;
        static const int MINUTE_Y = 107;
        static const int MINUTE_X_SIZE = 40;
        static const int MINUTE_Y_SIZE = 32;
        static const int SLIDER_RADIUS = 32;
        static const int MODE_TIME = 1;
        static const int MODE_WORKTIME = 2;
        static const int SENSOR_MINUTE = 1;
        static const int SENSOR_HOUR = 2;
        
    private:
        void config(void);   

};
extern Clock clock;

/* Info ------------------------------------------------------------------*/


#endif /* __CLOCK_H */
