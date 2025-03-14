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
#include "backup.h"
#include "stdio.h"

Setup setup;
//-----------------------------------------------------
Setup::Setup(void)
{
	core.ClassInit(this,sizeof(this));
    ///readSetup();
    numberScreen = 1;
    isChange = true;
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
    
    viewPage(numberScreen, 1);
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
                numberScreen++;
                mode = 1;
                if (numberScreen > maxNumberScreen) numberScreen = 1;
                viewScreenChange();
            }
        }
        if (slider.touch==0 && 
            sensor.x1 <= startTimer.EXIT_SIZE && 
            sensor.y1 <= startTimer.EXIT_SIZE){  // касание кнопки выхода из настроек
            sensor.touch = 2;
            if (sensor.isTouchElement == FALSE){
                isChange = true;
                sensor.isTouchElement = TRUE;
                result = 1;
            }
        }
        if (slider.touch==0 && 
            sensor.x1>(BUTTON_RIGHT_X) && sensor.x1<(BUTTON_RIGHT_X+BUTTON_RIGHT_SIZE) && 
            sensor.y1>(BUTTON_RIGHT_Y) && sensor.y1<(BUTTON_RIGHT_Y+BUTTON_RIGHT_SIZE)){  // касание кнопки секретного меню
            //sensor.touch = 0;
                if (numberScreen == 6){
                    if (sensor.isTouchElement == FALSE){
                    sensor.isTouchElement = TRUE;
                    numberScreen = 100;
                    mode = 1;
                    //if (numberScreen > maxNumberScreen) numberScreen = 1;
                    viewScreenChange();
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
    if (result == 0) result = viewPage(numberScreen, mode);
    
    if (sensor.touch != 0){
        hcu.lockTimer = core.getTick();
    }
    
    return result;
}
//-----------------------------------------------------
uint8_t Setup::viewPage(uint8_t number, uint8_t mode)
{
    uint8_t result = 0;
    
    maxNumberScreen = 8;
    if (numberScreen == 100){
        viewScreen100(mode); // секретный экран
    }
    else{
        if (numberScreen > maxNumberScreen){
            numberScreen = maxNumberScreen;
            number = numberScreen;
        }
        if (number == 1) viewScreen1(mode);
        if (number == 2) viewScreen10(mode);
        if (number == 3) result = viewScreen11(mode);
        if (number == 4) viewScreen12(mode);
        if (number == 5) viewScreen13(mode);
        if (number == 6) viewScreen14(mode); //экран версий и моточасов
        if (number == 7) viewScreen15(mode);
		if (number == 8) viewScreen16(mode); //Schedule mode screen + temp shift
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
        isSlider1 = TRUE;
        display.setup.displaySleep |= 2;
        display.setup.viewClock |= 2;
        isSlider2 = TRUE;
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
    result[0] = slider.drawSlim(SLIDER1_X, SLIDER1_Y, display.setup.brightness, "Brightness", display.setup.brightness, NULL, mode, &isSlider1, TRUE, SLIDER_COLOR_3, 1);
    display.setLight(display.setup.brightness);
    checkbox.draw(CHECKBOX2_X, CHECKBOX2_Y, "Sleep mode", &display.setup.displaySleep);
    if (display.setup.timeout <= 90){
        unit = 's';
        result[1] = slider.drawSlim(SLIDER2_X, SLIDER2_Y, posTimeout, "Timeout", display.setup.timeout, unit, mode, &isSlider2, TRUE, SLIDER_COLOR_2, 2);
    }
    else {
        unit = 'm';
        result[1] = slider.drawSlim(SLIDER2_X, SLIDER2_Y, posTimeout, "Timeout", display.setup.timeout/60, unit, mode, &isSlider2, TRUE, SLIDER_COLOR_2, 2);
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
        isSlider1 = TRUE;
        hcu.pumpOn |= 2;
    }
    if ((hcu.fanManual & 0x02)==0x02) isSlider1 = TRUE;
    checkbox.draw(CHECKBOX1_X, CHECKBOX1_Y, "Fan auto mode", &hcu.fanAuto);
    checkbox.draw(CHECKBOX2_X, CHECKBOX2_Y, "Fan manual mode", &hcu.fanManual);
    //slider.draw(SLIDER1_X, SLIDER1_Y, hcu.fanPower, "Fan power", hcu.fanPower, '%', mode, &isSlider1, hcu.fanManual==TRUE);
    result = slider.drawSlim(SLIDER1_X, SLIDER1_Y, hcu.fanPower, "Fan power", hcu.fanPower, '%', mode, &isSlider1, hcu.fanManual==TRUE, SLIDER_COLOR_3, 4);
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
        isSlider1 = TRUE;
    }
    
    if (isSlider1 == TRUE){
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
    slider.drawDouble(SLIDER1_X, SLIDER1_Y, z1, z2, "Daytime", mode, &isSlider1, numSelectSlider);
    
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
            isSlider1 = TRUE;
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
            isSlider1 = TRUE;
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
            isSlider1 = TRUE;
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
            isSlider1 = TRUE;
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
        isSlider1 = TRUE;
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
        
        isSlider2 = TRUE;
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
    
    result[0] = slider.drawSlim(SLIDER1_X, SLIDER1_Y, posDomesticWater, "Domestic water", hcu.durationDomesticWater, 'm', mode, &isSlider1, TRUE, SLIDER_COLOR_2, 5);
    
    if (hcu.durationSystem <= 360){
        if (isSlider2) canvas.writeFillRect(100,SLIDER2_Y,220,18,0);
        result[1] = slider.drawSlim(SLIDER2_X, SLIDER2_Y, posSystem, "System", hcu.durationSystem/60, 'h', mode, &isSlider2, true, SLIDER_COLOR_3, 6);
    }
    else if (posSystem < 100){
        if (isSlider2) canvas.writeFillRect(100,SLIDER2_Y,220,18,0);
        result[1] = slider.drawSlim(SLIDER2_X, SLIDER2_Y, posSystem, "System", hcu.durationSystem/60/24, 'd', mode, &isSlider2, true, SLIDER_COLOR_3, 6);
    }
    else{
        if (isSlider2) text.writeString(100,SLIDER2_Y,"- without time limit",Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
        result[1] = slider.drawSlim(SLIDER2_X, SLIDER2_Y, posSystem, "System", mode, &isSlider2, true, SLIDER_COLOR_3, 6);
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
            isChange = true;
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
            isChange = true;
        }
        if ((sensor.x1>=(CHECKBOX3_X-20) && sensor.x1<=(CHECKBOX3_X+CHECKBOX3_W) && sensor.y1>=(CHECKBOX3_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX3_Y+CHECKBOX3_H-CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 3
            sensor.touch = 3;
            display.setup.h12 = !display.setup.h12+2;
            isChange = true;
        }
        if ((sensor.x1>=(CHECKBOX4_X-20) && sensor.x1<=(CHECKBOX4_X+CHECKBOX4_W) && sensor.y1>=(CHECKBOX4_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX4_Y+CHECKBOX4_H-CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 4
            sensor.touch = 4;
            air.isPanelSensor = !air.isPanelSensor+2;
            isChange = true;
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
		
		result = slider.drawSlim(SLIDER1_X, SLIDER1_Y, posShift, display.setup.celsius?"Temperature shift,~C":"Temperature shift,~F", temp ,'~', mode, &isSlider1, 1,color , 7);
    
    
    if (sensor.status == 1){   // касание сенсорного экрана
        //sensor.touch = 1;
        if ((sensor.x1>=(CHECKBOX1_X-20) && sensor.x1<=(CHECKBOX1_X+CHECKBOX1_W) && sensor.y1>=(CHECKBOX1_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX1_Y+CHECKBOX1_H-CHECKBOXS_S) && sensor.touch==0)){ // касание чекбокса 1
            sensor.touch = 1;
            display.setup.scheduleMode = !display.setup.scheduleMode+2;
                hcu.airHeaterTSetPoint[0] = hcu.airHeaterTSetPoint[1]; //Equalizing setpoints. day one is master
            isChange = true;
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
    //uint8_t x, n;
    char str[64];
    static uint32_t timer=0;
    static float pressureOld = 0;
    static float voltageOld = 0;
   // static uint32_t faultLinkCounterOld = 0;
    static uint32_t reinitCounterOld = 0;
	static uint32_t HCUuptimeOld = 0;
	static uint32_t PanelUptimeOld = 0;
	static uint32_t PacketToHCUCounterOld = 0;
	static uint32_t PacketToPanelCounterOld = 0;
	static uint32_t HcuRestartCounterOld = 0;
	static uint32_t PanelRestartCounterOld = 0;
	static uint32_t Code14CounterOld = 0;
	static uint16_t codeHistoryOld[5] = {0,};
    
    if (((core.getTick()-timer)>1000)||mode){
        timer=core.getTick();
        
        text.writeString(160-11*5.5,10,"System Info",Font_11x18,display.WHITE,display.COLOR_BACK);
        
        if ((pressureOld!=hcu.pressure)||(mode)) {
            pressureOld=hcu.pressure;
			sprintf(str,"Pressure: %.3f kPa",hcu.pressure);
            text.writeString(10,60,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
        }
        
        if ((voltageOld!=hcu.voltage)||(mode)) {
            voltageOld=hcu.voltage;
			sprintf(str,"Voltage: %.1f",hcu.voltage);
            text.writeString(10,75,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
        }
        
        if ((reinitCounterOld!=hcu.reinitialisationCounter)||(mode)) {
            reinitCounterOld=hcu.reinitialisationCounter;
			sprintf(str,"UART init cnt: %d",hcu.reinitialisationCounter);
            text.writeString(10,90,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
        }
		
		
        if ((HCUuptimeOld!=hcu.uptime)||(mode)) {
            HCUuptimeOld=hcu.uptime;
			sprintf(str,"HCU uptime: %05d min",hcu.uptime);
            text.writeString(10,105,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
        }
		
		
        if ((PanelUptimeOld!=(core.getTick()/60000))||(mode)) {
            PanelUptimeOld=(core.getTick()/60000);
			sprintf(str,"Panel uptime: %05d min",core.getTick()/60000);
            text.writeString(10,120,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
        }
		
        if ((PacketToHCUCounterOld!=hcu.ReceivedByHCUPacketCounter)||(mode)) {
            PacketToHCUCounterOld=hcu.ReceivedByHCUPacketCounter;
			sprintf(str,"PU->HCU: %05d",hcu.ReceivedByHCUPacketCounter);
            text.writeString(10,135,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
				}
				
			if (PacketToPanelCounterOld!=hcu.ReceivedByPanelPacketCounter||(mode))
				{
					PacketToPanelCounterOld=hcu.ReceivedByPanelPacketCounter;
					sprintf(str,"HCU->PU: %05d",hcu.ReceivedByPanelPacketCounter);
						text.writeString(10,150,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
				}
				
						
			if (HcuRestartCounterOld!=hcu.restartCounter||(mode))
				{
					HcuRestartCounterOld=hcu.restartCounter;
					sprintf(str,"HCU restart: %05d",hcu.restartCounter);
						text.writeString(10,165,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
				}
				
			if (PanelRestartCounterOld!=*backup.reloadCounter||(mode))
				{
					PanelRestartCounterOld=*backup.reloadCounter;
					sprintf(str,"Panel restart: %05d",*backup.reloadCounter);
					text.writeString(10,180,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
				}
				
			bool historyChanged = 	*backup.lastErrors1!=codeHistoryOld[0] || 
									*backup.lastErrors2!=codeHistoryOld[1] || 
									*backup.lastErrors3!=codeHistoryOld[2] || 
									*backup.lastErrors4!=codeHistoryOld[3] || 
									*backup.lastErrors5!=codeHistoryOld[4];
		
			codeHistoryOld[0]=*backup.lastErrors1;
			codeHistoryOld[1]=*backup.lastErrors2;
			codeHistoryOld[2]=*backup.lastErrors3;
			codeHistoryOld[3]=*backup.lastErrors4;
			codeHistoryOld[4]=*backup.lastErrors5;
					
			if (mode || historyChanged)
				{
				    sprintf(str,"Error log: %02d-%02d-%02d-%02d-%02d",*backup.lastErrors1,*backup.lastErrors2,*backup.lastErrors3,*backup.lastErrors4,*backup.lastErrors5);
					text.writeString(10,195,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
				}
				
			if (Code14CounterOld!=hcu.code14Counter||(mode))
				{
					Code14CounterOld=hcu.code14Counter;
					sprintf(str,"Code 14 cnt: %d/%d",hcu.code14Counter,hcu.Code14CounterTotal);
					text.writeString(10,210,str,Font_7x10,checkbox.COLOR_OFF,display.COLOR_BACK);
				}

    }
}
//------------------------------------------------------------
void Setup::UpdateFanAutoGraph(void)   // 
{
	if ((screen_visible == SCREEN_VISIBLE_SETUP)&&(numberScreen == 2))
	{
		viewScreen10(1);   // настройки отопителя салона и помпы
//		hcu.fanAuto |= 2;
//		hcu.fanManual |= 2;
	}
}
//-------------------------------------------------------------
void Setup::UpdateTimeSetGraph(void)   // 
{
	if ((screen_visible == SCREEN_VISIBLE_SETUP)&&(numberScreen == 5))
	{
		viewScreen13(1);
	}
}
//-------------------------------------------------------------
void Setup::UpdateDayTimeGraph(void)   // 
{
	if ((screen_visible == SCREEN_VISIBLE_SETUP)&&(numberScreen == 3))
	{
		viewScreen11(1);
//		isSlider1 = TRUE;
	}
}
void Setup::UpdateScheduleModeGraph(void)   // 
{
	if ((screen_visible == SCREEN_VISIBLE_SETUP)&&(numberScreen == 8))
	{
		viewScreen16(1);   // настройки системные
//		display.setup.scheduleMode |= 2;
	}
}
void Setup::UpdateCelsiusGraph(void)   // 
{
	if ((screen_visible == SCREEN_VISIBLE_SETUP)&&(numberScreen == 7))
	{
		viewScreen15(1);   // настройки системные
	}
}

//-------------------------------------------------------------
