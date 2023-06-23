/******************************************************************************
* ООО Теплостар
* Самара
* 
* Программисты: Клюев А.А.
* 
* 08.08.2018
* Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
// Objects
#include "clock.h"
// Drivers
#include "temperature.h"
#include "core.h"
#include "usart.h"
#include "start_timer.h"
#include "memory.h"
#include "display.h"
#include "sensor.h"
#include "canvas.h"
#include "text.h"
#include "unix_time.h"
// Library
#include <math.h>
#include <string.h>

Clock clock;
/* Defines ------------------------------------------------------------------*/
/* Uncomment the corresponding line to select the RTC Clock source */
//#define RTC_CLOCK_SOURCE_LSE   /* LSE used as RTC source clock */
//#define RTC_CLOCK_SOURCE_LSI  // LSI used as RTC source this-> The RTC Clock may varies due to LSI frequency dispersion
#define RTC_CLOCK_SOURCE_HSE
#define BKP_VALUE    0x32F0 

//-----------------------------------------------------
Clock::Clock(void)
{
    this->mode = this->MODE_TIME;
    this->setupClock=1;
    canvas.color0 = 0x000000;
    canvas.color1 = 0x808080;
    canvas.color2 = 0xC0C0C0;
    canvas.color3 = 0xFFFFFF;
    strncat(this->weekName[0], "Monday", 10);
    strncat(this->weekName[1], "Tuesday", 10);
    strncat(this->weekName[2], "Wednesday", 10);
    strncat(this->weekName[3], "Thursday", 10);
    strncat(this->weekName[4], "Friday", 10);
    strncat(this->weekName[5], "Saturday", 10);
    strncat(this->weekName[6], "Sunday", 10);
    this->isWork = TRUE;
    
    sensorClock = clock.SENSOR_HOUR;
}
//-----------------------------------------------------
void Clock::handler(void)
{
    if (this->isWork) this->timeUpdate();                                   // берем текущее время только если не настраиваем часы
}
//-----------------------------------------------------
void Clock::config(void)
{
    // enable PMU and BKPI clocks
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    // allow access to BKP domain
    pmu_backup_write_enable();
    // reset backup domain
    bkp_deinit();
    // select HSE/128 as RTC clock source
    rcu_rtc_clock_config(RCU_RTCSRC_HXTAL_DIV_128);
    // enable RTC Clock */
    rcu_periph_clock_enable(RCU_RTC);
    // wait for RTC registers synchronization
    rtc_register_sync_wait();
    // wait until last write operation on RTC registers has finished
    rtc_lwoff_wait();
    // set RTC prescaler: set RTC period to 1s
    rtc_prescaler_set(125000);
    // wait until last write operation on RTC registers has finished
    rtc_lwoff_wait();
}
//-----------------------------------------------------
void Clock::timeRegulate(void)
{
    // allow access to BKP domain
    rcu_periph_clock_enable(RCU_PMU);
    pmu_backup_write_enable();
    // wait for RTC registers synchronization
    rtc_register_sync_wait();
    rtc_lwoff_wait();
    // wait until last write operation on RTC registers has finished
    rtc_lwoff_wait();
    
    unixTime.wday = weekDay;
    unixTime.hour = hour;
    unixTime.min = minute;
    unixTime.sec = second;
    
    uint32_t timer = unixTime.calToTimer();
    rtc_counter_set(timer);
    bkp_write_data(BKP_DATA_0, BKP_VALUE);
}
//-----------------------------------------------------
bool Clock::isTimeOk(void)
{
    uint16_t data = bkp_read_data(BKP_DATA_0);
    return (data == BKP_VALUE);
}
//-----------------------------------------------------
void Clock::timeUpdate(void)
{
    static uint8_t secondOld;
    static int16_t correct = core.calibrationValue;
    
    uint32_t timer = rtc_counter_get();
    unixTime.timerToCal(timer);
    weekDay = unixTime.wday;
    hour = unixTime.hour;
    minute = unixTime.min;
    second = unixTime.sec;
    
    if (secondOld != second){
        secondOld = second;
        
        if (minute == 30 && second == 30){
            // раз в час корректируем время
            if (hour == 0){
                second += correct;
                correct = core.calibrationValue;
            }
            else{
                int16_t a = core.calibrationValue/24;
                second += a;
                correct -= a;
            }
        }
        
    }
}
//-----------------------------------------------------
void Clock::initialize(void)
{
    uint32_t i;
    float d, f;
    
    f = 3.1428;    
    d = 3.1428/60.0*2;
    for (i=0; i<60; i++){
        this->minuteX[i] = sin(f)*75+160;                // расчет возможных положений ползунка
        this->minuteY[i] = cos(f)*75+120;
        f -= d;
    }
    
    f = 3.1428;    
    d = 3.1428/12.0*2;
    for (i=0; i<13; i++){
        this->hourX[i] = sin(f)*75+160;                // расчет возможных положений ползунка r=69
        this->hourY[i] = cos(f)*75+120;
        f -= d;
    }
    
    if (isTimeOk() == false)
    {  
        /* RTC configuration  */
        this->config();
    }
    else
    {
        timeUpdate();
    }
}
//-----------------------------------------------------
void Clock::viewAnClock(uint8_t mode)
{
    uint8_t a;
    static uint8_t hour_f = 1, minute_f = 1;
    
    if (mode){
        display.turnOff();
        canvas.writeFillRect(0,0,320,240,display.COLOR_BACK);
        canvas.loadImage((320-185)/2+1,(240-185)/2+1,SETUP_CLOCK_IMAGE);
        display.switchOn();
        this->drawMinute(0,2);
        for (a=0; a<12; a++){
            this->drawHour(a, 0);
        }
        this->viewCenterDigClock(0);
        if (hour >= 12) this->isPm = 1;
        else this->isPm = 0;
        this->positionHour = this->hour-(this->isPm*12);
        this->positionMinute = this->minute;
        this->positionSecond = this->second;
        this->resetTimer();
        if (mode == 2){
            this->positionMinuteTemp = 0;
            this->positionHourTemp = 0;
        }
        else {
            this->positionMinuteTemp = this->positionMinute;
            this->positionHourTemp = this->positionHour;
        }
        
    }
    this->sensorCheck();
    this->viewAnClockSecond();
    a = this->viewAnClockMinute();
    if (a){
        minute_f = 1;
        this->setTimer(1500);
    }
    a = this->viewAnClockHour(&isPm, &weekDay, mode);
    if (a){
        hour_f = 1;
        this->setTimer(1500);
    }
    
    if (this->sensorClock == this->SENSOR_HOUR){
        if (this->getTimer() == 0){
            hour_f = !hour_f;
            this->setTimer(500);
        }
        this->viewCenterDigClock(hour_f*2+1);
    }
    else if (this->sensorClock == this->SENSOR_MINUTE){
        if (this->getTimer() == 0){
            this->setTimer(500);
            minute_f = !minute_f;
        }
        this->viewCenterDigClock(2+minute_f);
    }
    else{
        hour_f = 1;
        minute_f = 1;
        this->viewCenterDigClock(3);
    }
    if (mode != 2){
        if (this->positionHourTemp == 12 && this->isPm == 0){
            this->hour = 0;
        }
        else if (this->positionHourTemp == 12 && this->isPm == 1){
            this->hour = 12;
        }
        else{
            this->hour = this->positionHourTemp+(this->isPm*12);
        }
        this->minute = this->positionMinuteTemp;
        if (this->hour >= 24) this->hour = 0;
    }
}
//-----------------------------------------------------
bool Clock::viewAnTimer(uint8_t mode, int8_t *hour, int8_t *minute, uint8_t *isPm)
{
    uint8_t a;
    static uint8_t hour_f = 1, minute_f = 1;
    char isPmTemp;
    static int8_t week_temp = 0;
    bool isChange = false;
    
    isPmTemp = *isPm;
    if (mode){
        if (mode == 2){
            display.turnOff();
            canvas.writeFillRect(0,0,320,240,display.COLOR_BACK);
            canvas.loadImage((320-185)/2+1,(240-185)/2+1,SETUP_CLOCK_IMAGE);
            display.switchOn();
            this->drawMinute(0,2);
            for (a=0; a<12; a++){
                this->drawHour(a, 0);
            }
        }
        this->viewCenterDigClock(0, *hour, *minute);
        if (*hour >= 12) isPmTemp = 1;
        else isPmTemp = 0;
        this->positionHour = *hour-(isPmTemp*12);
        this->positionMinute = *minute;
        this->positionSecond = 0;
        this->resetTimer();
        if (mode == 2){
            this->positionMinuteTemp = this->positionMinute;
            this->positionHourTemp = this->positionHour;
            this->sensorClock = 0;
        }
        else {
            this->positionMinuteTemp = this->positionMinute;
            this->positionHourTemp = this->positionHour;
        }
        
    }
    this->sensorCheck();
    this->viewAnClockSecond();
    a = this->viewAnClockMinute();
    if (a){
        minute_f = 1;
        this->setTimer(1500);
        isChange = true;
    }
    a = this->viewAnClockHour(&isPmTemp, &week_temp, mode);
    if (a){
        hour_f = 1;
        this->setTimer(1500);
        isChange = true;
    }
    
    if (this->sensorClock == this->SENSOR_HOUR){
        if (this->getTimer() == 0){
            hour_f = !hour_f;
            this->setTimer(500);
        }
        this->viewCenterDigClock(hour_f*2+1, *hour, *minute);
    }
    else if (this->sensorClock == this->SENSOR_MINUTE){
        if (this->getTimer() == 0){
            this->setTimer(500);
            minute_f = !minute_f;
        }
        this->viewCenterDigClock(2+minute_f, *hour, *minute);
    }
    else{
        hour_f = 1;
        minute_f = 1;
        this->viewCenterDigClock(3, *hour, *minute);
    }
    if (mode != 2){
        ///if (*hour >= 12) isPmTemp = 1;
        ///else isPmTemp = 0;
        if (this->positionHourTemp == 12 && isPmTemp == 0){
            *hour = 0;
        }
        else if (this->positionHourTemp == 12 && isPmTemp == 1){
            *hour = 12;
        }
        else{
            *hour = this->positionHourTemp+(isPmTemp*12);
        }
        *minute = this->positionMinuteTemp;
        *isPm = isPmTemp;
        if (this->hour >= 24) this->hour = 0;
    }
    
    return isChange;
}
//-----------------------------------------------------
void Clock::viewDigClock(uint8_t h, uint8_t m)
{
    uint8_t x, n, tempHour;
    char str[10];
    char week[7][3] = {
        {'M','O',0},
        {'T','U',0},
        {'W','E',0},
        {'T','H',0},
        {'F','R',0},
        {'S','A',0},
        {'S','U',0},
    };
    
    if (display.setup.h12 == true){
        if (h >= 12){
            tempHour = h - 12;
        }
        else{
            tempHour = h;
        } 
        if (tempHour == 0) tempHour = 12;
    }
    else{
        tempHour = h;
    }
    if (tempHour < 100 && m < 60){
        for (x=0; x<10; x++) str[x] = 0;
        n = text.decToStringFix(str, tempHour, 2);
        str[n] = ':';
        n = text.decToStringFix(&str[n+1], m, 2);
        ///text.writeString(5,240-Font_16x26.height,str,Font_16x26,display.COLOR_TEXT,display.COLOR_BACK);
        text.writeDigit(5,240-Font_16x26.height,str,display.COLOR_TEXT, false);
        if (display.setup.h12 == true){
            if (h < 12){ // !isPm
                text.writeString(90,237-Font_7x10.height,"AM",Font_7x10,display.COLOR_TEXT,display.COLOR_BACK);
            }
            else{
                text.writeString(90,237-Font_7x10.height,"PM",Font_7x10,display.COLOR_TEXT,display.COLOR_BACK);
            }
        }
        text.writeString(5,210-Font_7x10.height,week[weekDay],Font_7x10,display.COLOR_TEXT,display.COLOR_BACK);
    }
    
}
//-----------------------------------------------------
void Clock::viewDigTimer(uint8_t h, uint8_t m)
{
    uint8_t x, n;
    char str[10];
    
    text.writeString(90,237-Font_7x10.height,"  ",Font_7x10,display.COLOR_TEXT,display.COLOR_BACK);
    text.writeString(5,210-Font_7x10.height,"  ",Font_7x10,display.COLOR_TEXT,display.COLOR_BACK);
    if (h < 100 && m < 60){
        for (x=0; x<10; x++) str[x] = 0;
        n = text.decToStringFix(str, h, 2);
        str[n] = ':';
        n = text.decToStringFix(&str[n+1], m, 2);
        ///text.writeString(5,240-Font_16x26.height,str,Font_16x26,display.COLOR_TEXT,display.COLOR_BACK);
        text.writeDigit(5,240-Font_16x26.height,str,display.COLOR_TEXT, false);
    }
    else{
        for (x=0; x<10; x++) str[x] = 0;
        n = text.decToStringFix(str, h, 5);
        text.writeString(5,240-Font_16x26.height,str,Font_16x26,display.COLOR_TEXT,display.COLOR_BACK);
        text.writeDigit(5,240-Font_16x26.height,str,display.COLOR_TEXT, false);
    }
}
//-----------------------------------------------------
void Clock::viewCenterDigClock(char mode)
{
    static uint8_t h, m, mode_old, isPmOld;
    uint8_t x, n, tempHour;
    char str[10];
    
    if (!mode){
        this->weekDayOld = 0xFF;
        h = 0xFF;
        m = 0xFF;
        isPmOld = 0xFF;
        mode_old = 0xFF;
        return;
    }
    
    if (1){
        if (h==this->hour && m==this->minute && mode_old==mode && this->weekDayOld == this->weekDay) return;
        h = this->hour;
        m = this->minute;
        if (display.setup.h12 == true){
            if (this->hour > 12){
                tempHour = this->hour - 12;
            }
            else{
                tempHour = this->hour;
            }
            if (tempHour == 0) tempHour = 12;
            uint8_t isPm = this->hour >= 12;
            if (isPm != isPmOld){
                isPmOld = isPm;
                if (!isPm){
                    canvas.loadImageEffect(startTimer.AM_X,startTimer.AM_Y+6,TIMER_AM_IMAGE,STEP_ZERO,0);  // 
                }
                else{
                    canvas.loadImageEffect(startTimer.AM_X,startTimer.AM_Y+6,TIMER_PM_IMAGE,STEP_ZERO,0);  // 
                }  
            }
        }
        else{
            tempHour = this->hour;
        }
        // отображение часов
        for (x=0; x<10; x++) str[x] = 0;
        n = 0;
        if ((mode&0x02) == 0x02) n = text.decToStringFix(str, tempHour, 2);
        else{
            str[n++] = ' '; str[n++] = ' ';
        }
        str[n++] = ':';
        if ((mode&0x01) == 0x01) n += text.decToStringFix(&str[n], this->minute, 2);
        else{
            str[n++] = ' '; str[n++] = ' ';
        }
        ///text.writeString(this->HOUR_X,this->HOUR_Y,str,Font_16x26,0xD5D5D5,display.COLOR_BACK);
        text.writeDigit(this->HOUR_X,this->HOUR_Y,str,0xD5D5D5, false);
        n=0;
        for (x=0; x<10; x++){
            if (this->weekName[this->weekDay][x] == 0) break;
            n++;
        }
        if (this->weekDayOld != this->weekDay){
            this->weekDayOld = this->weekDay;
            canvas.writeFillRect(125, 135, 70, 10, display.BLACK);
            text.writeStringOver(160-(Font_7x10.width*n/2),135,this->weekName[this->weekDay],Font_7x10,0xD5D5D5,display.COLOR_BACK, 0);
        }
    }
    mode_old = mode;
}
//-----------------------------------------------------
void Clock::viewCenterDigClock(char mode, int8_t hour, int8_t minute)
{
    static uint8_t h, m, mode_old, isPmOld;
    uint8_t x, n, tempHour;
    char str[10];
    
    if (!mode){
        this->weekDayOld = 0xFF;
        h = 0xFF;
        m = 0xFF;
        isPmOld = 0xFF;
        mode_old = 0xFF;
        return;
    }
    
    if (hour < 24 && hour >= 0 && minute < 60 && minute >= 0){
        if (h==hour && m==minute && mode_old==mode && this->weekDayOld == this->weekDay) return;
        h = hour;
        m = minute;
        if (display.setup.h12 == true){
            if (hour > 12){
                tempHour = hour - 12;
            }
            else{
                tempHour = hour;
            } 
            if (tempHour == 0) tempHour = 12;
            uint8_t isPm = hour >= 12;
            if (isPm != isPmOld){
                isPmOld = isPm;
                if (!isPm){
                    canvas.loadImageEffect(startTimer.AM_X,startTimer.AM_Y+6,TIMER_AM_IMAGE,STEP_ZERO,0);  // 
                }
                else{
                    canvas.loadImageEffect(startTimer.AM_X,startTimer.AM_Y+6,TIMER_PM_IMAGE,STEP_ZERO,0);  // 
                }  
            }
        }
        else{
            tempHour = hour;
        }
        // отображение часов
        for (x=0; x<10; x++) str[x] = 0;
        n = 0;
        if ((mode&0x02) == 0x02) n = text.decToStringFix(str, tempHour, 2);
        else{
            str[n++] = ' '; str[n++] = ' ';
        }
        str[n++] = ':';
        if ((mode&0x01) == 0x01) n += text.decToStringFix(&str[n], minute, 2);
        else{
            str[n++] = ' '; str[n++] = ' ';
        }
        ///text.writeString(this->HOUR_X,this->HOUR_Y,str,Font_16x26,0xD5D5D5,display.COLOR_BACK);
        text.writeDigit(this->HOUR_X,this->HOUR_Y,str,0xD5D5D5, false);
    }
    mode_old = mode;
}
//-----------------------------------------------------
uint8_t Clock::viewAnClockSecond(void)
{
    char r, p;
    
    if (this->sensorClock) return 0;
    
    r = (this->positionSecond<15 && this->positionSecondTemp>45);
    p = (this->positionSecond>45 && this->positionSecondTemp<15);
    
    if ((this->positionSecondTemp < this->positionSecond || r ) && !p){               // анимация движения ползунка вправо
            this->positionSecondTempOld = this->positionSecondTemp;
            this->positionSecondTemp += 1;
            if (r){
                if (this->positionSecondTemp >= 60){
                    this->positionSecondTemp -= 60;
                    //clockViewGridClockMinute(0,2);
                }
            }
            else if (this->positionSecondTemp > this->positionSecond) this->positionSecondTemp = this->positionSecond;
            
            //sldViewGrid(slider.positionTemp,BACKGROUND_IMAGE);     // скрытие ползунка
            this->clearDrawSecond(this->positionSecondTempOld, 1);
            this->drawSecond(this->positionSecondTempOld, 0);
            this->drawSecond(this->positionSecondTemp, 1);
            if (!(this->positionSecondTempOld%5)){
                if (this->sensorClock == 0) this->drawHour(this->positionSecondTempOld/5, ((this->positionSecondTempOld/5) == this->positionHourTemp));
                else this->drawHour(this->positionSecondTempOld/5, 0);
            }
            if (this->positionSecondTempOld == this->positionMinuteTemp) this->drawMinute(this->positionMinuteTemp, 1);
    }
    //d = 0;
    if ((this->positionSecondTemp > this->positionSecond || p) && !r){               // анимация движения ползунка влево
            this->positionSecondTempOld = this->positionSecondTemp;
            this->positionSecondTemp -= 1;
            if (p){
                if (this->positionSecondTemp < 0){
                    this->positionSecondTemp += 60;
                    //clockViewGridClockMinute(0,2);
                }
            }
            else if (this->positionSecondTemp < this->positionSecond) this->positionSecondTemp = this->positionSecond;
            
            //sldViewGridInvert(slider.positionTemp,BACKGROUND_IMAGE);     // скрытие ползунка
            this->clearDrawSecond(this->positionSecondTempOld, 1);
            this->drawSecond(this->positionSecondTempOld, 0);
            this->drawSecond(this->positionSecondTemp, 1);
            if (!(this->positionSecondTempOld%5)){
                if (this->sensorClock == 0) this->drawHour(this->positionSecondTempOld/5, ((this->positionSecondTempOld/5) == this->positionHourTemp));
                else this->drawHour(this->positionSecondTempOld/5, 0);
            }
            if (this->positionSecondTempOld == this->positionMinuteTemp) this->drawMinute(this->positionMinuteTemp, 1);
    }
    return (this->positionSecondTemp != this->positionSecond);
}
//-----------------------------------------------------
uint8_t Clock::viewAnClockMinute(void)
{
    char r, p;
    
    if (this->sensorClock == this->SENSOR_HOUR) return 0;
    
    r = (this->positionMinute<15 && this->positionMinuteTemp>45);
    p = (this->positionMinute>45 && this->positionMinuteTemp<15);
    
    if ((this->positionMinuteTemp < this->positionMinute || r ) && !p){               // анимация движения ползунка вправо
            this->positionMinuteTempOld = this->positionMinuteTemp;
            this->positionMinuteTemp += 1;
            if (r){
                if (this->positionMinuteTemp >= 60){
                    this->positionMinuteTemp -= 60;
                    //clockViewGridClockMinute(0,2);
                }
            }
            else if (this->positionMinuteTemp > this->positionMinute) this->positionMinuteTemp = this->positionMinute;
            
            //sldViewGrid(slider.positionTemp,BACKGROUND_IMAGE);     // скрытие ползунка
            this->clearDrawMinute(this->positionMinuteTempOld, 1);
            this->drawMinute(this->positionMinuteTempOld, 0);
            this->drawMinute(this->positionMinuteTemp, 1);
            if (!(this->positionMinuteTempOld%5)){
                if (this->sensorClock == 0) this->drawHour(this->positionMinuteTempOld/5, ((this->positionMinuteTempOld/5) == this->positionHourTemp));
                else this->drawHour(this->positionMinuteTempOld/5, 0);
            }
    }
    //d = 0;
    if ((this->positionMinuteTemp > this->positionMinute || p) && !r){               // анимация движения ползунка влево
            this->positionMinuteTempOld = this->positionMinuteTemp;
            this->positionMinuteTemp -= 1;
            if (p){
                if (this->positionMinuteTemp < 0){
                    this->positionMinuteTemp += 60;
                    //clockViewGridClockMinute(0,2);
                }
            }
            else if (this->positionMinuteTemp < this->positionMinute) this->positionMinuteTemp = this->positionMinute;
            
            //sldViewGridInvert(slider.positionTemp,BACKGROUND_IMAGE);     // скрытие ползунка
            this->clearDrawMinute(this->positionMinuteTempOld, 1);
            this->drawMinute(this->positionMinuteTempOld, 0);
            this->drawMinute(this->positionMinuteTemp, 1);
            if (!(this->positionMinuteTempOld%5)){
                if (this->sensorClock == 0) this->drawHour(this->positionMinuteTempOld/5, ((this->positionMinuteTempOld/5) == this->positionHourTemp));
                else this->drawHour(this->positionMinuteTempOld/5, 0);
            }
    }
    return (this->positionMinuteTemp != this->positionMinute);
}
//-----------------------------------------------------
uint8_t Clock::viewAnClockHour(char *isPm, int8_t *weekDay, uint8_t mode)
{
    char r, p;
    
    if (this->sensorClock == this->SENSOR_MINUTE) return 0;
    
    r = (this->positionHour<3 && this->positionHourTemp>9);
    p = (this->positionHour>9 && this->positionHourTemp<3);
    
    if ((this->positionHourTemp < this->positionHour || r) && !p){               // анимация движения ползунка вправо
            this->positionHourTempOld = this->positionHourTemp;
            this->positionHourTemp += 1;
            if (r){
                if (this->positionHourTemp > 12){
                    this->positionHourTemp -= 12;
                    if (!mode){
                        if (*isPm){
                            (*weekDay)++;
                            if (*weekDay >= 7) *weekDay = 0;
                        }
                        *isPm = 1;
                    }
                }
            }
            else if (this->positionHourTemp > this->positionHour) this->positionHourTemp = this->positionHour;
            
            //sldViewGrid(slider.positionTemp,BACKGROUND_IMAGE);
            this->drawHour(this->positionHourTempOld, 0);
            this->drawHour(this->positionHourTemp, 1);
            if (!(this->positionMinuteTemp%5)){
                if ((this->positionMinuteTemp/5)==this->positionHourTempOld){
                    if (this->sensorClock == 0) this->drawMinute(this->positionMinuteTemp, 1);
                }
            }  
            this->isPmOld = *isPm;
    }
    if ((this->positionHourTemp > this->positionHour || p) && !r){               // анимация движения ползунка влево
            this->positionHourTempOld = this->positionHourTemp;
            this->positionHourTemp -= 1;
            if (p){
                if (this->positionHourTemp <= 0){
                    this->positionHourTemp += 12;
                    if (!mode){
                        if (!*isPm){
                            (*weekDay)--;
                            if (*weekDay < 0) *weekDay = 6; 
                        }
                        *isPm = 0;
                    }
                }
            }
            else if (this->positionHourTemp < this->positionHour) this->positionHourTemp = this->positionHour;
            
            //sldViewGridInvert(slider.positionTemp,BACKGROUND_IMAGE);
            this->drawHour(this->positionHourTempOld, 0);
            this->drawHour(this->positionHourTemp, 1);
            if (!(this->positionMinuteTemp%5)){
                if ((this->positionMinuteTemp/5)==this->positionHourTempOld){
                    if (this->sensorClock == 0) this->drawMinute(this->positionMinuteTemp, 1);
                }
            }
            this->isPmOld = *isPm;
    }
    return (this->positionHourTemp != this->positionHour);
}
//-----------------------------------------------------
void Clock::viewGridClockMinute(uint16_t pos, uint8_t flag)
{
    // при flag равном 0 - рисуются крайние через одну позиции от ползунка
    // при flag равном 1 - рисуются крайние через одну позиции от ползунка и стираются ближайшие к ползунку
    // при flag равном 2 - рисуются все позиции
    uint32_t color;
    int32_t x, y, a, b, i, k;
    char str[2][7];
    
    y = 0;
    for (i=0; i<60; i++){
        for (x=0; x<6; x++) str[0][x] = 0;
        b = i;
        k = pos-b;
        if (k < 0) k = -k;
        if (k<5 && k>=2 && flag==1) color = display.BLACK;
        else if (k<2 && flag==1) continue;
        else if (k<5 && flag==0) continue;
        else color = display.LIGHTGREY;
        if (k>10 && flag!=2) continue;
        ///*
        if ((b%10)==0 && y!=b){
            y = b;
            a = text.decToString(str[0], b);
            //if (i < (slider.VALUE_LEN/2)) a=VALUE_FONT.width*(a-1);
            //else a = 0;
            text.writeStringOver(this->minuteX[i]-(a*VALUE_FONT.width/2),this->minuteY[i]-VALUE_FONT.height/2,str[0],VALUE_FONT,color,display.COLOR_BACK,0);
        }
        if ((b%5)==0 && y!=b){
            y = b;
            a = text.decToString(str[0], b);
            //if (i < (slider.VALUE_LEN/2)) a=VALUE_SMALL_FONT.width*(a-1);
            //else a = 0;
            text.writeStringOver(this->minuteX[i]-(a*VALUE_SMALL_FONT.width/2),this->minuteY[i]-VALUE_SMALL_FONT.height/2,str[0],VALUE_SMALL_FONT,color,display.COLOR_BACK,0);
        }
        //*/
        /*
        if ((b%10)==0 && y!=b){
            y = b;
            a = text.decToString(str[0], b);
            if (i < (slider.VALUE_LEN/2)) a=VALUE_FONT.width*(a-1);
            else a = 0;
            text.writeStringOver(slider.valueX[i]-VALUE_FONT.width/2-a,slider.valueY[i]-VALUE_FONT.height/2,str[0],VALUE_FONT,color,display.COLOR_BACK,0);
        }
        if ((b%5)==0 && y!=b){
            y = b;
            a = text.decToString(str[0], b);
            if (i < (slider.VALUE_LEN/2)) a=VALUE_SMALL_FONT.width*(a-1);
            else a = 0;
            text.writeStringOver(slider.valueX[i]-VALUE_SMALL_FONT.width/2-a,slider.valueY[i]-VALUE_SMALL_FONT.height/2,str[0],VALUE_SMALL_FONT,color,display.COLOR_BACK,0);
        }
        */
    }
}
//-----------------------------------------------------
void Clock::viewGridClockHour(uint16_t pos, uint8_t flag)
{
    // при flag равном 0 - стирается позиция ползунка
    // при flag равном 1 - рисуется позиция ползунка
    // при flag равном 2 - рисуются все позиции
    uint32_t color;
    int32_t x, y, a, b, i, k;
    char str[2][7];
    
    y = (-1);
    for (i=1; i<13; i++){
        for (x=0; x<6; x++) str[0][x] = 0;
        b = i;
        k = pos-b;
        if (k < 0) k = -k;
        if (k >= 12) k -= 12;
        if (k==0 && flag==0) color = display.BLACK;
        else if (k>0 && flag==1) continue;
        else if (k>0 && flag==0) continue;
        else color = display.LIGHTGREY;
        if (k>10 && flag!=2) continue;
        ///*
        if (this->isPm) b += 12;
        if (b == 24) b = 0;
        if ((b%3)==0 && y!=b){
            y = b;
            a = text.decToString(str[0], b);
            //if (i < (slider.VALUE_LEN/2)) a=VALUE_FONT.width*(a-1);
            //else a = 0;
            text.writeStringOver(this->hourX[i]-(a*VALUE_FONT.width/2),this->hourY[i]-VALUE_FONT.height/2,str[0],VALUE_FONT,color,display.COLOR_BACK,0);
        }
        if ((b%1)==0 && y!=b){
            y = b;
            a = text.decToString(str[0], b);
            //if (i < (slider.VALUE_LEN/2)) a=VALUE_SMALL_FONT.width*(a-1);
            //else a = 0;
            text.writeStringOver(this->hourX[i]-(a*VALUE_SMALL_FONT.width/2),this->hourY[i]-VALUE_SMALL_FONT.height/2,str[0],VALUE_SMALL_FONT,color,display.COLOR_BACK,0);
        }
        //*/
        /*
        if ((b%10)==0 && y!=b){
            y = b;
            a = text.decToString(str[0], b);
            if (i < (slider.VALUE_LEN/2)) a=VALUE_FONT.width*(a-1);
            else a = 0;
            text.writeStringOver(slider.valueX[i]-VALUE_FONT.width/2-a,slider.valueY[i]-VALUE_FONT.height/2,str[0],VALUE_FONT,color,display.COLOR_BACK,0);
        }
        if ((b%5)==0 && y!=b){
            y = b;
            a = text.decToString(str[0], b);
            if (i < (slider.VALUE_LEN/2)) a=VALUE_SMALL_FONT.width*(a-1);
            else a = 0;
            text.writeStringOver(slider.valueX[i]-VALUE_SMALL_FONT.width/2-a,slider.valueY[i]-VALUE_SMALL_FONT.height/2,str[0],VALUE_SMALL_FONT,color,display.COLOR_BACK,0);
        }
        */
    }
}
//-----------------------------------------------------
void Clock::clearGridClockHour(void)
{
    int32_t x, y, a, b, i;
    char str[2][7];
    
    y = (-1);
    for (i=1; i<13; i++){
        for (x=0; x<6; x++) str[0][x] = 0;
        b = i;
        ///*
        if (this->isPm) b += 12;
        if (b == 24) b = 0;
        if ((b%3)==0 && y!=b){
            y = b;
            a = text.decToString(str[0], b);
            //if (i < (slider.VALUE_LEN/2)) a=VALUE_FONT.width*(a-1);
            //else a = 0;
            text.writeStringOver(this->hourX[i]-(a*VALUE_FONT.width/2),this->hourY[i]-VALUE_FONT.height/2,str[0],VALUE_FONT,display.BLACK,display.COLOR_BACK,0);
        }
        if ((b%1)==0 && y!=b){
            y = b;
            a = text.decToString(str[0], b);
            //if (i < (slider.VALUE_LEN/2)) a=VALUE_SMALL_FONT.width*(a-1);
            //else a = 0;
            text.writeStringOver(this->hourX[i]-(a*VALUE_SMALL_FONT.width/2),this->hourY[i]-VALUE_SMALL_FONT.height/2,str[0],VALUE_SMALL_FONT,display.BLACK,display.COLOR_BACK,0);
        }
        //*/
        /*
        if ((b%10)==0 && y!=b){
            y = b;
            a = text.decToString(str[0], b);
            if (i < (slider.VALUE_LEN/2)) a=VALUE_FONT.width*(a-1);
            else a = 0;
            text.writeStringOver(slider.valueX[i]-VALUE_FONT.width/2-a,slider.valueY[i]-VALUE_FONT.height/2,str[0],VALUE_FONT,color,display.COLOR_BACK,0);
        }
        if ((b%5)==0 && y!=b){
            y = b;
            a = text.decToString(str[0], b);
            if (i < (slider.VALUE_LEN/2)) a=VALUE_SMALL_FONT.width*(a-1);
            else a = 0;
            text.writeStringOver(slider.valueX[i]-VALUE_SMALL_FONT.width/2-a,slider.valueY[i]-VALUE_SMALL_FONT.height/2,str[0],VALUE_SMALL_FONT,color,display.COLOR_BACK,0);
        }
        */
    }
}
//-----------------------------------------------------
void Clock::setColorGridClock(char mode)
{
    if (mode == 0){
        canvas.color0 = display.BLACK;
        canvas.color1 = display.DARKGREY;
        canvas.color2 = display.LIGHTGREY;
        canvas.color3 = display.WHITE;
    }
    else if (mode == 2){
        canvas.color0 = display.BLACK;
        canvas.color1 = 0x00363F;
        canvas.color2 = 0x006C7F;
        canvas.color3 = 0x00D9FF;
    }
    else if (mode == 1){
        canvas.color0 = display.BLACK;
        canvas.color1 = 0x3F0000;
        canvas.color2 = 0x7F0000;
        canvas.color3 = 0xFF0000;
    }
    
}
//-----------------------------------------------------
uint8_t Clock::drawHour(uint16_t pos, char flag)
{
    const double PI = 3.14159265359;
    uint8_t i;
    uint16_t x[12], y[12];
    char c1, c2, size;
    
    if (pos < 13){
        if (flag == 0){ // серая полоска циферблата
            c1 = 0; // крайние точки прозрачные
            c2 = 2; // внутренние точки сплошные
            size = 0;   // точки тонкие
        }
        else if (flag == 1){ // белая полоска циферблата
            c1 = 1; // крайние точки прозрачные
            c2 = 3; // внутренние точки сплошные
            size = 0;   // точки тонкие
            this->setColorGridClock(1);
        }
        else if (flag == 4){ // черная полоска циферблата
            c1 = 4; // крайние точки черные
            c2 = 4; // внутренние точки черные
            size = 0;   // точки тонкие
        }
        
        float f = (6-pos)/6.0*PI;
        for (i=0; i<12; i++){
            x[i] = sin(f)*(69+i)+160;
            y[i] = cos(f)*(69+i)+120;
        }
        
        canvas.writeDot(x[0], y[0], c1, size);
        canvas.writeDot(x[11], y[11], c1, size);
        for (i=1; i<11; i++){
            canvas.writeDot(x[i], y[i], c2, size);
        }
        
        this->setColorGridClock(0);
    }
    return 0x00;
}
//-----------------------------------------------------
void Clock::drawMinute(uint16_t pos, uint8_t flag)
{
    uint8_t i;
    
    if (pos < 60){
        if (flag < 2){
            if (flag == 1) this->setColorGridClock(1);
            canvas.writeDot(this->minuteX[pos], this->minuteY[pos], flag, flag);
            this->setColorGridClock(0);
        }
        else if (flag == 4){
            canvas.writeDot(this->minuteX[pos], this->minuteY[pos], flag, 1);
        }
        else if (flag == 2){   // выводим все минуты циферблата
            for (i=0; i<60; i++){
                canvas.writeDot(this->minuteX[i], this->minuteY[i], 0, 0);
            }
        }
    }
}
//-----------------------------------------------------
void Clock::clearDrawMinute(uint16_t pos, uint8_t flag)
{
    uint8_t i;
    
    if (pos < 60){
        if (flag < 2){
            canvas.clearDot(this->minuteX[pos], this->minuteY[pos], flag, flag);
        }
        else{   // выводим все минуты циферблата
            for (i=0; i<60; i++){
                canvas.clearDot(this->minuteX[i], this->minuteY[i], 0, 0);
            }
        }
    }
}
//-----------------------------------------------------
void Clock::drawSecond(uint16_t pos, uint8_t flag)
{
    uint8_t i;
    
    if (flag < 2){
        //if (flag == 1) clockSetColorGridClock(2);
        canvas.writeDot(this->minuteX[pos], this->minuteY[pos], flag, flag);
        //clockSetColorGridClock(0);
    }
    else if (flag == 4){
        canvas.writeDot(this->minuteX[pos], this->minuteY[pos], flag, 1);
    }
    else if (flag == 2){   // выводим все минуты циферблата
        for (i=0; i<60; i++){
            canvas.writeDot(this->minuteX[i], this->minuteY[i], 0, 0);
        }
    }
}
//-----------------------------------------------------
void Clock::clearDrawSecond(uint16_t pos, uint8_t flag)
{
    uint8_t i;
    
    if (flag < 2){
        canvas.clearDot(this->minuteX[pos], this->minuteY[pos], flag, flag);
    }
    else{   // выводим все минуты циферблата
        for (i=0; i<60; i++){
            canvas.clearDot(this->minuteX[i], this->minuteY[i], 0, 0);
        }
    }
}
//-----------------------------------------------------
void Clock::sensorCheck(void)
{
    uint32_t i;
    int32_t x, y, a, b, c;
    static bool help_touch = FALSE;
    static char help_count = 0;
    static uint32_t help_timer;
    static uint8_t isTouchElement = FALSE;
    
    if (sensor.status == 1 && (sensor.touch == 0 || clock.touch != 0)){   // касание сенсорного экрана
        //sensor.touch = 1;
        
        if (1){
            if (clock.touch==0 && 
                sensor.x1>(clock.HOUR_X+22) && sensor.x1<(clock.MINUTE_X+clock.MINUTE_X_SIZE-22) && 
                sensor.y1>(clock.HOUR_Y) && sensor.y1<(clock.MINUTE_Y+clock.MINUTE_Y_SIZE)){  // касание часов
                sensor.touch = 1;
                if (isTouchElement == FALSE){
                    isTouchElement = TRUE;
                    if (clock.sensorClock == clock.SENSOR_MINUTE){
                        clock.sensorClock = clock.SENSOR_HOUR;
                        clock.clearDrawMinute(clock.positionMinuteTemp, 1);
                        clock.drawMinute(clock.positionMinuteTemp, 0);
                        clock.drawHour(clock.positionHourTemp, 1);
                    }
                    else{
                        clock.sensorClock = clock.SENSOR_MINUTE;
                        clock.drawHour(clock.positionHourTemp, 0);
                        clock.drawMinute(clock.positionMinuteTemp, 1);
                    }
                }
            }
            else if ((sensor.x1>(clock.minuteX[clock.positionMinuteTemp]-clock.SLIDER_RADIUS) && sensor.x1<(clock.minuteX[clock.positionMinuteTemp]+clock.SLIDER_RADIUS) && 
                sensor.y1>(clock.minuteY[clock.positionMinuteTemp]-clock.SLIDER_RADIUS) && sensor.y1<(clock.minuteY[clock.positionMinuteTemp]+clock.SLIDER_RADIUS) && clock.touch==0) ||
                (clock.sensorClock==clock.SENSOR_MINUTE && clock.touch != 0)){ // касание ползунка
                sensor.touch = 2;
                clock.sensorClock = clock.SENSOR_MINUTE;
                clock.touch = 1;
                a = 1000;
                b = 0;
                for (i=0; i<60; i++){
                    x = clock.minuteX[i] - sensor.x1;
                    if (x < 0) x = -x;
                    y = clock.minuteY[i] - sensor.y1;
                    if (y < 0) y = -y;
                    
                    c = sqrt(std::pow((float)x,2) + std::pow((float)y,2));
                    
                    if (c < a){
                        a = c;
                        b = i;
                    }
                }
                clock.positionMinute = b;
            }
            else if ((sensor.x1>(clock.hourX[clock.positionHourTemp]-clock.SLIDER_RADIUS) && sensor.x1<(clock.hourX[clock.positionHourTemp]+clock.SLIDER_RADIUS) && 
                sensor.y1>(clock.hourY[clock.positionHourTemp]-clock.SLIDER_RADIUS) && sensor.y1<(clock.hourY[clock.positionHourTemp]+clock.SLIDER_RADIUS) && clock.touch==0) ||
                (clock.sensorClock==clock.SENSOR_HOUR && clock.touch != 0)){ // касание ползунка
                sensor.touch = 3;
                clock.sensorClock = clock.SENSOR_HOUR;
                clock.touch = 2;
                a = 1000;
                b = 0;
                for (i=1; i<13; i++){
                    x = clock.hourX[i] - sensor.x1;
                    if (x < 0) x = -x;
                    y = clock.hourY[i] - sensor.y1;
                    if (y < 0) y = -y;
                    
                    c = sqrt(std::pow((float)x,2) + std::pow((float)y,2));
                    
                    if (c < a){
                        a = c;
                        b = i;
                    }
                }
                clock.positionHour = b;
            }
            
            if (sensor.touch == 0){ // касание пустоты(подсказка)
                //sensor.touch = 0;
                if (help_touch == FALSE){
                    help_count = 1;
                    help_touch = TRUE;
                }
            }
        }
        
    }
    else if (sensor.status == 0){   // отпускание сенсорного экрана
        sensor.touch = 0;
        //clock.sensorClock = 0;
        clock.touch = 0;
        core.setTimer(0);
        isTouchElement = FALSE;
        sensor.isTouchElement = FALSE;
    }
    
    if (/*help_touch == TRUE*/sensor.status == 0){
        if ((core.getTick() - help_timer) > 100){
            help_timer = core.getTick();
            if(clock.sensorClock == clock.SENSOR_HOUR){
                if(help_count & 0x01) clock.drawHour(clock.positionHourTemp, 1);
                else clock.drawHour(clock.positionHourTemp, 4);
            }
            else if(clock.sensorClock == clock.SENSOR_MINUTE){
                if(help_count & 0x01) clock.drawMinute(clock.positionMinuteTemp, 1);
                else clock.drawMinute(clock.positionMinuteTemp, 4); 
            }
            if (help_count <= 8) help_count++;
            else help_touch = FALSE;
        }
    }
}
//-----------------------------------------------------
void Clock::setTimer(uint32_t value)
{
    this->tickTimer = core.getTick() + value;
}
//-----------------------------------------------------
uint32_t Clock::getTimer(void)
{
    int32_t i;
    
    i = this->tickTimer - core.getTick();
    if (i < 0) i = 0;
    return i;
}
//-----------------------------------------------------
void Clock::resetTimer(void)
{
    this->tickTimer = core.getTick();
}
//-----------------------------------------------------
