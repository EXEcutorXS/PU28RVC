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
#include "start_timer.h"
#include "clock.h"
// Drivers
#include "temperature.h"
#include "core.h"
#include "usart.h"
#include "can.h"
#include "memory.h"
#include "display.h"
#include "sensor.h"
#include "canvas.h"
#include "text.h"

StartTimer startTimer;
//-----------------------------------------------------
StartTimer::StartTimer(void)
{
    this->on_old = 0xFF;
    this->isChange = true;
}
//-----------------------------------------------------
void StartTimer::check(void)
{
    uint8_t a, i, f;
    
    if (!this->isOn) return;
    if (this->getTimer() > 0) return;
    
    f = 0;
    for (i=0; i<3; i++){
        if (!this->on[i]) continue;
        a = 1<<clock.weekDay;
        if (!(this->day[i] & a)) continue;
        if (this->hour[i] != clock.hour) continue;
        if (this->minute[i] != clock.minute) continue;
        
        f = 1;
    }
    if (f == 1){
        this->isCommandRun = TRUE;
    }
}
//-----------------------------------------------------
void StartTimer::viewButton(void)
{
    if (this->on_old != this->isOn){
        this->on_old = this->isOn;
        if (this->isOn == 1){
            if (!this->isUnlimited){
                canvas.loadImageEffect(5,5,BUTTON_TIMER_ON_IMAGE,BUTTON_TIMER_STEP,1);                       // отображение значка таймеров
            }
            else {
                canvas.loadImageEffect(5,5,BUTTON_TIMER_RED_IMAGE,BUTTON_TIMER_STEP,1);                       // отображение значка таймеров
            }
        }
        else{
            canvas.loadImageEffect(5,5,BUTTON_TIMER_OFF_IMAGE,BUTTON_TIMER_STEP,1);                       // отображение значка таймеров
        }
    }
}
//-----------------------------------------------------
uint8_t StartTimer::viewScreen(uint8_t mode)
{
    int16_t x, y;
    uint8_t z, n, result = 0;
    static uint8_t h, m;
    char str[8];
    
    char week[7][3] = {
        {'M','O',0},
        {'T','U',0},
        {'W','E',0},
        {'T','H',0},
        {'F','R',0},
        {'S','A',0},
        {'S','U',0},
    };
    
    const int SLIDER1_X = 50;
    const int SLIDER1_Y = 45;
    const int SLIDER1_H = 50;
    const int SLIDER1_W = 200;
    
    const int BUTTON1_X = SLIDER1_X-15;
    const int BUTTON1_Y = SLIDER1_Y+19;
    const int BUTTON1_H = 30;
    const int BUTTON1_W = 30;
    
    const int BUTTON2_X = SLIDER1_X+SLIDER1_W+20;
    const int BUTTON2_Y = SLIDER1_Y+21;
    const int BUTTON2_H = BUTTON1_H;
    const int BUTTON2_W = BUTTON1_W;
    
    const int SLIDER2_X = SLIDER1_X;
    const int SLIDER2_Y = (SLIDER1_Y+50);
    const int SLIDER2_H  = SLIDER1_H;
    const int SLIDER2_W = SLIDER1_W;
    
    const int BUTTON3_X = SLIDER2_X-15;
    const int BUTTON3_Y = SLIDER2_Y+19;
    const int BUTTON3_H = BUTTON1_H;
    const int BUTTON3_W = BUTTON1_W;
    
    const int BUTTON4_X = SLIDER2_X+SLIDER2_W+20;
    const int BUTTON4_Y = SLIDER2_Y+21;
    const int BUTTON4_H = BUTTON3_H;
    const int BUTTON4_W = BUTTON3_W;
    
    const int CHECKBOXS_S = 20;
    
    const int CHECKBOX1_X = 50;
    const int CHECKBOX1_Y = (SLIDER2_Y+65);
    const int CHECKBOX1_H = 40;
    const int CHECKBOX1_W = 150;
    
//---//    const int TIME_X = 210;
//---//    const int TIME_Y = 150;
//---//    const int TIME_ZOOM_X = 1;
//---//    const int TIME_ZOOM_Y = 2;
    
    const int WEEK_Y = 209;
    
    if (sensor.status == 1 && sensor.touch == 0){   // касание сенсорного экрана
        //sensor.touch = 1;
        if (this->touch==0 &&
            sensor.x1>(this->NEXT_X) && sensor.x1<(this->NEXT_X+this->NEXT_SIZE) && 
            sensor.y1>(this->NEXT_Y) && sensor.y1<(this->NEXT_Y+this->NEXT_SIZE)){  // касание кнопки перехода по настройкам
            //sensor.touch = 1;
            if (sensor.isTouchElement == FALSE){
                sensor.isTouchElement = TRUE;
                this->numb++;
                mode = 1;
                if (this->numb > 2) this->numb = 0;
                this->viewScreenChange();
            }
        }
        if (this->touch==0 && 
            sensor.x1>(this->EXIT_X) && sensor.x1<(this->EXIT_X+this->EXIT_SIZE) && 
            sensor.y1>(this->EXIT_Y) && sensor.y1<(this->EXIT_Y+this->EXIT_SIZE)){  // касание кнопки выхода из настроек
            //sensor.touch = 1;
            if (sensor.isTouchElement == FALSE){
                sensor.isTouchElement = TRUE;
                this->isChange = true;
                result = 1;
            }
        }
    }
    else if (sensor.status == 0){   // отпускание сенсорного экрана
        sensor.touch = 0;
        this->touch = 0;
        this->mode = 0;
        core.setTimer(0);
        sensor.isTouchElement = FALSE;
    }
    
    if (mode){
        if (mode == 2){
            display.turnOff();                                                          // отключение дисплея
            canvas.writeFillRect(0,0,320,240,display.COLOR_BACK);                                                // вывод заднего фона
            canvas.loadImageEffect(5,10,SETUP_SAVE_IMAGE,BUTTON_TIMER_STEP,1);                                // отображение значка выхода
            canvas.loadImageEffect(285,5,SETUP_EXIT_IMAGE,BUTTON_SETUP_STEP,1);                              // отображение значка перехода
            display.switchOn();                                                           // включение дисплея
        }
        str[0] = 'T'; str[1] = 'i'; str[2] = 'm'; str[3] = 'e'; str[4] = 'r'; str[5] = ' ';
        text.decToString(&str[6], this->numb+1);
        str[7] = 0;
        text.writeString(160-11*4,10,str,Font_11x18,display.WHITE,display.COLOR_BACK);
        
        text.writeString(BUTTON1_X,BUTTON1_Y,"-",Font_11x18,display.DARKGREY,display.COLOR_BACK);
        text.writeString(BUTTON2_X,BUTTON2_Y,"+",Font_11x18,display.DARKGREY,display.COLOR_BACK);
        text.writeString(BUTTON3_X,BUTTON3_Y,"-",Font_11x18,display.DARKGREY,display.COLOR_BACK);
        text.writeString(BUTTON4_X,BUTTON4_Y,"+",Font_11x18,display.DARKGREY,display.COLOR_BACK);
        
        this->isDay = 1;
        this->isHour = 1;
        this->isMinute = 1;
        this->on[this->numb] |= 2;
        h = this->hour[this->numb]*4.35;
        m = this->minute[this->numb]*1.7;
    }
    x = SLIDER1_X;
    y = SLIDER1_Y;
    z = h;
    if (mode) text.writeString(x,y,"Hours",Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
    if (this->isHour){
        this->isHour = 0;
        canvas.drawSlider(x,y,z,(this->on[this->numb]&0x01));
        for (n=0; n<8; n++) str[n] = 0;
        str[0] = ' ';
        n = this->hour[this->numb] < 10;
        n += text.decToString(&str[n], this->hour[this->numb]);
        str[n++] = ':';
        if (this->minute[this->numb] < 10) str[n++] = '0';
        n += text.decToString(&str[n], this->minute[this->numb]);
//---// text.writeStringZoom(TIME_X,TIME_Y,str,Font_16x26,display.COLOR_TEXT,display.COLOR_BACK,TIME_ZOOM_X,TIME_ZOOM_Y);
    }
    
    y = SLIDER2_Y;
    z = m;
    if (mode) text.writeString(x,y,"Minutes",Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
    if (this->isMinute){
        this->isMinute = 0;
        canvas.drawSlider(x,y,z,(this->on[this->numb]&0x01));
        for (n=0; n<8; n++) str[n] = 0;
        str[0] = ' ';
        n = this->hour[this->numb] < 10;
        n += text.decToString(&str[n], this->hour[this->numb]);
        str[n++] = ':';
        if (this->minute[this->numb] < 10) str[n++] = '0';
        n += text.decToString(&str[n], this->minute[this->numb]);
//---//        text.writeStringZoom(TIME_X,TIME_Y,str,Font_16x26,display.COLOR_TEXT,display.COLOR_BACK,TIME_ZOOM_X,TIME_ZOOM_Y);
    }
    
    if (this->isDay){
        this->isDay = 0;
        for (n=0; n<7; n++){
            if (this->on[this->numb]&0x01){
                if ((this->day[this->numb]>>n)&0x01) text.writeString(n*45+9,WEEK_Y,week[n],Font_16x26,0x0000D9FF,display.COLOR_BACK);
                else text.writeString(n*45+9,WEEK_Y,week[n],Font_16x26,this->COLOR_OFF,display.COLOR_BACK);
            }
            else {
                if ((this->day[this->numb]>>n)&0x01) text.writeString(n*45+9,WEEK_Y,week[n],Font_16x26,this->COLOR_OFF,display.COLOR_BACK);
                else text.writeString(n*45+9,WEEK_Y,week[n],Font_16x26,display.DARKGREY,display.COLOR_BACK);
            }
        }
    }
    
    y = CHECKBOX1_Y;
    if (this->on[this->numb] == 3){
        text.writeString(x+30,y,"Timer on",Font_11x18,this->COLOR_ON,display.COLOR_BACK);
        canvas.loadImageEffect(x,y,SETUP_IMAGE_ON,this->SELECTOR_STEP,0);
        this->on[this->numb] = 1;
    }
    else if (this->on[this->numb] == 2){
        text.writeString(x+30,y,"Timer on",Font_11x18,this->COLOR_OFF,display.COLOR_BACK);
        canvas.loadImageEffect(x,y,SETUP_IMAGE_OFF,this->SELECTOR_STEP,0);
        this->on[this->numb] = 0;
    }
    
    if (sensor.status == 1){   // касание сенсорного экрана
        //sensor.touch = 1;
        
        if ((sensor.x1>=(BUTTON1_X) && sensor.x1<=(BUTTON1_X+BUTTON1_W) && sensor.y1>=(BUTTON1_Y) && sensor.y1<=(BUTTON1_Y+BUTTON1_H) && sensor.touch==0) || sensor.touch==6){ // касание - ползунка 1
            sensor.touch = 6;
            if (core.getTimer() == 0){
                if (this->mode == 0) core.setTimer(500);
                    this->mode = 1;
                    this->hour[this->numb] -= 1;
                    if (this->hour[this->numb] < 0) this->hour[this->numb] = 0;
                    else if (this->hour[this->numb] > 23) this->hour[this->numb] = 23;
                    h = this->hour[this->numb]*4.35;
                    this->isHour = 1;
            }
        }  
        if ((sensor.x1>=(BUTTON2_X) && sensor.x1<=(BUTTON2_X+BUTTON2_W) && sensor.y1>=(BUTTON2_Y) && sensor.y1<=(BUTTON2_Y+BUTTON2_H) && sensor.touch==0) || sensor.touch==7){ // касание + ползунка 1
            sensor.touch = 7;
            if (core.getTimer() == 0){
                if (this->mode == 0) core.setTimer(500);
                    this->mode = 1;
                    this->hour[this->numb] += 1;
                    if (this->hour[this->numb] < 0) this->hour[this->numb] = 0;
                    else if (this->hour[this->numb] > 23) this->hour[this->numb] = 23;
                    h = this->hour[this->numb]*4.35;
                    this->isHour = 1;
            }
        }
        
        if ((sensor.x1>=(BUTTON3_X) && sensor.x1<=(BUTTON3_X+BUTTON3_W) && sensor.y1>=(BUTTON3_Y) && sensor.y1<=(BUTTON3_Y+BUTTON3_H) && sensor.touch==0) || sensor.touch==8){ // касание - ползунка 2
            sensor.touch = 8;
            if (core.getTimer() == 0){
                if (this->mode == 0) core.setTimer(500);
                    this->mode = 1;
                    this->minute[this->numb] -= 1;
                    if (this->minute[this->numb] < 0) this->minute[this->numb] = 0;
                    else if (this->minute[this->numb] > 59) this->minute[this->numb] = 59;
                    m = this->minute[this->numb]*1.7;
                    this->isMinute = 1;
            }
        }  
        if ((sensor.x1>=(BUTTON4_X) && sensor.x1<=(BUTTON4_X+BUTTON4_W) && sensor.y1>=(BUTTON4_Y) && sensor.y1<=(BUTTON4_Y+BUTTON4_H) && sensor.touch==0) || sensor.touch==9){ // касание + ползунка 2
            sensor.touch = 9;
            if (core.getTimer() == 0){
                if (this->mode == 0) core.setTimer(500);
                    this->mode = 1;
                    this->minute[this->numb] += 1;
                    if (this->minute[this->numb] < 0) this->minute[this->numb] = 0;
                    else if (this->minute[this->numb] > 59) this->minute[this->numb] = 59;
                    m = this->minute[this->numb]*1.7;
                    this->isMinute = 1;
            }
        }
        
        if ((sensor.x1>=(SLIDER1_X) && sensor.x1<=(SLIDER1_X+SLIDER1_W) && sensor.y1>=(SLIDER1_Y) && sensor.y1<=(SLIDER1_Y+SLIDER1_H) && sensor.touch==0) || sensor.touch==1){ // касание ползунка 1
            sensor.touch = 1;
            this->hour[this->numb] = (sensor.x1-SLIDER1_X)/8;
            if (this->hour[this->numb] < 0) this->hour[this->numb] = 0;
            else if (this->hour[this->numb] > 23) this->hour[this->numb] = 23;
            h = this->hour[this->numb]*4.35;
            this->isHour = 1;
        }  
        if ((sensor.x1>=(SLIDER2_X) && sensor.x1<=(SLIDER2_X+SLIDER2_W) && sensor.y1>=(SLIDER2_Y) && sensor.y1<=(SLIDER2_Y+SLIDER2_H) && sensor.touch==0) || sensor.touch==2){ // касание ползунка 2
            sensor.touch = 2;
            this->minute[this->numb] = (sensor.x1-SLIDER2_X)/3;
            this->minute[this->numb] = this->minute[this->numb] - this->minute[this->numb]%5;
            if (this->minute[this->numb] < 0) this->minute[this->numb] = 0;
            else if (this->minute[this->numb] > 59) this->minute[this->numb] = 59;
            m = this->minute[this->numb]*1.7;
            this->isMinute = 1;
        }
        if ((sensor.x1>=(CHECKBOX1_X-20) && sensor.x1<=(CHECKBOX1_X+CHECKBOX1_W) && sensor.y1>=(CHECKBOX1_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX1_Y+CHECKBOX1_H-CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 1
            sensor.touch = 4;
            this->on[this->numb] = !this->on[this->numb]+2;
            this->isDay = 1;
            this->isHour = 1;
            this->isMinute = 1;
        }
        
        if (sensor.y1>=(WEEK_Y-20) && sensor.touch==0){ // касание дня недели
            sensor.touch = 5;
            this->isDay = 1;
            n = (sensor.x1-9)/45;
            this->day[this->numb] ^= 1<<n;
        }
    }
    
    return result;
}
//-----------------------------------------------------
void StartTimer::viewScreenChange(void)
{
    canvas.writeFillRect(40,0,240,60,display.COLOR_BACK);
    canvas.writeFillRect(0,40,320,200,display.COLOR_BACK);
}
//-----------------------------------------------------
void StartTimer::setTimer(uint32_t value)
{
    this->tickTimer = core.getTick() + value;
}
//-----------------------------------------------------
uint32_t StartTimer::getTimer(void)
{
    int32_t i;
    
    i = this->tickTimer - core.getTick();
    if (i < 0) i = 0;
    return i;
}
//-----------------------------------------------------
void StartTimer::resetTimer(void)
{
    this->tickTimer = core.getTick();
}
//-----------------------------------------------------
