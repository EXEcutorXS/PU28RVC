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
#include "work.h"
#include "setup.h"      // исправить, нарушение иерархии классов!!!
// Objects
#include "heater.h"
#include "start_timer.h"
#include "slider.h"
#include "checkbox.h"
#include "clock.h"
// Drivers
#include "temperature.h"
#include "core.h"
#include "usart.h"
#include "can.h"
#include "bluetooth.h"
#include "memory.h"
#include "display.h"
#include "sensor.h"
#include "canvas.h"
#include "text.h"
// Library
#include <math.h>

Work work;
//-----------------------------------------------------
Work::Work(void)
{
    isFHeaterOld = !isFHeater;
    isEHeaterOld = !isEHeater;
}
//-----------------------------------------------------
void Work::viewScreen(void)
{
    canvas.writeFillRect(0,0,320,240,display.COLOR_BACK);                               // вывод заднего фона
    display.switchOn();                                             // включение дисплея//slider.viewScreen();
    
    heater.errorOld = 0;
    heater.stageOld = 0xFF;
    temperature.oldValue = 127;
    
    canvas.loadImageEffect(275,5,BUTTON_SETUP_IMAGE,BUTTON_SETUP_STEP,1);               // отображение значка настроек
    if (startTimer.isOn == 1){
        if (!startTimer.isUnlimited){
            canvas.loadImageEffect(5,5,BUTTON_TIMER_ON_IMAGE,BUTTON_TIMER_STEP,1);      // отображение значка активных таймеров
        }
        else {
            canvas.loadImageEffect(5,5,BUTTON_TIMER_RED_IMAGE,BUTTON_TIMER_STEP,1);     // отображение значка опасно активных таймеров
        }
    }
    else{
        canvas.loadImageEffect(5,5,BUTTON_TIMER_OFF_IMAGE,BUTTON_TIMER_STEP,1);         // отображение значка отключенных таймеров
    }
    
    canvas.loadImageEffect(BUTTON_LEFT_X,BUTTON_LEFT_Y,ARROW_LEFT_IMAGE,BUTTON_LEFT_STEP,1);      // отображение значка стрелки влево
    canvas.loadImageEffect(BUTTON_RIGHT_X,BUTTON_RIGHT_Y,ARROW_RIGHT_IMAGE,BUTTON_RIGHT_STEP,1);      // отображение значка срелки вправо
    
    isFHeaterOld = !isFHeater;
    isEHeaterOld = !isEHeater;
}
//-----------------------------------------------------
uint8_t Work::viewHandler(void)
{
    uint8_t result;
    
    result = this->sensorCheck();   // обработка касания сенсорного экрана
    this->viewMode();               // вывод на экран режима работы отопителя и электронагревателя
    startTimer.viewButton();        // отображение кнопки таймеров
    this->viewDigClock();           // отображение времени
    return result;
    //this->viewVoltage();          // отображение напряжения
    //this->viewTemperature();      // отображение температуры
    //this->buttonCheck(0);         // обработка нажатия механической кнопки
}
//-----------------------------------------------------
void Work::viewDigClock(void)
{
    //static uint8_t _heater_mode=0;
    uint8_t h, m;
    
    //if (heater.stage != _heater_mode){
    //    _heater_mode = heater.stage;
    //    if (heater.stage == heater.MODE_WAIT){
    //        clock.mode = clock.MODE_TIME;
    //    }
    //    else{
    //        clock.mode = clock.MODE_WORKTIME;
    //    }
    //}
    
    if (clock.mode == clock.MODE_TIME || heater.workTime==0xFFFF){
        // отображение часов
        clock.viewDigClock(clock.hour, clock.minute);
    }
    else if (clock.mode == clock.MODE_WORKTIME){
        // отображение таймера работы изделия
        h = heater.workTime/60;
        m = heater.workTime%60;
        clock.viewDigClock(h, m);
    }
}
//-----------------------------------------------------
/*void Work::viewVoltage(void)
{
    uint8_t x, n;
    char str[10];
    
    if (heater.voltage > 6.0){
        for (x=0; x<10; x++) str[x] = 0;
        n = 0;
        if (heater.voltage < 10.0) str[n++] = ' ';
        n += text.floatToString(&str[n], heater.voltage, 1);
        str[n++] = 'v';
        text.writeString(320-(Font_16x26.width*n)-6,240-Font_16x26.height,str,Font_16x26,display.COLOR_TEXT,display.COLOR_BACK);
    }
    else{
        text.writeString(320-(Font_16x26.width*5)-6,240-Font_16x26.height,"--.-v",Font_16x26,display.COLOR_TEXT,display.COLOR_BACK);
    }
}
*/
//-----------------------------------------------------
void Work::viewMode(void)
{
    if (isFHeaterOld != isFHeater){
        isFHeaterOld = isFHeater;
        isChange = true;
        if (isFHeater == true){
            canvas.loadImageEffect(BUTTON_FHEAT_X,BUTTON_FHEAT_Y,BLUE_FHEAT_IMAGE,BUTTON_FHEAT_STEP,0);      // отображение значка отопителя
        }
        else{
            canvas.loadImageEffect(BUTTON_FHEAT_X,BUTTON_FHEAT_Y,BLACK_FHEAT_IMAGE,BUTTON_FHEAT_STEP,0);      // отображение значка отопителя
        }
    }
    if (isEHeaterOld != isEHeater){
        isEHeaterOld = isEHeater;
        isChange = true;
        if (isEHeater == true){
            canvas.loadImageEffect(BUTTON_EHEAT_X,BUTTON_EHEAT_Y,BLUE_EHEAT_IMAGE,BUTTON_EHEAT_STEP,0);      // отображение значка электронагревателя
        }
        else{
            canvas.loadImageEffect(BUTTON_EHEAT_X,BUTTON_EHEAT_Y,BLACK_EHEAT_IMAGE,BUTTON_EHEAT_STEP,0);      // отображение значка электронагревателя
        }
    }
}
//-----------------------------------------------------
/*void Work::viewTemperature(void)
{
    uint8_t x, n, indent;
    char str[7];
    signed char val;
    static signed t;
    
    if (this->temperature_source == this->TEMPERATURE_SOURCE_LIQUID) val = heater.temperature;                   // отображение значка источника температуры
    else if (this->temperature_source == this->TEMPERATURE_SOURCE_AIR) val =  heater.temperature;                // отображение значка источника температуры
    else if (this->temperature_source == this->TEMPERATURE_SOURCE_PANEL) val = temperature.getValue();           // отображение значка источника температуры
    if (temperature.oldValue != val){
        temperature.oldValue = val;
        if (val <= (-100) || (this->temperature_source == this->TEMPERATURE_SOURCE_PANEL && val == (-40))){
            canvas.writeFillRect(TEMPERATURE_ICON_X+30-8,188,60,26,display.COLOR_BACK);   // COLOR_BACK
            text.writeString(160-16+8,192,"--",Font_16x26,display.COLOR_TEMP,display.COLOR_BACK);
        }
        else{
            t = temperature.oldValue;
            n=0;
            if (val >= 0 && val <= 99){
                indent = 16;
            }
            else if (val > 99){
                indent = 8;
            }
            else{
                indent = 0;
            }
            for (x=0; x<6; x++) str[x] = 0;
            if (temperature.oldValue < 0){
                t = -temperature.oldValue;
                str[n] = '-';
                n += 1;
            }
            n += text.decToString(&str[n], t);
            //str[n++] = '~';
            //str[n++] = 'C';
            n++;
            //text.writeString(160+4-(Font_16x26.width*5/2),192,"    ",Font_16x26,display.COLOR_TEMP,display.COLOR_BACK);
            canvas.writeFillRect(TEMPERATURE_ICON_X+30-16,188,60,26,display.COLOR_BACK);   // COLOR_BACK
            text.writeString(160-16+indent,192,str,Font_16x26,display.COLOR_TEMP,display.COLOR_BACK);
        }
        temperature.sourceOld = NULL;
    }
    if (this->temperature_source != temperature.sourceOld){
        temperature.sourceOld = this->temperature_source;
        canvas.writeFillRect(120,188,30,30,display.BLACK);
        if (this->temperature_source == this->TEMPERATURE_SOURCE_LIQUID) canvas.loadImageEffect(TEMPERATURE_ICON_X,TEMPERATURE_ICON_Y-1,TEMPERATURE_FLUID_IMAGE,0,0);                   // отображение значка источника температуры
        else if (this->temperature_source == this->TEMPERATURE_SOURCE_AIR) canvas.loadImageEffect(TEMPERATURE_ICON_X,TEMPERATURE_ICON_Y+1,TEMPERATURE_AIR_IMAGE,0,0);              // отображение значка источника температуры
        else if (this->temperature_source == this->TEMPERATURE_SOURCE_PANEL) canvas.loadImageEffect(TEMPERATURE_ICON_X,TEMPERATURE_ICON_Y+1,TEMPERATURE_PANEL_IMAGE,0,0);              // отображение значка источника температуры
    }
}
*/
//-----------------------------------------------------
uint8_t Work::sensorCheck(void)
{
    uint8_t result = 0;
    
    if ((sensor.status == 1 || numbTouchElement>0) && sensor.touch == 0){   // касание сенсорного экрана
        sensor.touch = 1;
        
        if (1){
            if (slider.touch==0 && numbTouchElement != 1 && 
                sensor.x1>(BUTTON_FHEAT_X) && sensor.x1<(BUTTON_FHEAT_X+BUTTON_FHEAT_SIZE) && 
                sensor.y1>(BUTTON_FHEAT_Y) && sensor.y1<(BUTTON_FHEAT_Y+BUTTON_FHEAT_SIZE)){   // касание кнопки отопителя
                sensor.touch = 0;
                numbTouchElement = 1;
                isFHeater = !isFHeater;
            }
            else if (slider.touch==0 && numbTouchElement != 2 && 
                sensor.x1>(BUTTON_EHEAT_X) && sensor.x1<(BUTTON_EHEAT_X+BUTTON_EHEAT_SIZE) && 
                sensor.y1>(BUTTON_EHEAT_Y) && sensor.y1<(BUTTON_EHEAT_Y+BUTTON_EHEAT_SIZE)){  // касание кнопки электронагревателя
                sensor.touch = 0;
                numbTouchElement = 2;
                isEHeater = !isEHeater;
            }
            else numbTouchElement = 0;
            if (slider.touch==0 && 
                sensor.x1>(BUTTON_TIMER_X) && sensor.x1<(BUTTON_TIMER_X+BUTTON_TIMER_SIZE) && 
                sensor.y1>(BUTTON_TIMER_Y) && sensor.y1<(BUTTON_TIMER_Y+BUTTON_TIMER_SIZE)){  // касание кнопки таймеров
                sensor.touch = 0;
                if (sensor.isTouchElement == FALSE){
                    sensor.isTouchElement = TRUE;
                    sensor.resetTickTouch();
                }
                else if (sensor.getTickTouch() >= 500){
                    result = 1;
                }
            }
            if (slider.touch==0 && 
                sensor.x1>(BUTTON_UP_RIGHT_X) && sensor.x1<(BUTTON_UP_RIGHT_X+BUTTON_UP_RIGHT_SIZE) && 
                sensor.y1>(BUTTON_UP_RIGHT_Y) && sensor.y1<(BUTTON_UP_RIGHT_Y+BUTTON_UP_RIGHT_SIZE)){  // касание кнопки настроек
                sensor.touch = 0;
                if (sensor.isTouchElement == FALSE){
                    sensor.isTouchElement = TRUE;
                    result = 2;
                }
            }
            else if (slider.touch==0 && 
                sensor.x1>(0) && sensor.x1<(BUTTON_LEFT_X+BUTTON_LEFT_SIZE) && 
                sensor.y1>(BUTTON_LEFT_Y-10) && sensor.y1<(240)){  // касание кнопки влево
                sensor.touch = 0;
                if (sensor.isTouchElement == FALSE){
                    sensor.isTouchElement = TRUE;
                    result = 3;
                }
            }
            else if (slider.touch==0 && 
                sensor.x1>(BUTTON_RIGHT_X-10) && sensor.x1<(320) && 
                sensor.y1>(BUTTON_RIGHT_Y-10) && sensor.y1<(240)){  // касание кнопки вправо
                sensor.touch = 0;
                if (sensor.isTouchElement == FALSE){
                    sensor.isTouchElement = TRUE;
                    result = 4;
                }
            }
            else if (slider.touch==0 && 
                sensor.x1>(DIG_CLOCK_X) && sensor.x1<(DIG_CLOCK_X+DIG_CLOCK_X_SIZE) && 
                sensor.y1>(DIG_CLOCK_Y) && sensor.y1<(DIG_CLOCK_Y+DIG_CLOCK_Y_SIZE)){  // касание времени
                sensor.touch = 0;
                if (sensor.isTouchElement == FALSE){
                    sensor.isTouchElement = TRUE;
                    sensor.resetTickTouch();
                }
                else if (sensor.getTickTouch() >= 500){
                    result = 5;
                }
            }
        }
        
    }
    else if (sensor.status == 0){   // отпускание сенсорного экрана
        if (sensor.isTouchElement == TRUE){
            if (sensor.x1>(BUTTON_UP_LEFT_X) && sensor.x1<(BUTTON_UP_LEFT_X+BUTTON_UP_LEFT_SIZE) && 
                sensor.y1>(BUTTON_UP_LEFT_Y) && sensor.y1<(BUTTON_UP_LEFT_Y+BUTTON_UP_LEFT_SIZE)){  // отпускание кнопки таймеров
                if (sensor.getTickTouch() < 500){
                    if (startTimer.isOn == 0 && (startTimer.on[0]==1 || startTimer.on[1]==1 || startTimer.on[2]==1)){
                        startTimer.isOn = 1;
                        if (!startTimer.isUnlimited){
                            canvas.loadImageEffect(5,5,BUTTON_TIMER_ON_IMAGE,BUTTON_TIMER_STEP,1);                       // отображение значка таймеров
                        }
                        else {
                            canvas.loadImageEffect(5,5,BUTTON_TIMER_RED_IMAGE,BUTTON_TIMER_STEP,1);                       // отображение значка таймеров
                        }
                    }
                    else{
                        startTimer.isOn = 0;
                        canvas.loadImageEffect(5,5,BUTTON_TIMER_OFF_IMAGE,BUTTON_TIMER_STEP,1);                       // отображение значка таймеров
                    }
                    startTimer.isChange = true;
                }
            }
            if (sensor.x1>(DIG_CLOCK_X) && sensor.x1<(DIG_CLOCK_X+DIG_CLOCK_X_SIZE) && 
                sensor.y1>(DIG_CLOCK_Y) && sensor.y1<(DIG_CLOCK_Y+DIG_CLOCK_Y_SIZE)){  // отпускание времени
                if (sensor.getTickTouch() < 500){
                    //if (clock.mode == clock.MODE_TIME){
                    //    clock.mode = clock.MODE_WORKTIME;
                    //}
                    //else{
                    //    clock.mode = clock.MODE_TIME;
                    //}
                }
            }
        }
        
        sensor.touch = 0;
        slider.touch = 0;
        slider.mode = 0;
        core.setTimer(0);
        sensor.isTouchElement = FALSE;
    }
    return result;
}
//-----------------------------------------------------
/*void Work::buttonPress(void)
{
    if (heater.isConnect==heater.CONNECT_STATUS_OK && core.getTimer()==0){
        if (heater.error!=0 && heater.isError==0){
            heater.stageOld = 0;
            heater.isError = 1;
        }
        else{
            heater.isError = 0;
            heater.errorOld = 0;
            heater.error = 0;
            if (heater.stage == heater.MODE_WAIT){
                heater.stage = heater.MODE_START;
                core.setTimer(800);
                heater.workTime = 0;
                heater.run(0xFFFF,slider.values[slider.positionTemp]);                       
            }
            else{
                heater.stage = heater.MODE_STOP;
                core.setTimer(800);
                heater.stop();
            }
        }
                    
    }
    else if (heater.isConnect==heater.CONNECT_STATUS_ERROR){
        usart.linkCnt = 1;
        heater.isConnect = heater.CONNECT_STATUS_SEARCH;
        canvas.writeFillRect(104, 74, 112, 112, display.COLOR_BACK);
        heater.error = 0;
    }
}
*/
//-----------------------------------------------------
/*void Work::buttonCheck(uint8_t mode)
{
    if ((sensor.button==1 && sensor.status==0) || 
        (sensor.button==1 && sensor.status==1 && sensor.touch<2 && 
         sensor.x1>120 && sensor.x1<200 && sensor.y1>90 && sensor.y1<170)){
        core.delayMs(1); 
        
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) != Bit_RESET){     // нажатие механической кнопки
            sensor.button = 0;
            if (!mode)this->buttonPress();
        }
        else sensor.button = 0;
    }
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) != Bit_RESET){     // нажатие механической кнопки
        display.setTimer(display.setup.timeout*1000);
        if (sensor.button==0 && sensor.status==0){
            if (!mode)this->buttonPress();
        }
    }
}
*/
//-----------------------------------------------------
/*bool Work::checkHeaterSetup(void)
{
    uint32_t i;
    bool result = false;
    
    if (slider.mode == 0 && slider.touch == 0){             // замену шкалы производим только при отпущенном слайдере
        if (heater.isPreheater){                            // ЖИДКОСТНЫЙ ПОДОГРЕВАТЕЛЬ
            if (heater.isWarmingUp == TRUE){                // режим догревателя:
                if (slider.values[slider.position] != heater.setup.warmUpTSetpoint){
                    for (i=0; i<slider.VALUE_LEN; i++){                // ставим ползунок по настройке
                        if (slider.values[i] == heater.setup.warmUpTSetpoint) break;
                    }
                    if (i < slider.VALUE_LEN) slider.position = i;
                    else slider.position = 0;
                    result = true;
                }
            }
            else{                                           // режим подогревателя:
                if (slider.values[slider.position] != heater.setup.preheaterTSetpoint){
                    for (i=0; i<slider.VALUE_LEN; i++){                // ставим ползунок по настройке
                        if (slider.values[i] == heater.setup.preheaterTSetpoint) break;
                    }
                    if (i < slider.VALUE_LEN) slider.position = i;
                    else slider.position = 0;
                    slider.positionTempOld = 0;
                    slider.viewGrid(slider.position,BACKGROUND_IMAGE);    // красная шкала
                    result = true;
                }
            }
        }
        else{                                               // ВОЗДУШНЫЙ ОТОПИТЕЛЬ
            if (heater.setup.workBy == 3){                  // по мощности:
                if (slider.values[slider.position] != heater.setup.airHeaterPowerSetPoint){
                    for (i=0; i<slider.VALUE_LEN; i++){                // ставим ползунок по настройке
                        if (slider.values[i] == heater.setup.airHeaterPowerSetPoint) break;
                    }
                    if (i < slider.VALUE_LEN) slider.position = i;
                    else slider.position = 0;
                    result = true;
                }
            }
            else{                                           // по температуре:
                if (slider.values[slider.position] != heater.setup.airHeaterTSetPoint){
                    for (i=0; i<slider.VALUE_LEN; i++){                // ставим ползунок по настройке
                        if (slider.values[i] == heater.setup.airHeaterTSetPoint) break;
                    }
                    if (i < slider.VALUE_LEN) slider.position = i;
                    else slider.position = 0;
                    result = true;
                }
            }
        }
    }
    return result;
}
*/
//-----------------------------------------------------
