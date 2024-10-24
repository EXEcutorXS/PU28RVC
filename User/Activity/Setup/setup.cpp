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
#include "setup.h"
#include "system.h"
// Objects
#include "hcu.h"
#include "start_timer.h"
#include "slider.h"
#include "checkbox.h"
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

Setup setup;
//-----------------------------------------------------
Setup::Setup(void)
{
		core.ClassInit(this,sizeof(this));
    ///readSetup();
    this->numberScreen = 1;
    this->isChange = true;
}
//-----------------------------------------------------
void Setup::viewScreenChange(void)
{
    canvas.writeFillRect(40,0,240,60,display.COLOR_BACK);
    canvas.writeFillRect(0,40,320,200,display.COLOR_BACK);
}
//-----------------------------------------------------
void Setup::viewScreen(void)
{   
    display.turnOff();                                                          // отключение дисплея
    canvas.writeFillRect(0,0,320,240,display.COLOR_BACK);                       // вывод заднего фона
    canvas.loadImageEffect(5,10,SETUP_SAVE_IMAGE,BUTTON_TIMER_STEP,1);          // отображение значка выхода
    canvas.loadImageEffect(285,5,SETUP_EXIT_IMAGE,BUTTON_SETUP_STEP,1);         // отображение значка перехода
    display.switchOn();                                                         // включение дисплея
    
    this->viewPage(this->numberScreen, 1);
}
//-----------------------------------------------------
uint8_t Setup::handler(void)
{
    uint8_t mode = 0, 
            result = 0;
    
    if (sensor.status == 1 && sensor.touch == 0){   // касание сенсорного экрана
        //sensor.touch = 1;
        if (slider.touch==0 && 
            sensor.x1 >= startTimer.NEXT_X && 
            sensor.y1 <= startTimer.NEXT_SIZE){  // касание кнопки перехода по настройкам
            //sensor.touch = 0;
            if (sensor.isTouchElement == FALSE){
                sensor.isTouchElement = TRUE;
                this->numberScreen++;
                mode = 1;
                if (this->numberScreen > this->maxNumberScreen) this->numberScreen = 1;
                this->viewScreenChange();
            }
        }
        if (slider.touch==0 && 
            sensor.x1 <= startTimer.EXIT_SIZE && 
            sensor.y1 <= startTimer.EXIT_SIZE){  // касание кнопки выхода из настроек
            sensor.touch = 2;
            if (sensor.isTouchElement == FALSE){
                this->isChange = true;
                sensor.isTouchElement = TRUE;
                result = 1;
            }
        }
        if (slider.touch==0 && 
            sensor.x1>(BUTTON_RIGHT_X) && sensor.x1<(BUTTON_RIGHT_X+BUTTON_RIGHT_SIZE) && 
            sensor.y1>(BUTTON_RIGHT_Y) && sensor.y1<(BUTTON_RIGHT_Y+BUTTON_RIGHT_SIZE)){  // касание кнопки секретного меню
            //sensor.touch = 0;
                if (this->numberScreen == 6){
                    if (sensor.isTouchElement == FALSE){
                    sensor.isTouchElement = TRUE;
                    this->numberScreen = 100;
                    mode = 1;
                    //if (this->numberScreen > this->maxNumberScreen) this->numberScreen = 1;
                    this->viewScreenChange();
                }
            }
        }    
    }
    else if (sensor.status == 0){   // отпускание сенсорного экрана
        sensor.touch = 0;
        slider.touch = 0;
        slider.mode = 0;
        sensor.isTouchElement = FALSE;
    }
    if (result == 0) result = this->viewPage(this->numberScreen, mode);
    
    if (sensor.touch != 0){
        hcu.lockTimer = core.getTick();
    }
    
    return result;
}
//-----------------------------------------------------
uint8_t Setup::viewPage(uint8_t number, uint8_t mode)
{
    uint8_t result = 0;
    
    this->maxNumberScreen = 8;
    if (this->numberScreen == 100){
        this->viewScreen100(mode); // секретный экран
    }
    else{
        if (this->numberScreen > this->maxNumberScreen){
            this->numberScreen = this->maxNumberScreen;
            number = this->numberScreen;
        }
        if (number == 1) this->viewScreen1(mode);
        if (number == 2) this->viewScreen10(mode);
        if (number == 3) result = this->viewScreen11(mode);
        if (number == 4) this->viewScreen12(mode);
        if (number == 5) this->viewScreen13(mode);
        if (number == 6) this->viewScreen14(mode); //экран версий и моточасов
        if (number == 7) this->viewScreen15(mode);
				if (number == 8) this->viewScreen16(mode); //Schedule mode screen + temp shift
    }
    
    return result;
}
//-----------------------------------------------------
void Setup::viewScreen1(uint8_t mode)   // настройки дисплея
{
    static int16_t posTimeout=0;
    int16_t result[2];
    char unit;
    
    const int CHECKBOXS_S = 10;
    
    const int SLIDER1_X = 35;
    const int SLIDER1_Y = 65;
    
    const int CHECKBOX2_X = 35;
    const int CHECKBOX2_Y = (SLIDER1_Y+70);
    const int CHECKBOX2_H = 40;
    const int CHECKBOX2_W = 200;
    
    const int SLIDER2_X = 35;
    const int SLIDER2_Y = (CHECKBOX2_Y+50);
    
    if (mode){
        text.writeString(160-10*4,10,"Display",Font_11x18,display.WHITE,display.COLOR_BACK);
        this->isSlider1 = TRUE;
        display.setup.displaySleep |= 2;
        display.setup.viewClock |= 2;
        this->isSlider2 = TRUE;
        if (display.setup.timeout <= 90){
            posTimeout = display.setup.timeout-30;
        }
        else {
            //display.setup.timeout = (posTimeout-50)*6;
            posTimeout = display.setup.timeout/6+50;
        }
        posTimeout = posTimeout - posTimeout%10;
        if (posTimeout < 0) posTimeout = 0;
        else if (posTimeout > 100) posTimeout = 100;
    }
    result[0] = slider.drawSlim(SLIDER1_X, SLIDER1_Y, display.setup.brightness, "Brightness", display.setup.brightness, NULL, mode, &this->isSlider1, TRUE, SLIDER_COLOR_3, 1);
    display.setLight(display.setup.brightness);
    checkbox.draw(CHECKBOX2_X, CHECKBOX2_Y, "Sleep mode", &display.setup.displaySleep);
    if (display.setup.timeout <= 90){
        unit = 's';
        result[1] = slider.drawSlim(SLIDER2_X, SLIDER2_Y, posTimeout, "Timeout", display.setup.timeout, unit, mode, &this->isSlider2, TRUE, SLIDER_COLOR_2, 2);
    }
    else {
        unit = 'm';
        result[1] = slider.drawSlim(SLIDER2_X, SLIDER2_Y, posTimeout, "Timeout", display.setup.timeout/60, unit, mode, &this->isSlider2, TRUE, SLIDER_COLOR_2, 2);
    }
    
    if (result[0] == -1){
        display.setup.brightness -= 1;
        if (display.setup.brightness < 10) display.setup.brightness = 10;
        isSlider1 = TRUE;
    }
    else if(result[0] == -2){
        display.setup.brightness += 1;
        if (display.setup.brightness > 100) display.setup.brightness = 100;
        isSlider1 = TRUE;
    }
    else if (result[0] != -3){
        display.setup.brightness = result[0]/2;
        if (display.setup.brightness < 10) display.setup.brightness = 10;
        if (display.setup.brightness > 100) display.setup.brightness = 100;
        isSlider1 = TRUE;
    }
    
    if (result[1] == -1){
        if (display.setup.timeout <= 90){
            display.setup.timeout -= 10;
        }
        else if (display.setup.timeout <= 120){
            display.setup.timeout -= 30;
        }
        else{
            display.setup.timeout -= 60;
        }
        if (display.setup.timeout < 30) display.setup.timeout = 30;
        if (display.setup.timeout > 300) display.setup.timeout = 300;
        if (display.setup.timeout <= 90){
            posTimeout = display.setup.timeout-30;
        }
        else {
            posTimeout = display.setup.timeout/6+50;
        }
        posTimeout = posTimeout - posTimeout%10;
        if (posTimeout < 0) posTimeout = 0;
        else if (posTimeout > 100) posTimeout = 100;
        isSlider2 = TRUE;
    }
    else if(result[1] == -2){
        if (display.setup.timeout < 90){
            display.setup.timeout += 10;
        }
        else if (display.setup.timeout == 90){
            display.setup.timeout += 30;
        }
        else{
            display.setup.timeout += 60;
        }
        if (display.setup.timeout < 30) display.setup.timeout = 30;
        if (display.setup.timeout > 300) display.setup.timeout = 300;
        if (display.setup.timeout <= 90){
            posTimeout = display.setup.timeout-30;
        }
        else {
            posTimeout = display.setup.timeout/6+50;
        }
        posTimeout = posTimeout - posTimeout%10;
        if (posTimeout < 0) posTimeout = 0;
        else if (posTimeout > 100) posTimeout = 100;
        isSlider2 = TRUE;
    }
    else if (result[1] != -3){
        posTimeout = result[1]/2;
        posTimeout = posTimeout - posTimeout%10;
        if (posTimeout < 0) posTimeout = 0;
        else if (posTimeout > 100) posTimeout = 100;
        if (posTimeout <= 60){
            display.setup.timeout = posTimeout+30;
        }
        else{
            display.setup.timeout = (posTimeout-50)*6;
        }
        isSlider2 = TRUE;
    }
    
    
    if (sensor.status == 1){   // касание сенсорного экрана
        //sensor.touch = 1;
        if ((sensor.x1>=(CHECKBOX2_X-20) && sensor.x1<=(CHECKBOX2_X+CHECKBOX2_W) && sensor.y1>=(CHECKBOX2_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX2_Y+CHECKBOX2_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 2
            sensor.touch = 3;
            display.setup.displaySleep = !display.setup.displaySleep+2;
        }
    }
}
//-----------------------------------------------------
/*
void Setup::viewScreen2(uint8_t mode)   // настройки отключения дисплея
{
    const int CHECKBOXS_S = 10;
    
    const int CHECKBOX1_X = 35;
    const int CHECKBOX1_Y = 85;
    const int CHECKBOX1_H = 40;
    const int CHECKBOX1_W = 200;
    
    const int CHECKBOX2_X = 35;
    const int CHECKBOX2_Y = (85+40);
    const int CHECKBOX2_H = 40;
    const int CHECKBOX2_W = 200;
    
    if (mode){
        text.writeString(160-11*6.5,10,"Display is on",Font_11x18,display.WHITE,display.COLOR_BACK);
        display.setup.displayHeater |= 2;
        display.setup.displaySleep |= 2;
    }
    checkbox.draw(CHECKBOX1_X, CHECKBOX1_Y, "When the system works", &display.setup.displayHeater);
    
    checkbox.draw(CHECKBOX2_X, CHECKBOX2_Y, "All the time", &display.setup.displaySleep);
    
    if (sensor.status == 1){   // касание сенсорного экрана
        //sensor.touch = 1;
        if ((sensor.x1>=(CHECKBOX1_X-20) && sensor.x1<=(CHECKBOX1_X+CHECKBOX1_W) && sensor.y1>=(CHECKBOX1_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX1_Y+CHECKBOX1_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 1
            sensor.touch = 1;
            display.setup.displayHeater = !display.setup.displayHeater+2;
            display.setup.displaySleep = 2;
        }  
        if ((sensor.x1>=(CHECKBOX2_X-20) && sensor.x1<=(CHECKBOX2_X+CHECKBOX2_W) && sensor.y1>=(CHECKBOX2_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX2_Y+CHECKBOX2_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 2
            sensor.touch = 3;
            display.setup.displaySleep = !display.setup.displaySleep+2;
            display.setup.displayHeater = 2;
            display.setup.displayEngine = 2;
        }
        
    }
    
}
//-----------------------------------------------------
void Setup::viewScreen3(uint8_t mode)   // настройки языка
{
    const int CHECKBOXS_S = 10;
    
    const int CHECKBOX1_X = 35;
    const int CHECKBOX1_Y = 55;
    const int CHECKBOX1_H = 40;
    const int CHECKBOX1_W = 200;
    
    if (mode){
        text.writeString(160-11*4,10,"Language",Font_11x18,display.WHITE,display.COLOR_BACK);
        display.setup.language |= 0x80;
    }
    if (display.setup.language & 0xF0){
        display.setup.language &= 0x0F;
        
        checkbox.draw(CHECKBOX1_X, CHECKBOX1_Y, "English", display.setup.language==0);
    }
    
    if (sensor.status == 1){   // касание сенсорного экрана
        //sensor.touch = 1;
        if ((sensor.x1>=(CHECKBOX1_X-20) && sensor.x1<=(CHECKBOX1_X+CHECKBOX1_W) && sensor.y1>=(CHECKBOX1_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX1_Y+CHECKBOX1_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 1
            sensor.touch = 1;
            display.setup.language = 0;
            display.setup.language |= 0x80;
        }
    }  
}
//-----------------------------------------------------
void Setup::viewScreen4(uint8_t mode)   // настройки режимов работы воздушного отопителя
{
    const int CHECKBOXS_S = 10;
    
    const int CHECKBOX1_X = 35;
    const int CHECKBOX1_Y = 55;
    const int CHECKBOX1_H = 40;
    const int CHECKBOX1_W = 200;
    
    const int CHECKBOX2_X = 35;
    const int CHECKBOX2_Y = (55+35);
    const int CHECKBOX2_H = 35;
    const int CHECKBOX2_W = 200;
    
    const int CHECKBOX3_X = 35;
    const int CHECKBOX3_Y = (55+35+35);
    const int CHECKBOX3_H = 35;
    const int CHECKBOX3_W = 200;
    
    const int CHECKBOX4_X = 35;
    const int CHECKBOX4_Y = (55+35+35+35);
    const int CHECKBOX4_H = 35;
    const int CHECKBOX4_W = 200;
    
    if (mode){
        text.writeString(160-11*7,10,"Heater work by",Font_11x18,display.WHITE,display.COLOR_BACK);
        heater.setup.workBy |= 0x80;
    }
    if (heater.setup.workBy & 0xF0){
        heater.setup.workBy &= 0x0F;
        
        checkbox.draw(CHECKBOX1_X, CHECKBOX1_Y, "Heater sensor", heater.setup.workBy==0);
        checkbox.draw(CHECKBOX2_X, CHECKBOX2_Y, "Panel sensor", heater.setup.workBy==1);
        checkbox.draw(CHECKBOX3_X, CHECKBOX3_Y, "Outside sensor", heater.setup.workBy==2);
        checkbox.draw(CHECKBOX4_X, CHECKBOX4_Y, "Power", heater.setup.workBy==3);
    }
    
    if (sensor.status == 1){   // касание сенсорного экрана
        //sensor.touch = 1;
        if ((sensor.x1>=(CHECKBOX1_X-20) && sensor.x1<=(CHECKBOX1_X+CHECKBOX1_W) && sensor.y1>=(CHECKBOX1_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX1_Y+CHECKBOX1_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 1
            sensor.touch = 1;
            heater.setup.workBy = 0;
            heater.setup.workBy |= 0x80;
        }  
        if ((sensor.x1>=(CHECKBOX2_X-20) && sensor.x1<=(CHECKBOX2_X+CHECKBOX2_W) && sensor.y1>=(CHECKBOX2_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX2_Y+CHECKBOX2_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 2
            sensor.touch = 2;
            heater.setup.workBy = 1;
            heater.setup.workBy |= 0x80;
        }
        if ((sensor.x1>=(CHECKBOX3_X-20) && sensor.x1<=(CHECKBOX3_X+CHECKBOX3_W) && sensor.y1>=(CHECKBOX3_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX3_Y+CHECKBOX3_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 3
            sensor.touch = 3;
            heater.setup.workBy = 2;
            heater.setup.workBy |= 0x80;
        }
        if ((sensor.x1>=(CHECKBOX4_X-20) && sensor.x1<=(CHECKBOX4_X+CHECKBOX4_W) && sensor.y1>=(CHECKBOX4_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX4_Y+CHECKBOX4_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 4
            sensor.touch = 4;
            heater.setup.workBy = 3;
            heater.setup.workBy |= 0x80;
        }
    }
}
//-----------------------------------------------------
void Setup::viewScreen5(uint8_t mode)   // настройки времени работы воздушного отопителя
{
    static uint8_t d, h, m;
    
    const int SLIDER1_X = 35;
    const int SLIDER1_Y = 40;
    const int SLIDER1_H = 50;
    const int SLIDER1_W = 200;
    
    const int SLIDER2_X = 35;
    const int SLIDER2_Y = (40+50);
    const int SLIDER2_H  = 50;
    const int SLIDER2_W = 200;
    
    const int SLIDER3_X = 35;
    const int SLIDER3_Y = (40+50+50);
    const int SLIDER3_H  = 50;
    const int SLIDER3_W = 200;
    
    const int CHECKBOXS_S = 10;
    
    const int CHECKBOX1_X = 35;
    const int CHECKBOX1_Y = (40+50+50+60);
    const int CHECKBOX1_H = 60;
    const int CHECKBOX1_W = 200;
    
    if (mode){
        text.writeString(160-11*4,10,"Work Time",Font_11x18,display.WHITE,display.COLOR_BACK);
        this->isSlider1 = TRUE;
        this->isSlider2 = TRUE;
        this->isSlider3 = TRUE;
        heater.setup.workUnlimited |= 2;
        d = heater.setup.workDay*14.3;
        h = heater.setup.workHour*4.35;
        m = heater.setup.workMinute*1.8;
    }
    
    slider.draw(SLIDER1_X, SLIDER1_Y, d, "Days", heater.setup.workDay, NULL, mode, &this->isSlider1, !(heater.setup.workUnlimited&0x01));
    slider.draw(SLIDER2_X, SLIDER2_Y, h, "Hours", heater.setup.workHour, NULL, mode, &this->isSlider2, !(heater.setup.workUnlimited&0x01));
    slider.draw(SLIDER3_X, SLIDER3_Y, m, "Minutes", heater.setup.workMinute, NULL, mode, &this->isSlider3, !(heater.setup.workUnlimited&0x01));
    checkbox.draw(CHECKBOX1_X, CHECKBOX1_Y, "Unlimited", &heater.setup.workUnlimited);
    
    if (sensor.status == 1){   // касание сенсорного экрана
        //sensor.touch = 1;
        if ((sensor.x1>=(SLIDER1_X) && sensor.x1<=(SLIDER1_X+SLIDER1_W) && sensor.y1>=(SLIDER1_Y) && sensor.y1<=(SLIDER1_Y+SLIDER1_H) && sensor.touch==0) || sensor.touch==1){ // касание ползунка 1
            sensor.touch = 1;
            heater.setup.workDay = (sensor.x1-SLIDER1_X)/28;
            if (heater.setup.workDay < 0) heater.setup.workDay = 0;
            else if (heater.setup.workDay > 7) heater.setup.workDay = 7;
            if (heater.setup.workDay==0 && heater.setup.workHour==0 && heater.setup.workMinute<10){
                heater.setup.workMinute = 10;
                m = heater.setup.workMinute*1.8;
                this->isSlider3 = TRUE;
            }
            d = heater.setup.workDay*14.3;
            this->isSlider1 = TRUE;
        }  
        if ((sensor.x1>=(SLIDER2_X) && sensor.x1<=(SLIDER2_X+SLIDER2_W) && sensor.y1>=(SLIDER2_Y) && sensor.y1<=(SLIDER2_Y+SLIDER2_H) && sensor.touch==0) || sensor.touch==2){ // касание ползунка 2
            sensor.touch = 2;
            heater.setup.workHour = (sensor.x1-SLIDER2_X)/8;
            if (heater.setup.workHour < 0) heater.setup.workHour = 0;
            else if (heater.setup.workHour > 23) heater.setup.workHour = 23;
            if (heater.setup.workDay==0 && heater.setup.workHour==0 && heater.setup.workMinute<10){
                heater.setup.workMinute = 10;
                m = heater.setup.workMinute*1.8;
                this->isSlider3 = TRUE;
            }
            h = heater.setup.workHour*4.35;
            this->isSlider2 = TRUE;
        }
        if ((sensor.x1>=(SLIDER3_X) && sensor.x1<=(SLIDER3_X+SLIDER3_W) && sensor.y1>=(SLIDER3_Y) && sensor.y1<=(SLIDER3_Y+SLIDER3_H) && sensor.touch==0) || sensor.touch==3){ // касание ползунка 3
            sensor.touch = 3;
            heater.setup.workMinute = (sensor.x1-SLIDER3_X)/3;
            heater.setup.workMinute = heater.setup.workMinute - heater.setup.workMinute%5;
            if (heater.setup.workMinute < 0) heater.setup.workMinute = 0;
            else if (heater.setup.workMinute > 55) heater.setup.workMinute = 55;
            if (heater.setup.workDay==0 && heater.setup.workHour==0 && heater.setup.workMinute<10){
                heater.setup.workMinute = 10;
            }
            m = heater.setup.workMinute*1.8;
            this->isSlider3 = TRUE;
        }
        if ((sensor.x1>=(CHECKBOX1_X-20) && sensor.x1<=(CHECKBOX1_X+CHECKBOX1_W) && sensor.y1>=(CHECKBOX1_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX1_Y+CHECKBOX1_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 1
            sensor.touch = 4;
            heater.setup.workUnlimited = !heater.setup.workUnlimited+2;
            this->isSlider1 = TRUE;
            this->isSlider2 = TRUE;
            this->isSlider3 = TRUE;
        }  
    }
}
//-----------------------------------------------------
void Setup::viewScreen6(uint8_t mode)   // настройки работы таймеров запуска
{
    const int CHECKBOXS_S = 10;
    
    const int CHECKBOX1_X = 35;
    const int CHECKBOX1_Y = 55;
    const int CHECKBOX1_H = 40;
    const int CHECKBOX1_W = 200;
    
    if (mode){
        text.writeString(160-11*3,10,"Timers",Font_11x18,display.WHITE,display.COLOR_BACK);
        startTimer.isUnlimited |= 0x80;
    }
    if (startTimer.isUnlimited & 0xF0){
        startTimer.isUnlimited &= 0x0F;
        
        checkbox.draw(CHECKBOX1_X, CHECKBOX1_Y, "Do not reset timers", startTimer.isUnlimited == 1);
    }
    
    if (sensor.status == 1){   // касание сенсорного экрана
        //sensor.touch = 1;
        if ((sensor.x1>=(CHECKBOX1_X-20) && sensor.x1<=(CHECKBOX1_X+CHECKBOX1_W) && sensor.y1>=(CHECKBOX1_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX1_Y+CHECKBOX1_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 1
            sensor.touch = 1;
            startTimer.isUnlimited = !startTimer.isUnlimited;
            startTimer.isUnlimited |= 0x80;
        }  
    }  
}
//-----------------------------------------------------
void Setup::viewScreen7(uint8_t mode)   // настройки жидкостного подогревателя/догревателя
{
    const int CHECKBOXS_S = 10;
    
    const int CHECKBOX1_X = 35;
    const int CHECKBOX1_Y = 55;
    const int CHECKBOX1_H = 40;
    const int CHECKBOX1_W = 200;
    
    const int CHECKBOX2_X = CHECKBOX1_X;
    const int CHECKBOX2_Y = (CHECKBOX1_Y+35);
    const int CHECKBOX2_H = CHECKBOX1_H;
    const int CHECKBOX2_W = CHECKBOX1_W;
    
    const int SLIDER1_X = 35;
    const int SLIDER1_Y = (CHECKBOX2_Y+35);
    const int SLIDER1_H = 50;
    const int SLIDER1_W = 200;
    
    const int SLIDER2_X = 35;
    const int SLIDER2_Y = (SLIDER1_Y+60);
    const int SLIDER2_H  = SLIDER1_H;
    const int SLIDER2_W = SLIDER1_W;
    
    if (mode){
        text.writeString(160-11*9,10,"Preheater & warm up",Font_11x18,display.WHITE,display.COLOR_BACK);
        heater.setup.warmUpAuto |= 2;
        heater.setup.warmUpManual |= 2;
        this->isSlider1 = TRUE;
        this->isSlider2 = TRUE;
    }
    if (((heater.setup.warmUpAuto | heater.setup.warmUpManual) & 0x02)==0x02) this->isSlider1 = TRUE;
    checkbox.draw(CHECKBOX1_X, CHECKBOX1_Y, "Warm up auto", &heater.setup.warmUpAuto);
    checkbox.draw(CHECKBOX2_X, CHECKBOX2_Y, "Warm up manual", &heater.setup.warmUpManual);
    slider.draw(SLIDER1_X, SLIDER1_Y, (heater.setup.warmUpTSetpoint-80)*6.6666, "Warm up temperature", heater.setup.warmUpTSetpoint, 'C', mode, &this->isSlider1, (heater.setup.warmUpAuto | heater.setup.warmUpManual)==TRUE);
    slider.draw(SLIDER2_X, SLIDER2_Y, (heater.setup.preheaterTSetpoint-20)*1.3333, "Preheater temperature", heater.setup.preheaterTSetpoint, 'C', mode, &this->isSlider2, TRUE);
    
    if (sensor.status == 1){   // касание сенсорного экрана
        //sensor.touch = 1;
        if ((sensor.x1>=(CHECKBOX1_X-20) && sensor.x1<=(CHECKBOX1_X+CHECKBOX1_W) && sensor.y1>=(CHECKBOX1_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX1_Y+CHECKBOX1_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 1
            sensor.touch = 1;
            heater.setup.warmUpAuto = !heater.setup.warmUpAuto+2;
            if (heater.setup.warmUpAuto == (TRUE+2)) heater.setup.warmUpManual = FALSE+2;
        }
        if ((sensor.x1>=(CHECKBOX2_X-20) && sensor.x1<=(CHECKBOX2_X+CHECKBOX2_W) && sensor.y1>=(CHECKBOX2_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX2_Y+CHECKBOX2_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 2
            sensor.touch = 2;
            heater.setup.warmUpManual = !heater.setup.warmUpManual+2;
            if (heater.setup.warmUpManual == (TRUE+2)) heater.setup.warmUpAuto = FALSE+2;
        }
        if ((sensor.x1>=(SLIDER1_X) && sensor.x1<=(SLIDER1_X+SLIDER1_W) && sensor.y1>=(SLIDER1_Y) && sensor.y1<=(SLIDER1_Y+SLIDER1_H) && sensor.touch==0) || sensor.touch==3){ // касание ползунка 1
            sensor.touch = 3;
            heater.setup.warmUpTSetpoint = (sensor.x1-SLIDER1_X)/13.3+80;
            if (heater.setup.warmUpTSetpoint < 80) heater.setup.warmUpTSetpoint = 80;
            if (heater.setup.warmUpTSetpoint > 95) heater.setup.warmUpTSetpoint = 95;
            this->isSlider1 = TRUE;
        }  
        if ((sensor.x1>=(SLIDER2_X) && sensor.x1<=(SLIDER2_X+SLIDER2_W) && sensor.y1>=(SLIDER2_Y) && sensor.y1<=(SLIDER2_Y+SLIDER2_H) && sensor.touch==0) || sensor.touch==4){ // касание ползунка 2
            sensor.touch = 4;
            heater.setup.preheaterTSetpoint = (sensor.x1-SLIDER2_X)/2.66+20;
            if (heater.setup.preheaterTSetpoint < 20) heater.setup.preheaterTSetpoint = 20;
            if (heater.setup.preheaterTSetpoint > 95) heater.setup.preheaterTSetpoint = 95;
            this->isSlider2 = TRUE;
        }
    }
}
//-----------------------------------------------------
void Setup::viewScreen8(uint8_t mode)   // настройки отопителя салона и помпы
{
    const int CHECKBOXS_S = 10;
    
    const int CHECKBOX1_X = 35;
    const int CHECKBOX1_Y = 55;
    const int CHECKBOX1_H = 40;
    const int CHECKBOX1_W = 200;
    
    const int CHECKBOX2_X = CHECKBOX1_X;
    const int CHECKBOX2_Y = (CHECKBOX1_Y+35);
    const int CHECKBOX2_H = CHECKBOX1_H;
    const int CHECKBOX2_W = CHECKBOX1_W;
    
    const int CHECKBOX3_X = CHECKBOX2_X;
    const int CHECKBOX3_Y = (CHECKBOX2_Y+35);
    const int CHECKBOX3_H = CHECKBOX2_H;
    const int CHECKBOX3_W = CHECKBOX2_W;
    
    const int SLIDER1_X = 35;
    const int SLIDER1_Y = (CHECKBOX3_Y+35);
    const int SLIDER1_H = 50;
    const int SLIDER1_W = 200;
    
    if (mode){
        text.writeString(160-11*10.5,10,"Pump & vehicle heater",Font_11x18,display.WHITE,display.COLOR_BACK);
        heater.setup.pumpInStandby |= 2;
        heater.setup.pumpOnEngine |= 2;
        heater.setup.turnOnFurnace |= 2;
        this->isSlider1 = TRUE;
    }
    if ((heater.setup.turnOnFurnace & 0x02)==0x02) this->isSlider1 = TRUE;
    checkbox.draw(CHECKBOX1_X, CHECKBOX1_Y, "Pump in standby mode", &heater.setup.pumpInStandby);
    checkbox.draw(CHECKBOX2_X, CHECKBOX2_Y, "Pump on with engine", &heater.setup.pumpOnEngine);
    checkbox.draw(CHECKBOX3_X, CHECKBOX3_Y, "Turn on vehicle heater", &heater.setup.turnOnFurnace);
    slider.draw(SLIDER1_X, SLIDER1_Y, (heater.setup.furnaceTSetpoint-30)*3.33333, "Vehicle heater temp.", heater.setup.furnaceTSetpoint, 'C', mode, &this->isSlider1, heater.setup.turnOnFurnace==TRUE);
    
    if (sensor.status == 1){   // касание сенсорного экрана
        //sensor.touch = 1;
        if ((sensor.x1>=(CHECKBOX1_X-20) && sensor.x1<=(CHECKBOX1_X+CHECKBOX1_W) && sensor.y1>=(CHECKBOX1_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX1_Y+CHECKBOX1_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 1
            sensor.touch = 1;
            heater.setup.pumpInStandby = !heater.setup.pumpInStandby+2;
        }
        if ((sensor.x1>=(CHECKBOX2_X-20) && sensor.x1<=(CHECKBOX2_X+CHECKBOX2_W) && sensor.y1>=(CHECKBOX2_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX2_Y+CHECKBOX2_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 2
            sensor.touch = 2;
            heater.setup.pumpOnEngine = !heater.setup.pumpOnEngine+2;
        }
        if ((sensor.x1>=(CHECKBOX3_X-20) && sensor.x1<=(CHECKBOX3_X+CHECKBOX3_W) && sensor.y1>=(CHECKBOX3_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX3_Y+CHECKBOX3_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 3
            sensor.touch = 3;
            heater.setup.turnOnFurnace = !heater.setup.turnOnFurnace+2;
        }
        if ((sensor.x1>=(SLIDER1_X) && sensor.x1<=(SLIDER1_X+SLIDER1_W) && sensor.y1>=(SLIDER1_Y) && sensor.y1<=(SLIDER1_Y+SLIDER1_H) && sensor.touch==0) || sensor.touch==4){ // касание ползунка 1
            sensor.touch = 4;
            heater.setup.furnaceTSetpoint = (sensor.x1-SLIDER1_X)/6.66+30;
            if (heater.setup.furnaceTSetpoint < 30) heater.setup.furnaceTSetpoint = 30;
            if (heater.setup.furnaceTSetpoint > 60) heater.setup.furnaceTSetpoint = 60;
            this->isSlider1 = TRUE;
        }
    }
}
//-----------------------------------------------------
void Setup::viewScreen9(uint8_t mode)   // настройки времени работы жидкостного отопителя
{
    static uint8_t d, h, m;
    
    const int SLIDER1_X = 35;
    const int SLIDER1_Y = 40;
    const int SLIDER1_H = 50;
    const int SLIDER1_W = 200;
    
    const int SLIDER2_X = 35;
    const int SLIDER2_Y = (40+50);
    const int SLIDER2_H  = 50;
    const int SLIDER2_W = 200;
    
    const int SLIDER3_X = 35;
    const int SLIDER3_Y = (40+50+50);
    const int SLIDER3_H  = 50;
    const int SLIDER3_W = 200;
    
    const int CHECKBOXS_S = 10;
    
    const int CHECKBOX1_X = 35;
    const int CHECKBOX1_Y = (40+50+50+60);
    const int CHECKBOX1_H = 60;
    const int CHECKBOX1_W = 200;
    
    if (mode){
        text.writeString(160-11*4,10,"Work Time",Font_11x18,display.WHITE,display.COLOR_BACK);
        this->isSlider1 = TRUE;
        this->isSlider2 = TRUE;
        this->isSlider3 = TRUE;
        heater.setup.externalOperating |= 2;
        d = heater.setup.workDay*14.3;
        h = heater.setup.workHour*4.35;
        m = heater.setup.workMinute*1.8;
    }
    
    slider.draw(SLIDER1_X, SLIDER1_Y, d, "Days", heater.setup.workDay, NULL, mode, &this->isSlider1, TRUE);
    slider.draw(SLIDER2_X, SLIDER2_Y, h, "Hours", heater.setup.workHour, NULL, mode, &this->isSlider2, TRUE);
    slider.draw(SLIDER3_X, SLIDER3_Y, m, "Minutes", heater.setup.workMinute, NULL, mode, &this->isSlider3, TRUE);
    checkbox.draw(CHECKBOX1_X, CHECKBOX1_Y, "External operating", &heater.setup.externalOperating);
    
    if (sensor.status == 1){   // касание сенсорного экрана
        //sensor.touch = 1;
        if ((sensor.x1>=(SLIDER1_X) && sensor.x1<=(SLIDER1_X+SLIDER1_W) && sensor.y1>=(SLIDER1_Y) && sensor.y1<=(SLIDER1_Y+SLIDER1_H) && sensor.touch==0) || sensor.touch==1){ // касание ползунка 1
            sensor.touch = 1;
            heater.setup.workDay = (sensor.x1-SLIDER1_X)/28;
            if (heater.setup.workDay < 0) heater.setup.workDay = 0;
            else if (heater.setup.workDay > 7) heater.setup.workDay = 7;
            if (heater.setup.workDay==0 && heater.setup.workHour==0 && heater.setup.workMinute<10){
                heater.setup.workMinute = 10;
                m = heater.setup.workMinute*1.8;
                this->isSlider3 = TRUE;
            }
            d = heater.setup.workDay*14.3;
            this->isSlider1 = TRUE;
        }  
        if ((sensor.x1>=(SLIDER2_X) && sensor.x1<=(SLIDER2_X+SLIDER2_W) && sensor.y1>=(SLIDER2_Y) && sensor.y1<=(SLIDER2_Y+SLIDER2_H) && sensor.touch==0) || sensor.touch==2){ // касание ползунка 2
            sensor.touch = 2;
            heater.setup.workHour = (sensor.x1-SLIDER2_X)/8;
            if (heater.setup.workHour < 0) heater.setup.workHour = 0;
            else if (heater.setup.workHour > 23) heater.setup.workHour = 23;
            if (heater.setup.workDay==0 && heater.setup.workHour==0 && heater.setup.workMinute<10){
                heater.setup.workMinute = 10;
                m = heater.setup.workMinute*1.8;
                this->isSlider3 = TRUE;
            }
            h = heater.setup.workHour*4.35;
            this->isSlider2 = TRUE;
        }
        if ((sensor.x1>=(SLIDER3_X) && sensor.x1<=(SLIDER3_X+SLIDER3_W) && sensor.y1>=(SLIDER3_Y) && sensor.y1<=(SLIDER3_Y+SLIDER3_H) && sensor.touch==0) || sensor.touch==3){ // касание ползунка 3
            sensor.touch = 3;
            heater.setup.workMinute = (sensor.x1-SLIDER3_X)/3;
            heater.setup.workMinute = heater.setup.workMinute - heater.setup.workMinute%5;
            if (heater.setup.workMinute < 0) heater.setup.workMinute = 0;
            else if (heater.setup.workMinute > 55) heater.setup.workMinute = 55;
            if (heater.setup.workDay==0 && heater.setup.workHour==0 && heater.setup.workMinute<10){
                heater.setup.workMinute = 10;
            }
            m = heater.setup.workMinute*1.8;
            this->isSlider3 = TRUE;
        }
        if ((sensor.x1>=(CHECKBOX1_X-20) && sensor.x1<=(CHECKBOX1_X+CHECKBOX1_W) && sensor.y1>=(CHECKBOX1_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX1_Y+CHECKBOX1_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 1
            sensor.touch = 4;
            heater.setup.externalOperating = !heater.setup.externalOperating+2;
        }  
    }
}
*/
//-----------------------------------------------------
void Setup::viewScreen10(uint8_t mode)   // настройки отопителя салона и помпы
{
    int16_t result;
    
    const int CHECKBOXS_S = 10;
    
    const int CHECKBOX1_X = 35;
    const int CHECKBOX1_Y = 65;
    const int CHECKBOX1_H = 40;
    const int CHECKBOX1_W = 200;
    
    const int CHECKBOX2_X = CHECKBOX1_X;
    const int CHECKBOX2_Y = (CHECKBOX1_Y+35);
    const int CHECKBOX2_H = CHECKBOX1_H;
    const int CHECKBOX2_W = CHECKBOX1_W;
    
    const int SLIDER1_X = 35;
    const int SLIDER1_Y = (CHECKBOX2_Y+35);
    
    const int CHECKBOX3_X = CHECKBOX2_X;
    const int CHECKBOX3_Y = (SLIDER1_Y+55);
    const int CHECKBOX3_H = CHECKBOX2_H;
    const int CHECKBOX3_W = CHECKBOX2_W;
    
    if (mode){
        text.writeString(160-10*5.5,10,"Fan speed &",Font_11x18,display.WHITE,display.COLOR_BACK);
        text.writeString(160-10*6,30,"coolant pump",Font_11x18,display.WHITE,display.COLOR_BACK);
        hcu.fanAuto |= 2;
        hcu.fanManual |= 2;
        this->isSlider1 = TRUE;
        hcu.pumpOn |= 2;
    }
    if ((hcu.fanManual & 0x02)==0x02) this->isSlider1 = TRUE;
    checkbox.draw(CHECKBOX1_X, CHECKBOX1_Y, "Fan auto mode", &hcu.fanAuto);
    checkbox.draw(CHECKBOX2_X, CHECKBOX2_Y, "Fan manual mode", &hcu.fanManual);
    //slider.draw(SLIDER1_X, SLIDER1_Y, hcu.fanPower, "Fan power", hcu.fanPower, '%', mode, &this->isSlider1, hcu.fanManual==TRUE);
    result = slider.drawSlim(SLIDER1_X, SLIDER1_Y, hcu.fanPower, "Fan power", hcu.fanPower, '%', mode, &this->isSlider1, hcu.fanManual==TRUE, SLIDER_COLOR_3, 4);
    checkbox.draw(CHECKBOX3_X, CHECKBOX3_Y, "Pump override (10 min)", &hcu.pumpOn);
    
    if (result == -1){
        hcu.fanPower -= 1;
        if (hcu.fanPower < 20) hcu.fanPower = 20;
        isSlider1 = TRUE;
    }
    else if(result == -2){
        hcu.fanPower += 1;
        if (hcu.fanPower > 100) hcu.fanPower = 100;
        isSlider1 = TRUE;
    }
    else if (result != -3){
        hcu.fanPower = result/2;
        if (hcu.fanPower < 20) hcu.fanPower = 20;
        if (hcu.fanPower > 100) hcu.fanPower = 100;
        isSlider1 = TRUE;
    }
    
    
    if (sensor.status == 1){   // касание сенсорного экрана
        //sensor.touch = 1;
        if ((sensor.x1>=(CHECKBOX1_X-20) && sensor.x1<=(CHECKBOX1_X+CHECKBOX1_W) && sensor.y1>=(CHECKBOX1_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX1_Y+CHECKBOX1_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 1
            sensor.touch = 1;
            hcu.fanAuto = !hcu.fanAuto+2;
            hcu.fanManual = !(hcu.fanAuto&0x01)+2;
        }
        if ((sensor.x1>=(CHECKBOX2_X-20) && sensor.x1<=(CHECKBOX2_X+CHECKBOX2_W) && sensor.y1>=(CHECKBOX2_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX2_Y+CHECKBOX2_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 2
            sensor.touch = 2;
            hcu.fanManual = !hcu.fanManual+2;
            hcu.fanAuto = !(hcu.fanManual&0x01)+2;
        }
        if ((sensor.x1>=(CHECKBOX3_X-20) && sensor.x1<=(CHECKBOX3_X+CHECKBOX3_W) && sensor.y1>=(CHECKBOX3_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX3_Y+CHECKBOX3_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 3
            sensor.touch = 3;
            hcu.pumpOn = !hcu.pumpOn+2;
            if (hcu.pumpOn == 3){
                hcu.timerPumpOn = core.getTick();  
            }
        }
    }
}
//-----------------------------------------------------
uint8_t Setup::viewScreen11(uint8_t mode)   // настройки времени
{
    uint8_t result = 0;
    uint8_t n;
    char str[16];
    static uint8_t numSelectSlider = 0;
    uint8_t z1 = (air.dayTimeH+air.dayTimeM/60.0)*4.196;
    uint8_t z2 = (air.nightTimeH+air.nightTimeM/60.0)*4.196;
    
    const int CHECKBOXS_S = 10;
    
    const int CHECKBOX1_X = 35;
    const int CHECKBOX1_Y = 55;
    const int CHECKBOX1_H = 40;
    const int CHECKBOX1_W = 200;
    
    const int CHECKBOX2_X = 35;
    const int CHECKBOX2_Y = CHECKBOX1_Y+50;
    const int CHECKBOX2_H = 40;
    const int CHECKBOX2_W = 200;
    
    const int SLIDER1_X = 35;
    const int SLIDER1_Y = (CHECKBOX2_Y+45);
    const int SLIDER1_H = 50;
    const int SLIDER1_W = 240;
    
    const int BUTTON1_X = SLIDER1_X-20;
    const int BUTTON1_Y = SLIDER1_Y+59;
    const int BUTTON1_H = 50;
    const int BUTTON1_W = 50;
    
    const int BUTTON2_X = SLIDER1_X+SLIDER1_W+22;
    const int BUTTON2_Y = SLIDER1_Y+60;
    const int BUTTON2_H = 50;
    const int BUTTON2_W = 50;
    
    if (mode){
        text.writeString(160-10*2.5,10,"Clock",Font_11x18,display.WHITE,display.COLOR_BACK);
        display.setup.viewClock |= 2;
        canvas.writeMinus(SLIDER1_X-10, SLIDER1_Y+62);
        canvas.writePlus(SLIDER1_X+256, SLIDER1_Y+62);
        this->isSlider1 = TRUE;
    }
    
    if (this->isSlider1 == TRUE){
        for (n=0; n<15; n++) str[n] = ' ';
        str[15] = 0;
        n = 1;
        if (display.setup.h12){
            bool isAm;
            if (air.dayTimeH < 12){
                n += text.decToString(&str[n], air.dayTimeH);
                isAm = true;  
            }
            else{
                if (air.dayTimeH > 12){
                    n += text.decToString(&str[n], air.dayTimeH-12);
                }
                else{
                    n += text.decToString(&str[n], air.dayTimeH);
                }
                isAm = false;
            }
            str[n++] = ':'; 
            n += text.decToStringFix(&str[n], air.dayTimeM, 2);
            if (isAm == true){
                str[n++] = 'A'; 
            }
            else{
                str[n++] = 'P'; 
            }
            str[n++] = '-';
            if (air.nightTimeH < 12){
                n += text.decToString(&str[n], air.nightTimeH);
                isAm = true; 
            }
            else{
                if (air.nightTimeH > 12){
                    n += text.decToString(&str[n], air.nightTimeH-12);
                }
                else{
                    n += text.decToString(&str[n], air.nightTimeH);
                }
                isAm = false;
            }
            str[n++] = ':'; 
            n += text.decToStringFix(&str[n], air.nightTimeM, 2);
            if (isAm == true){
                str[n++] = 'A'; 
            }
            else{
                str[n++] = 'P'; 
            }
            text.writeDigit((320-n*16)/2-8,SLIDER1_Y+25,str,display.COLOR_TEXT,false);
        }
        else{
            n += text.decToString(&str[n], air.dayTimeH);
            str[n++] = ':'; 
            n += text.decToStringFix(&str[n], air.dayTimeM, 2);
            str[n++] = '-';
            n += text.decToString(&str[n], air.nightTimeH);
            str[n++] = ':'; 
            n += text.decToStringFix(&str[n], air.nightTimeM, 2);
            text.writeDigit((320-n*16)/2-8,SLIDER1_Y+25,str,display.COLOR_TEXT,false);
        }
    }
    
    checkbox.draw(CHECKBOX1_X, CHECKBOX1_Y, "Show clock in sleep", &display.setup.viewClock);
    checkbox.draw(CHECKBOX2_X, CHECKBOX2_Y, "Set up time", false);
    slider.drawDouble(SLIDER1_X, SLIDER1_Y, z1, z2, "Daytime", mode, &this->isSlider1, numSelectSlider);
    
    if (sensor.status == 1){   // касание сенсорного экрана
        //sensor.touch = 1;
        if ((sensor.x1>=(CHECKBOX1_X-20) && sensor.x1<=(CHECKBOX1_X+CHECKBOX1_W) && sensor.y1>=(CHECKBOX1_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX1_Y+CHECKBOX1_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 1
            sensor.touch = 3;
            display.setup.viewClock = !display.setup.viewClock+2;
        }
        if ((sensor.x1>=(CHECKBOX2_X-20) && sensor.x1<=(CHECKBOX2_X+CHECKBOX2_W) && sensor.y1>=(CHECKBOX2_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX2_Y+CHECKBOX2_H+CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 2
            sensor.touch = 3;
            result = 2;
        }
        if ((sensor.x1>=(SLIDER1_X) && sensor.x1<=(SLIDER1_X+z1*2.4+50) && sensor.y1>=(SLIDER1_Y+30) && sensor.y1<=(SLIDER1_Y+30+SLIDER1_H) && sensor.touch==0) || sensor.touch==4){ // касание ползунка 1
            sensor.touch = 4;
            numSelectSlider = 0;
            if (sensor.x1 < (SLIDER1_X+z2*2.4-30)){
                float value = (sensor.x1-SLIDER1_X)/2.4/4.196*6;
                value = (int)(value+0.5f)*10;
                if (value < 0) value = 0;
                else if (value > 1430) value = 1430;
                air.dayTimeH = value/60;
                air.dayTimeM = value - air.dayTimeH*60;
            }
            this->isSlider1 = TRUE;
        }
        if ((sensor.x1>=(SLIDER1_X+z2*2.4-50) && sensor.x1<=(SLIDER1_X+SLIDER1_W) && sensor.y1>=(SLIDER1_Y+30) && sensor.y1<=(SLIDER1_Y+30+SLIDER1_H) && sensor.touch==0) || sensor.touch==5){ // касание ползунка 2
            sensor.touch = 5;
            numSelectSlider = 1;
            if (sensor.x1 > (SLIDER1_X+z1*2.4+50)){
                float value = (sensor.x1-SLIDER1_X)/2.4/4.196*6;
                value = (int)(value+0.5f)*10;
                if (value < 0) value = 0;
                else if (value > 1430) value = 1430;
                air.nightTimeH = value/60;
                air.nightTimeM = value - air.nightTimeH*60;
            }
            this->isSlider1 = TRUE;
        }
        if ((sensor.x1>=(0) && sensor.x1<=(BUTTON1_X+BUTTON1_W) && sensor.y1>=(BUTTON1_Y-BUTTON1_H) && sensor.y1<=(BUTTON1_Y+BUTTON1_H) && sensor.touch==0)){ // касание кнопки
            sensor.touch = 6;
            if (numSelectSlider == 0){
                if (air.dayTimeM > 0){
                    air.dayTimeM -= 10;
                }
                else{
                    air.dayTimeM = 50;
                    if (air.dayTimeH > 0){
                        air.dayTimeH--;
                    }
                }
            }
            else{
                if (air.nightTimeM > 0){
                    air.nightTimeM -= 10;
                }
                else{
                    air.nightTimeM = 50;
                    if (air.nightTimeH > 0){
                        air.nightTimeH--;
                    }
                }
            }
            this->isSlider1 = TRUE;
        }
        if ((sensor.x1>=(0) && sensor.x1<=(BUTTON2_X+BUTTON2_W) && sensor.y1>=(BUTTON2_Y-BUTTON2_H) && sensor.y1<=(BUTTON2_Y+BUTTON2_H) && sensor.touch==0)){ // касание кнопки
            sensor.touch = 7;
            if (numSelectSlider == 0){
                if (air.dayTimeM < 50){
                    air.dayTimeM += 10;
                }
                else{
                    air.dayTimeM = 0;
                    if (air.dayTimeH < 23){
                        air.dayTimeH++;
                    }
                }
            }
            else{
                if (air.nightTimeM < 50){
                    air.nightTimeM += 10;
                }
                else{
                    air.nightTimeM = 0;
                    if (air.nightTimeH < 23){
                        air.nightTimeH++;
                    }
                }
            }
            this->isSlider1 = TRUE;
        }
    }
    return result;
}
//-----------------------------------------------------
void Setup::viewScreen12(uint8_t mode)   // сервисный режим
{
    uint8_t x, y, n;
    char str[15];
    const uint8_t Y_STEP = 23;
    static uint32_t timer = 0;
    char unit;
    
    if (display.setup.celsius & 0x01) unit = 'C';
    else unit = 'F';
    
    if (mode){
        text.writeString(160-10*6,10,"Service info",Font_11x18,display.WHITE,display.COLOR_BACK);
        
        x = 35;
        y = 52;
        text.writeString(x,y,"Air temp:",Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
        y += Y_STEP;
        text.writeString(x,y,"Tank temp:",Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
        y += Y_STEP;
        text.writeString(x,y,"Heater temp:",Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
        y += Y_STEP;
        text.writeString(x,y,"Heat exchange:",Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
        y += Y_STEP;
        text.writeString(35,y,"Circ pump:",Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
        y += Y_STEP;
        text.writeString(x,y,"Solenoid:",Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
        y += Y_STEP;
        text.writeString(x,y,"Fuel pump:",Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
        y += Y_STEP;
        text.writeString(x,y,"Element relay:",Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
        y += Y_STEP;
    }
    
    if ((core.getTick() - timer) > 1000){
        timer = core.getTick();
        
        x = 220;
        y = 52;
        n = text.decToString(str, (int32_t)air.temperatureActual);
        str[n++] = '~'; str[n++] = unit; str[n++] = ' '; str[n] = 0;
        text.writeString(x,y,str,Font_11x18,0xFFC000,display.COLOR_BACK);
        
        y += Y_STEP;
        n = text.decToString(str, (int32_t)hcu.temperatureTank);
        str[n++] = '~'; str[n++] = unit; str[n++] = ' '; str[n] = 0;
        text.writeString(x,y,str,Font_11x18,0xFFC000,display.COLOR_BACK);
        
        y += Y_STEP;
        n = text.decToString(str, (int32_t)hcu.temperatureHeater);
        str[n++] = '~'; str[n++] = unit; str[n++] = ' '; str[n] = 0;
        text.writeString(x,y,str,Font_11x18,0xFFC000,display.COLOR_BACK);
        
        y += Y_STEP;
        n = text.decToString(str, (int32_t)hcu.temperatureExchanger);
        str[n++] = '~'; str[n++] = unit; str[n++] = ' '; str[n] = 0;
        text.writeString(x,y,str,Font_11x18,0xFFC000,display.COLOR_BACK);
        
        y += Y_STEP;
        if (hcu.statePump) text.writeString(x,y,"Yes",Font_11x18,0xFF0F00,display.COLOR_BACK);
        else text.writeString(x,y,"No ",Font_11x18,0x00C0FF,display.COLOR_BACK);
        
        y += Y_STEP;
        if (hcu.stateZone0) text.writeString(x,y,"Yes",Font_11x18,0xFF0F00,display.COLOR_BACK);
        else text.writeString(x,y,"No ",Font_11x18,0x00C0FF,display.COLOR_BACK);
        
        y += Y_STEP;
        if (hcu.stateFuelPump) text.writeString(x,y,"Yes",Font_11x18,0xFF0F00,display.COLOR_BACK);
        else text.writeString(x,y,"No ",Font_11x18,0x00C0FF,display.COLOR_BACK);
        
        y += Y_STEP;
        if (hcu.stateAch) text.writeString(x,y,"Yes",Font_11x18,0xFF0F00,display.COLOR_BACK);
        else text.writeString(x,y,"No ",Font_11x18,0x00C0FF,display.COLOR_BACK);
    }
}
//-----------------------------------------------------
void Setup::viewScreen13(uint8_t mode)   // настройки энергосбережения
{
    static int16_t posDomesticWater;
    static int16_t posSystem;
    int16_t result[2];
    
    const int SLIDER1_X = 35;
    const int SLIDER1_Y = 80;
    
    const int SLIDER2_X = 35;
    const int SLIDER2_Y = (SLIDER1_Y+80);
    
    if (mode){
        text.writeString(160-10*6.5,10,"Limitation of",Font_11x18,display.WHITE,display.COLOR_BACK);
        text.writeString(160-10*8,30,"heating duration",Font_11x18,display.WHITE,display.COLOR_BACK);
        this->isSlider1 = TRUE;
        //if (hcu.durationDomesticWater <= 90){
            posDomesticWater = (hcu.durationDomesticWater-30)*3.333;
        //}
        //else {
            //posDomesticWater = hcu.durationDomesticWater/6+50;
            //hcu.durationDomesticWater = (posDomesticWater-50)*6;
        //}
        //posDomesticWater = posDomesticWater - posDomesticWater%10;
        //if (posDomesticWater < 0) posDomesticWater = 0;
        //else if (posDomesticWater > 100) posDomesticWater = 100;
        
        this->isSlider2 = TRUE;
        if (hcu.durationSystem <= 360){
            posSystem = hcu.durationSystem/6-10;
        }
        else {
            posSystem = hcu.durationSystem/144+50;
            hcu.durationSystem = (posSystem-50)*144;
        }
        posSystem = posSystem - posSystem%10;
        if (posSystem < 0) posSystem = 0;
        else if (posSystem > 100) posSystem = 100;
    }
    
    result[0] = slider.drawSlim(SLIDER1_X, SLIDER1_Y, posDomesticWater, "Domestic water", hcu.durationDomesticWater, 'm', mode, &this->isSlider1, TRUE, SLIDER_COLOR_2, 5);
    
    if (hcu.durationSystem <= 360){
        if (isSlider2) canvas.writeFillRect(100,SLIDER2_Y,220,18,0);
        result[1] = slider.drawSlim(SLIDER2_X, SLIDER2_Y, posSystem, "System", hcu.durationSystem/60, 'h', mode, &this->isSlider2, true, SLIDER_COLOR_3, 6);
    }
    else if (posSystem < 100){
        if (isSlider2) canvas.writeFillRect(100,SLIDER2_Y,220,18,0);
        result[1] = slider.drawSlim(SLIDER2_X, SLIDER2_Y, posSystem, "System", hcu.durationSystem/60/24, 'd', mode, &this->isSlider2, true, SLIDER_COLOR_3, 6);
    }
    else{
        if (isSlider2) text.writeString(100,SLIDER2_Y,"- without time limit",Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
        result[1] = slider.drawSlim(SLIDER2_X, SLIDER2_Y, posSystem, "System", mode, &this->isSlider2, true, SLIDER_COLOR_3, 6);
    }
    
    if (result[0] == -1){
        posDomesticWater -= 33;
        if (posDomesticWater < 0) posDomesticWater = 0;
        hcu.durationDomesticWater = posDomesticWater/3.333+30.5;
        isSlider1 = TRUE;
    }
    else if(result[0] == -2){
        posDomesticWater += 33;
        if (posDomesticWater > 100) posDomesticWater = 100;
        hcu.durationDomesticWater = posDomesticWater/3.333+30.5;
        isSlider1 = TRUE;
    }
    else if (result[0] != -3){
        posDomesticWater = result[0]/2;
        posDomesticWater = posDomesticWater - posDomesticWater%33;
        if (posDomesticWater < 0) posDomesticWater = 0;
        if (posDomesticWater > 100) posDomesticWater = 100;
        hcu.durationDomesticWater = posDomesticWater/3.333+30.5;
        isSlider1 = TRUE;
    }
    
    if (result[1] == -1){
        posSystem -= 10;
        if (posSystem < 0) posSystem = 0;
        if (posSystem <= 50){
            hcu.durationSystem = (posSystem+10)*6;
        }
        else{
            hcu.durationSystem = (posSystem-50)*144;
        }
        isSlider2 = TRUE;
    }
    else if(result[1] == -2){
        posSystem += 10;
        if (posSystem > 100) posSystem = 100;
        if (posSystem <= 50){
            hcu.durationSystem = (posSystem+10)*6;
        }
        else{
            hcu.durationSystem = (posSystem-50)*144;
        }
        isSlider2 = TRUE;
    }
    else if (result[1] != -3){
        posSystem = result[1]/2;
        posSystem = posSystem - posSystem%10;
        if (posSystem < 0) posSystem = 0;
        if (posSystem > 100) posSystem = 100;
        if (posSystem <= 50){
            hcu.durationSystem = (posSystem+10)*6;
        }
        else{
            hcu.durationSystem = (posSystem-50)*144;
        }
        isSlider2 = TRUE;
    }
}
//-----------------------------------------------------
void Setup::viewScreen14(uint8_t mode)   // экран версий и моточасов
{
    uint8_t x, n;
    char str[32];
    static uint32_t timer=0;
    static uint32_t timeOld=0xFFFFFFFF;
    
    const int STROKE1_Y = 60;
    const int STROKE2_Y = STROKE1_Y + 40;
    const int STROKE3_Y = STROKE2_Y + 40;
    const int STROKE4_Y = STROKE3_Y + 40;
    
    if (mode){
        air.isPanelSensor |= 2;
        text.writeString(160-11*5.5,10,"System Info",Font_11x18,display.WHITE,display.COLOR_BACK);
    }
    
    if (((core.getTick()-timer)>1000)||mode){
        timer=core.getTick();
        
        if (mode) text.writeString(10,STROKE1_Y,"Heater SV:",Font_11x18,checkbox.COLOR_OFF,display.COLOR_BACK);
        if (hcu.isHeaterVersion || mode){
            hcu.isHeaterVersion=false;
            for (x=0; x<10; x++) str[x] = 0;
            n = 0;
            for (x=0; x<4; x++){
                n += text.decToString(&str[n], hcu.heaterVersion[x]);
                str[n++] = '.';
            }
            str[--n] = 0;
            text.writeString(10+10*11,STROKE1_Y,str,Font_11x18,checkbox.COLOR_ON,display.COLOR_BACK);
        }
        if (mode) text.writeString(10,STROKE2_Y,"Control Box SV:",Font_11x18,checkbox.COLOR_OFF,display.COLOR_BACK);
        if (hcu.isVersion || mode){
            hcu.isVersion=false;
            for (x=0; x<10; x++) str[x] = 0;
            n = 0;
            for (x=0; x<4; x++){
                n += text.decToString(&str[n], hcu.version[x]);
                str[n++] = '.';
            }
            str[--n] = 0;
            text.writeString(10+10*16,STROKE2_Y,str,Font_11x18,checkbox.COLOR_ON,display.COLOR_BACK);
        }
        if (mode) text.writeString(10,STROKE3_Y,"Panel SV:",Font_11x18,checkbox.COLOR_OFF,display.COLOR_BACK);
        if (mode){
            for (x=0; x<10; x++) str[x] = 0;
            n = 0;
            for (x=0; x<4; x++){
                n += text.decToString(&str[n], *(__IO uint8_t*)(CRC_FIRMWARE_ADDRESS+6+x));
                str[n++] = '.';
            }
            str[--n] = 0;
            text.writeString(10+10*10,STROKE3_Y,str,Font_11x18,checkbox.COLOR_ON,display.COLOR_BACK);
        }
        for (x=0; x<32; x++) str[x] = 0;
        if ((timeOld!=hcu.heaterTotalOperatingTime)||(mode)) {
            timeOld=hcu.heaterTotalOperatingTime;
            n = text.charToString(str, "Total Heater Hours: ");//Total Operating Time
            n += text.decToString(&str[n], hcu.heaterTotalOperatingTime/60);
            text.writeString(10,STROKE4_Y,str,Font_11x18,checkbox.COLOR_OFF,display.COLOR_BACK);
        }
    }
}
//-----------------------------------------------------
void Setup::viewScreen15(uint8_t mode)   // настройки системные
{
    const int CHECKBOXS_S = 20;
    
    const int CHECKBOX1_X = 35;
    const int CHECKBOX1_Y = 65;
    const int CHECKBOX1_H = 40;
    const int CHECKBOX1_W = 200;
    
    const int CHECKBOX2_X = CHECKBOX1_X;
    const int CHECKBOX2_Y = (CHECKBOX1_Y+40);
    const int CHECKBOX2_H = CHECKBOX1_H;
    const int CHECKBOX2_W = CHECKBOX1_W;
    
    const int CHECKBOX3_X = CHECKBOX1_X;
    const int CHECKBOX3_Y = (CHECKBOX2_Y+40);
    const int CHECKBOX3_H = CHECKBOX1_H;
    const int CHECKBOX3_W = CHECKBOX1_W;
    
    const int CHECKBOX4_X = CHECKBOX1_X;
    const int CHECKBOX4_Y = (CHECKBOX3_Y+40);
    const int CHECKBOX4_H = CHECKBOX1_H;
    const int CHECKBOX4_W = CHECKBOX1_W;
    
    if (mode){
        text.writeString(160-11*3,10,"System",Font_11x18,display.WHITE,display.COLOR_BACK);
        display.setup.celsius |= 2;
        display.setup.fahrenheit |= 2;
        display.setup.h12 |= 2;
        air.isPanelSensor |= 2;
    }
    checkbox.draw(CHECKBOX1_X, CHECKBOX1_Y, "Celsius", &display.setup.celsius);
    checkbox.draw(CHECKBOX2_X, CHECKBOX2_Y, "Fahrenheit", &display.setup.fahrenheit);
    
    checkbox.draw(CHECKBOX3_X, CHECKBOX3_Y, "12-hour clock", &display.setup.h12);
    
    checkbox.draw(CHECKBOX4_X, CHECKBOX4_Y, "Panel sensor", &air.isPanelSensor);
    
    if (sensor.status == 1){   // касание сенсорного экрана
        //sensor.touch = 1;
        if ((sensor.x1>=(CHECKBOX1_X-20) && sensor.x1<=(CHECKBOX1_X+CHECKBOX1_W) && sensor.y1>=(CHECKBOX1_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX1_Y+CHECKBOX1_H-CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 1
            sensor.touch = 1;
            display.setup.celsius = !display.setup.celsius+2;
            display.setup.fahrenheit = !display.setup.fahrenheit+2;
            if (display.setup.celsius & 0x01){
                air.changeScale(7, 10, 32, 7, BUTTON_CENTRAL_IMAGE, BUTTON_CENTRAL_IMAGE, 5, 5);
                hcu.airHeaterTSetPoint[0] = core.farToCel(hcu.airHeaterTSetPoint[0]);
                hcu.airHeaterTSetPoint[1] = core.farToCel(hcu.airHeaterTSetPoint[1]);
            }
            else{
                air.changeScale(45, 50, 90, 45, BUTTON_CENTRAL_IMAGE, BUTTON_CENTRAL_IMAGE, 5, 10);
                hcu.airHeaterTSetPoint[0] = core.celToFar(hcu.airHeaterTSetPoint[0]);
                hcu.airHeaterTSetPoint[1] = core.celToFar(hcu.airHeaterTSetPoint[1]);
            }
            this->isChange = true;
        }
        if ((sensor.x1>=(CHECKBOX2_X-20) && sensor.x1<=(CHECKBOX2_X+CHECKBOX2_W) && sensor.y1>=(CHECKBOX2_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX2_Y+CHECKBOX2_H-CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 2
            sensor.touch = 2;
            display.setup.fahrenheit = !display.setup.fahrenheit+2;
            display.setup.celsius = !display.setup.celsius+2;
            if (display.setup.celsius & 0x01){
                air.changeScale(7, 10, 32, 7, BUTTON_CENTRAL_IMAGE, BUTTON_CENTRAL_IMAGE, 5, 5);
                hcu.airHeaterTSetPoint[0] = core.farToCel(hcu.airHeaterTSetPoint[0]);
                hcu.airHeaterTSetPoint[1] = core.farToCel(hcu.airHeaterTSetPoint[1]);
            }
            else{
                air.changeScale(45, 50, 90, 45, BUTTON_CENTRAL_IMAGE, BUTTON_CENTRAL_IMAGE, 5, 10);
                hcu.airHeaterTSetPoint[0] = core.celToFar(hcu.airHeaterTSetPoint[0]);
                hcu.airHeaterTSetPoint[1] = core.celToFar(hcu.airHeaterTSetPoint[1]);
            }
            this->isChange = true;
        }
        if ((sensor.x1>=(CHECKBOX3_X-20) && sensor.x1<=(CHECKBOX3_X+CHECKBOX3_W) && sensor.y1>=(CHECKBOX3_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX3_Y+CHECKBOX3_H-CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 3
            sensor.touch = 3;
            display.setup.h12 = !display.setup.h12+2;
            this->isChange = true;
        }
        if ((sensor.x1>=(CHECKBOX4_X-20) && sensor.x1<=(CHECKBOX4_X+CHECKBOX4_W) && sensor.y1>=(CHECKBOX4_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX4_Y+CHECKBOX4_H-CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 4
            sensor.touch = 4;
            air.isPanelSensor = !air.isPanelSensor+2;
            this->isChange = true;
        } 
    }
}

void Setup::viewScreen16(uint8_t mode)   // настройки системные
{
    const int CHECKBOXS_S = 20;
    
    const int CHECKBOX1_X = 25;
    const int CHECKBOX1_Y = 65;
    const int CHECKBOX1_H = 40;
    const int CHECKBOX1_W = 200;
    
	  const int SLIDER1_X = 25;
    const int SLIDER1_Y = (CHECKBOX1_Y+50);
    
	int result;
	int posShift=(display.setup.tempShift+10)*5;
    if (mode){
        text.writeString(160-11*3,10,"Misc",Font_11x18,display.WHITE,display.COLOR_BACK);
        display.setup.scheduleMode |= 2;
				isSlider1 = true;
    }
		
		int temp;
		if (display.setup.celsius)
			temp = display.setup.tempShift;
		else
			temp = display.setup.tempShift*1.8;
    checkbox.draw(CHECKBOX1_X, CHECKBOX1_Y, "Schedule mode", &display.setup.scheduleMode);
		uint32_t color;
		if (display.setup.tempShift>0)
			color = SLIDER_COLOR_3;
		else if (display.setup.tempShift<0)
			color = SLIDER_COLOR_2;
		else color = SLIDER_COLOR_1;
		
		result = slider.drawSlim(SLIDER1_X, SLIDER1_Y, posShift, display.setup.celsius?"Temperature shift,~C":"Temperature shift,~F", temp ,'~', mode, &this->isSlider1, 1,color , 7);
    
    
    if (sensor.status == 1){   // касание сенсорного экрана
        //sensor.touch = 1;
        if ((sensor.x1>=(CHECKBOX1_X-20) && sensor.x1<=(CHECKBOX1_X+CHECKBOX1_W) && sensor.y1>=(CHECKBOX1_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX1_Y+CHECKBOX1_H-CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 1
            sensor.touch = 1;
            display.setup.scheduleMode = !display.setup.scheduleMode+2;
                hcu.airHeaterTSetPoint[0] = hcu.airHeaterTSetPoint[1]; //Equalizing setpoints. day one is master
            this->isChange = true;
        }
    }
		
		if (result == -1)
		{			
				posShift-=10;
				if (posShift<0) posShift=0;
				display.setup.tempShift=posShift/10-10;
				isSlider1=true;
		}
		else if (result == -2)
		{
				posShift+=10;
				if (posShift>200) posShift=200;
				display.setup.tempShift=posShift/10-10;
				isSlider1=true;
		}
			else if (result != -3){
				
				posShift=result;
				if (posShift>200) posShift=200;
				if (posShift<0) posShift=0;
				display.setup.tempShift = posShift/10-10;
				isSlider1=true;
		}
}
//-----------------------------------------------------
void Setup::viewScreen100(uint8_t mode)   // секретный экран
{
    uint8_t x, n;
    char str[32];
    static uint32_t timer=0;
    static float pressureOld = 0;
    static float voltageOld = 0;
   // static uint32_t faultLinkCounterOld = 0;
    static uint32_t reinitCounterOld = 0;
	static uint32_t HCUuptimeOld = 0;
	static uint32_t PanelUptimeOld = 0;
	static uint32_t PacketToHCUCounterOld = 0;
	static uint32_t PacketToPanelCounterOld = 0;
		static uint32_t RestartCounterOld = 0;
    
    if (((core.getTick()-timer)>1000)||mode){
        timer=core.getTick();
        
        text.writeString(160-11*5.5,10,"System Info",Font_11x18,display.WHITE,display.COLOR_BACK);
        
        for (x=0; x<32; x++) str[x] = 0;
        if ((pressureOld!=hcu.pressure)||(mode)) {
            pressureOld=hcu.pressure;
            n = text.charToString(str, "Pressure: ");
            n += text.floatToString(&str[n], hcu.pressure, 1);
            text.writeString(10,60,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
        }
        
        for (x=0; x<32; x++) str[x] = 0;
        if ((voltageOld!=hcu.voltage)||(mode)) {
            voltageOld=hcu.voltage;
            n = text.charToString(str, "Voltage: ");
            n += text.floatToString(&str[n], hcu.voltage, 1);
            text.writeString(10,75,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
        }
        /*
        for (x=0; x<32; x++) str[x] = 0;
        if ((faultLinkCounterOld!=hcu.faultedCommandCounter)||(mode)) {
            faultLinkCounterOld=hcu.faultedCommandCounter;
            n = text.charToString(str, "Fault Com Counter: ");
            n += text.decToString(&str[n], hcu.faultedCommandCounter);
            text.writeString(10,75,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
        }
        */
        for (x=0; x<32; x++) str[x] = 0;
        if ((reinitCounterOld!=hcu.reinitialisationCounter)||(mode)) {
            reinitCounterOld=hcu.reinitialisationCounter;
            n = text.charToString(str, "UART init cnt: ");
            n += text.decToString(&str[n], hcu.reinitialisationCounter);
            text.writeString(10,90,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
        }
		
		for (x=0; x<32; x++) str[x] = 0;
        if ((HCUuptimeOld!=hcu.uptime)||(mode)) {
            HCUuptimeOld=hcu.uptime;
            n = text.charToString(str, "HCU uptime: ");
            n += text.decToString(&str[n], hcu.uptime);
            text.writeString(10,105,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
        }
		
		for (x=0; x<32; x++) str[x] = 0;
        if ((PanelUptimeOld!=(core.getTick()/60000))||(mode)) {
            PanelUptimeOld=(core.getTick()/60000);
            n = text.charToString(str, "Panel uptime: ");
            n += text.decToString(&str[n], (core.getTick()/60000));
            text.writeString(10,120,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
        }
		for (x=0; x<32; x++) str[x] = 0;
        if ((PacketToHCUCounterOld!=hcu.ReceivedByHCUPacketCounter)||(mode)) {
            PacketToHCUCounterOld=hcu.ReceivedByHCUPacketCounter;
            n = text.charToString(str, "PU->HCU: ");
            n += text.decToString(&str[n], hcu.ReceivedByHCUPacketCounter);
	
            text.writeString(10,135,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
				}
				
					for (x=0; x<32; x++) str[x] = 0;
				if (PacketToPanelCounterOld!=hcu.ReceivedByPanelPacketCounter||(mode))
				{
					PacketToPanelCounterOld=hcu.ReceivedByPanelPacketCounter;
				    n = text.charToString(str, "HCU->PU: ");
            n += text.decToString(&str[n], hcu.ReceivedByPanelPacketCounter);
						text.writeString(10,150,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
				}
				
						for (x=0; x<32; x++) str[x] = 0;
								if (RestartCounterOld!=hcu.restartCounter||(mode))
				{
					RestartCounterOld=hcu.restartCounter;
				    n = text.charToString(str, "HCU restart: ");
            n += text.decToString(&str[n], hcu.restartCounter);
						text.writeString(10,165,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
				}

    }
}
