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
// Activity
#include "main.h"
#include "setup_clock.h"
// Objects
#include "start_timer.h"
#include "slider.h"
#include "checkbox.h"
#include "clock.h"
// Drivers
#include "temperature.h"
#include "core.h"
#include "usart.h"
///#include "can.h"
#include "memory.h"
#include "display.h"
#include "sensor.h"
#include "canvas.h"
#include "text.h"

SetupClock setupClock;

//-----------------------------------------------------
SetupClock::SetupClock(void)
{
    core.ClassInit(this,sizeof(this));
}
//-----------------------------------------------------
void SetupClock::viewScreen(void)
{
    setPosition(display.setup.h12);
    
    isChangeCorrection = false;
    weekDayOld = 0xFF;
    hourOld = 0xFF;
    minuteOld = 0xFF;
    calibrationValueOld = 1000;
    canvas.writeFillRect(0,0,320,240,display.COLOR_BACK);
    canvas.loadImageEffect(5,10,SETUP_SAVE_IMAGE,BUTTON_TIMER_STEP,1);  // отображение значка выхода
    
    text.writeStringCenter(160, 10, "Setup Clock", Font_11x18, display.COLOR_TEMP, display.COLOR_BACK);
    
    viewWeek(true);
    viewHour(true);
    viewMinute(true);
    viewCorrection(true); 
}
//-----------------------------------------------------
uint8_t SetupClock::handler(void)
{
    uint8_t result = 0;
    static bool isFast = false;
    
    if (clock.weekDay != weekDayOld){  
        weekDayOld = clock.weekDay;
        viewWeek(false);
    }
    if (clock.hour != hourOld){  
        hourOld = clock.hour;
        viewHour(false);
    }
    if (clock.minute != minuteOld){  
        minuteOld = clock.minute;
        viewMinute(false);
    }
    if (core.calibrationValue != calibrationValueOld){  
        calibrationValueOld = core.calibrationValue;
        viewCorrection(false);
    }
    
    
    if (sensor.status == 1 && sensor.touch == 0 && clock.touch == 0){   // касание сенсорного экрана
        if (slider.touch==0 && 
            sensor.x1>=(startTimer.EXIT_X) && sensor.x1<=(startTimer.EXIT_X+startTimer.EXIT_SIZE) && 
            sensor.y1>=(startTimer.EXIT_Y) && sensor.y1<=(startTimer.EXIT_Y+startTimer.EXIT_SIZE)){  // касание кнопки выхода из настройки времени
            if (sensor.isTouchElement == FALSE){
                sensor.isTouchElement = TRUE;
                result = 1;
                if (isChangeCorrection) writeSetup();
            }
        }
        
        if (slider.touch==0 && 
            sensor.x1>=(WEEKNAME_X-20) && sensor.x1<=(WEEKNAME_X+37+20) && 
            sensor.y1>=(WEEKNAME_Y-50-20) && sensor.y1<=(WEEKNAME_Y-50+37+20)){  // касание кнопки инкремента дня недели
            sensor.touch = 0;
            if (core.getTimer() == 0){
                if (!isFast) core.setTimer(500);
                else core.setTimer(300);
                isFast = true;
                clock.weekDay++;
                if (clock.weekDay > 6) clock.weekDay = 0;
            }
        }
        if (slider.touch==0 && 
            sensor.x1>=(HOUR_X-20) && sensor.x1<=(HOUR_X+37+20) && 
            sensor.y1>=(HOUR_Y-50-20) && sensor.y1<=(HOUR_Y-50+37+20)){  // касание кнопки инкремента часов
            sensor.touch = 0;
            if (core.getTimer() == 0){
                if (!isFast) core.setTimer(500);
                else core.setTimer(200);
                isFast = true;
                clock.hour++;
                if (clock.hour > 23) clock.hour = 0;
            }
        }  
        if (slider.touch==0 && 
            sensor.x1>=(MINUTE_X-20) && sensor.x1<=(MINUTE_X+37+20) && 
            sensor.y1>=(MINUTE_Y-50-20) && sensor.y1<=(MINUTE_Y-50+37+20)){  // касание кнопки инкремента минут
            sensor.touch = 0;
            if (core.getTimer() == 0){
                if (!isFast) core.setTimer(500);
                else core.setTimer(100);
                isFast = true;
                clock.minute++;
                if (clock.minute > 59) clock.minute = 0;
            }
        } 
        if (slider.touch==0 && 
            sensor.x1>=(285-10) && sensor.x1<=(285+30+10) && 
            sensor.y1>=(205-10) && sensor.y1<=(205+30+10)){  // касание кнопки инкремента коррекции
            sensor.touch = 0;
            if (core.getTimer() == 0){
                if (!isFast) core.setTimer(500);
                else core.setTimer(50);
                isFast = true;
                core.calibrationValue++;
                if (core.calibrationValue > 720) core.calibrationValue = -720;
                isChangeCorrection = true;
            }
        } 
            
        if (slider.touch==0 && 
            sensor.x1>=(WEEKNAME_X-20) && sensor.x1<=(WEEKNAME_X+37+20) && 
            sensor.y1>=(WEEKNAME_Y+49-20) && sensor.y1<=(WEEKNAME_Y+49+37+20)){  // касание кнопки декремента дня недели
            sensor.touch = 0;
            if (core.getTimer() == 0){
                if (!isFast) core.setTimer(500);
                else core.setTimer(300);
                isFast = true;
                clock.weekDay--;
                if (clock.weekDay < 0) clock.weekDay = 6;
            }
        }
        if (slider.touch==0 && 
            sensor.x1>=(HOUR_X-20) && sensor.x1<=(HOUR_X+37+20) && 
            sensor.y1>=(HOUR_Y+49-20) && sensor.y1<=(HOUR_Y+49+37+20)){  // касание кнопки декремента часов
            sensor.touch = 0;
            if (core.getTimer() == 0){
                if (!isFast) core.setTimer(500);
                else core.setTimer(200);
                isFast = true;
                clock.hour--;
                if (clock.hour < 0) clock.hour = 23;
            }
        }  
        if (slider.touch==0 && 
            sensor.x1>=(MINUTE_X-20) && sensor.x1<=(MINUTE_X+37+20) && 
            sensor.y1>=(MINUTE_Y+49-20) && sensor.y1<=(MINUTE_Y+49+37+20)){  // касание кнопки декремента минут
            sensor.touch = 0;
            if (core.getTimer() == 0){
                if (!isFast) core.setTimer(500);
                else core.setTimer(100);
                isFast = true;
                clock.minute--;
                if (clock.minute < 0) clock.minute = 59;
            }
        } 
        if (slider.touch==0 && 
            sensor.x1>=(5-10) && sensor.x1<=(5+30+10) && 
            sensor.y1>=(205-10) && sensor.y1<=(205+30+10)){  // касание кнопки декремента коррекции
            sensor.touch = 0;
            if (core.getTimer() == 0){
                if (!isFast) core.setTimer(500);
                else core.setTimer(50);
                isFast = true;
                core.calibrationValue--;
                if (core.calibrationValue < -720) core.calibrationValue = 720;
                isChangeCorrection = true;
            }
        } 
            
        if (display.setup.h12){
            if (slider.touch==0 && 
                sensor.x1>=(AM_PM_X-20) && sensor.x1<=(AM_PM_X+37+20) && 
                sensor.y1>=(AM_PM_Y-20) && sensor.y1<=(AM_PM_Y+37+20)){  // касание кнопки AM-PM
                sensor.touch = 1;
                if (clock.hour < 12) clock.hour += 12;
                else clock.hour -= 12;
            } 
        }
        
        sensor.isTouchElement = TRUE;     
    }
    else if (sensor.status == 0){   // отпускание сенсорного экрана
        isFast = false;
        core.setTimer(0);
        sensor.touch = 0;
        sensor.isTouchElement = FALSE;
    }
    
    
    //clock.viewAnClock(0);
    
    
    return result;
}
//-----------------------------------------------------
void SetupClock::setPosition(bool is12h)
{
    const int STEP_X = 70;
    const int POS_Y = 100;
    uint16_t count;
    
    if (is12h) count = 4;
    else count = 3;
    
    WEEKNAME_X = (320-STEP_X*(count-1)-16*2)/2;
    WEEKNAME_Y = POS_Y;
    HOUR_X = WEEKNAME_X + STEP_X;
    HOUR_Y = POS_Y;
    MINUTE_X = HOUR_X + STEP_X;
    MINUTE_Y = POS_Y;
    AM_PM_X = MINUTE_X + STEP_X;
    AM_PM_Y = POS_Y;
}
//-----------------------------------------------------
void SetupClock::viewWeek(bool isFirst)
{
    int8_t a, b, c;
    char week[7][3] = {
        {'M','o',0},
        {'T','u',0},
        {'W','e',0},
        {'T','h',0},
        {'F','r',0},
        {'S','a',0},
        {'S','u',0},
    };
    
    
    if (isFirst){
        canvas.drawRoundRect(WEEKNAME_X-5, WEEKNAME_Y-50, 42, 126, display.COLOR_BACK, display.LIGHTGREY, COLOR);
        canvas.drawRoundRect(WEEKNAME_X-7, WEEKNAME_Y-12, 46, 46, display.COLOR_BACK, display.LIGHTGREY, COLOR);
    }
    if (clock.weekDay >= 0 && clock.weekDay <= 6){
        a = clock.weekDay - 1;
        b = clock.weekDay;
        c = clock.weekDay + 1;
        
        if (a < 0) a = 6;
        if (c > 6) c = 0;
        
        text.writeString(WEEKNAME_X+10-6, WEEKNAME_Y-40, week[c], Font_11x18, display.COLOR_TEMP, COLOR);
        text.writeString(WEEKNAME_X, WEEKNAME_Y, week[b], Font_16x26, display.COLOR_TEMP, COLOR);
        text.writeString(WEEKNAME_X+10-6, WEEKNAME_Y+45, week[a], Font_11x18, display.COLOR_TEMP, COLOR);
    }
}
//-----------------------------------------------------
void SetupClock::viewHour(bool isFirst)
{
    uint8_t x, n;
    char str[10];
    
    if (isFirst){
        canvas.drawRoundRect(HOUR_X-5, HOUR_Y-50, 42, 126, display.COLOR_BACK, display.LIGHTGREY, COLOR);
        canvas.drawRoundRect(HOUR_X-7, HOUR_Y-12, 46, 46, display.COLOR_BACK, display.LIGHTGREY, COLOR);
        text.writeString(HOUR_X+10, HOUR_Y-40, "+", Font_11x18, display.COLOR_TEMP, COLOR);
        text.writeString(HOUR_X+10, HOUR_Y+45, "-", Font_11x18, display.COLOR_TEMP, COLOR);
        if (display.setup.h12){
            //canvas.drawRoundRect(AM_PM_X-5, AM_PM_Y-50, 42, 126, display.COLOR_BACK, display.LIGHTGREY, COLOR);
            canvas.drawRoundRect(AM_PM_X-7, AM_PM_Y-12, 46, 46, display.COLOR_BACK, display.LIGHTGREY, COLOR);
        }
    }
    for (x=0; x<10; x++) str[x] = 0;
    n = 0;
    if (display.setup.h12){
        //text.writeString(AM_PM_X+10, AM_PM_Y-40, "+", Font_11x18, display.COLOR_TEMP, COLOR);
        if (clock.hour < 12){
            x = clock.hour;
            if (x == 0) x = 12;
            if (x < 10) str[n++] = '0';
            n += text.decToString(&str[n], x);
            text.writeString(HOUR_X, HOUR_Y, str, Font_16x26, display.COLOR_TEMP, COLOR);
            text.writeString(AM_PM_X, AM_PM_Y, "AM", Font_16x26, display.COLOR_TEMP, COLOR);
        }
        else{
            x = clock.hour-12;
            if (x == 0) x = 12;
            if (x < 10) str[n++] = '0';
            n += text.decToString(&str[n], x);
            text.writeString(HOUR_X, HOUR_Y, str, Font_16x26, display.COLOR_TEMP, COLOR);
            text.writeString(AM_PM_X, AM_PM_Y, "PM", Font_16x26, display.COLOR_TEMP, COLOR);
        }
        //text.writeString(AM_PM_X+10, AM_PM_Y+45, "-", Font_11x18, display.COLOR_TEMP, COLOR);
    }
    else{
        if (clock.hour < 10) str[n++] = '0';
        n += text.decToString(&str[n], clock.hour);
        text.writeString(HOUR_X, HOUR_Y, str, Font_16x26, display.COLOR_TEMP, COLOR);
    }
    text.writeString(HOUR_X + 46, HOUR_Y, ":", Font_11x18, display.COLOR_TEMP, display.COLOR_BACK);
}
//-----------------------------------------------------
void SetupClock::viewMinute(bool isFirst)
{
    uint8_t x, n;
    char str[10];
    
    if (isFirst){
        canvas.drawRoundRect(MINUTE_X-5, MINUTE_Y-50, 42, 126, display.COLOR_BACK, display.LIGHTGREY, COLOR);
        canvas.drawRoundRect(MINUTE_X-7, MINUTE_Y-12, 46, 46, display.COLOR_BACK, display.LIGHTGREY, COLOR);
        text.writeString(MINUTE_X+10, MINUTE_Y-40, "+", Font_11x18, display.COLOR_TEMP, COLOR);
        text.writeString(MINUTE_X+10, MINUTE_Y+45, "-", Font_11x18, display.COLOR_TEMP, COLOR);
    }
    for (x=0; x<10; x++) str[x] = 0;
    n = 0;
    if (clock.minute < 10) str[n++] = '0';
    n += text.decToString(&str[n], clock.minute);
    text.writeString(MINUTE_X, MINUTE_Y, str, Font_16x26, display.COLOR_TEMP, COLOR);
}
//-----------------------------------------------------
void SetupClock::viewCorrection(bool isFirst)
{
    uint16_t x, n;
    char str[20];
    int16_t val;
    
    if (isFirst){
        canvas.drawRoundRect(5, 205, 30, 30, display.COLOR_BACK, display.LIGHTGREY, COLOR);
        canvas.drawRoundRect(285, 205, 30, 30, display.COLOR_BACK, display.LIGHTGREY, COLOR);
        text.writeString(15, 210, "-", Font_11x18, display.COLOR_TEMP, COLOR);
        text.writeString(295, 212, "+", Font_11x18, display.COLOR_TEMP, COLOR);
    }
    for (x=0; x<20; x++) str[x] = 0;
    n = 0;
    str[n++] = ' '; str[n++] = 'C'; str[n++] = 'o'; 
    str[n++] = 'r'; str[n++] = 'r'; str[n++] = 'e'; 
    str[n++] = 'c'; str[n++] = 't'; str[n++] = 'i'; 
    str[n++] = 'o'; str[n++] = 'n'; str[n++] = ' '; 
    val = core.calibrationValue;
    if (val < 0){
        val = -val;
        str[n++] = '-';
    }
    n += text.decToString(&str[n], val);
    //str[n++] = ' '; str[n++] = 's'; str[n++] = '/'; str[n++] = 'd'; 
    str[n++] = ' ';
    text.writeStringCenter(160, 212, str, Font_11x18, display.COLOR_TEMP, display.COLOR_BACK);
}
//-----------------------------------------------------
