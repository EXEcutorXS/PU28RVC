/******************************************************************************
* ООО Теплостар
* Самара
* 
* Программисты: Клюев А.А.
* 
* 07.05.2019
* Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
// Activity
#include "main.h"
#include "air.h"
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

Air air;
//-----------------------------------------------------
Air::Air(void)
{
    isOnOld = !isOn;
    value = START_VALUE;
}
//-----------------------------------------------------
void Air::viewScreen(void)
{
    bool result;
    uint32_t i;
    
    canvas.writeFillRect(0,0,320,240,display.COLOR_BACK);                               // вывод заднего фона
    slider.viewScreen();
    
    result = slider.setMinMax(MIN_VALUE, MAX_VALUE, 5, 10, 2, 5, 10); // ставим пределы шкалы и шаг отрисовки сетки
    if (true){
        for (i=0; i<slider.VALUE_LEN; i++){                // ставим ползунок по настройке
            if (slider.values[i] == value) break;
        }
        if (i < slider.VALUE_LEN) slider.position = i;
        else slider.position = 0;
        slider.positionTempOld = 0;
        slider.viewGrid(slider.position,BACKGROUND_IMAGE);    // красная шкала
    }
    
    heater.errorOld = 0;
    heater.stageOld = 0xFF;
    temperature.oldValue = 127;
    
    canvas.loadImageEffect(275,5,BUTTON_SETUP_IMAGE,BUTTON_SETUP_STEP,1);               // отображение значка настроек
    if (startTimer.isOn == 1){
        if (!startTimer.isUnlimited){
            canvas.loadImageEffect(5,5,BUTTON_TIMER_ON_IMAGE,BUTTON_TIMER_STEP,1);      // отображение значка таймеров
        }
        else {
            canvas.loadImageEffect(5,5,BUTTON_TIMER_RED_IMAGE,BUTTON_TIMER_STEP,1);     // отображение значка таймеров
        }
    }
    else{
        canvas.loadImageEffect(5,5,BUTTON_TIMER_OFF_IMAGE,BUTTON_TIMER_STEP,1);         // отображение значка таймеров
    }
    
    canvas.loadImageEffect(BUTTON_LEFT_X,BUTTON_LEFT_Y,ARROW_LEFT_IMAGE,BUTTON_LEFT_STEP,1);      // отображение значка стрелки влево
    canvas.loadImageEffect(BUTTON_RIGHT_X,BUTTON_RIGHT_Y,ARROW_RIGHT_IMAGE,BUTTON_RIGHT_STEP,1);      // отображение значка срелки вправо
    
    isOnOld = !isOn;
}
//-----------------------------------------------------
uint8_t Air::viewHandler(void)
{
    uint8_t a, result;
    
    this->viewTemperature();                        // отображение температуры
    a = slider.viewPosition();                      // программа визуализации настроечной шкалы
    if (a == 0) {                                   // ползунок дополз до места назначения
        value = slider.values[slider.position];
        this->viewMode();                           // вывод на экран режима работы хоз.воды
        this->buttonCheck(0);                       // обработка нажатия механической кнопки
        result = this->sensorCheck();               // обработка касания сенсорного экрана
        startTimer.viewButton();                    // отображение кнопки таймеров
    }
    return result;
}
//-----------------------------------------------------
void Air::viewMode(void)
{
    if (isOnOld != isOn){
        isOnOld = isOn;
        if (isOn == true){
            canvas.loadImageEffect(BUTTON_AIR_X,BUTTON_AIR_Y,BLUE_AIR_IMAGE,BUTTON_AIR_STEP,0);      // отображение значка обдува
        }
        else{
            canvas.loadImageEffect(BUTTON_AIR_X,BUTTON_AIR_Y,BLACK_AIR_IMAGE,BUTTON_AIR_STEP,0);      // отображение значка обдува
        }
    }
}
//-----------------------------------------------------
void Air::viewTemperature(void)
{
    uint8_t x, n;
    char str[7];
    signed char val;
    static signed t;
    
    val = heater.temperature;                   // отображение значка источника температуры
    if (temperature.oldValue != val){
        temperature.oldValue = val;
        if (val <= (-100)){
            //canvas.writeFillRect(TEMPERATURE_ICON_X+30-8,188,60-4,26,display.COLOR_BACK);   // COLOR_BACK
            text.writeString(160-16,192,"--",Font_16x26,display.COLOR_TEMP,display.COLOR_BACK);
        }
        else{
            t = temperature.oldValue;
            n=0;
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
            //canvas.writeFillRect(TEMPERATURE_ICON_X+30-16,188,60,26,display.COLOR_BACK);   // COLOR_BACK
            text.writeString(160-16,192,str,Font_16x26,display.COLOR_TEMP,display.COLOR_BACK);
        }
        temperature.sourceOld = NULL;
    }
}
//-----------------------------------------------------
uint8_t Air::sensorCheck(void)
{
    uint8_t result = 0;
    int8_t s;
    uint32_t i;
    float f;
    int32_t x, y, a, b, c;
    char str[2][7];
    
    if (sensor.status == 1 && sensor.touch == 0){   // касание сенсорного экрана
        sensor.touch = 1;
        
        if (1){
            if (slider.touch==0 && 
                sensor.x1>(BUTTON_AIR_X) && sensor.x1<(BUTTON_AIR_X+BUTTON_AIR_SIZE) && 
                sensor.y1>(BUTTON_AIR_Y) && sensor.y1<(BUTTON_AIR_Y+BUTTON_AIR_SIZE)){   // касание кнопки обдува
                sensor.touch = 0;
                isOn = !isOn;
            }
            if (slider.touch==0 && 
                sensor.x1>(BUTTON_DEC_X-BUTTON_DEC_SIZE) && sensor.x1<(BUTTON_DEC_X+BUTTON_DEC_SIZE) && 
                sensor.y1>(BUTTON_DEC_Y-BUTTON_DEC_SIZE) && sensor.y1<(BUTTON_DEC_Y+BUTTON_DEC_SIZE)){  // касание кнопки декремента
                sensor.touch = 0;
                if (core.getTimer() == 0){
                    if (slider.mode == 0) core.setTimer(500);
                    slider.mode = 1;
                    s = slider.values[slider.position];
                    while(1){
                        if (slider.position <= 0) break;
                        slider.position--;
                        if (slider.values[slider.position] != s) break;
                    }
                }
            }
            if (slider.touch==0 && 
                sensor.x1>(BUTTON_INC_X-BUTTON_INC_SIZE) && sensor.x1<(BUTTON_INC_X+BUTTON_INC_SIZE) && 
                sensor.y1>(BUTTON_INC_Y-BUTTON_INC_SIZE) && sensor.y1<(BUTTON_INC_Y+BUTTON_INC_SIZE)){  // касание кнопки инкремента
                sensor.touch = 0;
                if (core.getTimer() == 0){
                    if (slider.mode == 0) core.setTimer(500);
                    slider.mode = 1;
                    s = slider.values[slider.position];
                    while(1){
                        if (slider.position >= (slider.VALUE_LEN-1)) break;
                        slider.position++;
                        if (slider.values[slider.position] != s) break;
                    }
                }
            }
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
            if (slider.touch==0 && 
                sensor.x1>(BUTTON_LEFT_X) && sensor.x1<(BUTTON_LEFT_X+BUTTON_LEFT_SIZE) && 
                sensor.y1>(BUTTON_LEFT_Y) && sensor.y1<(BUTTON_LEFT_Y+BUTTON_LEFT_SIZE)){  // касание кнопки влево
                sensor.touch = 0;
                if (sensor.isTouchElement == FALSE){
                    sensor.isTouchElement = TRUE;
                    result = 3;
                }
            }
            if (slider.touch==0 && 
                sensor.x1>(BUTTON_RIGHT_X) && sensor.x1<(BUTTON_RIGHT_X+BUTTON_RIGHT_SIZE) && 
                sensor.y1>(BUTTON_RIGHT_Y) && sensor.y1<(BUTTON_RIGHT_Y+BUTTON_RIGHT_SIZE)){  // касание кнопки вправо
                sensor.touch = 0;
                if (sensor.isTouchElement == FALSE){
                    sensor.isTouchElement = TRUE;
                    result = 4;
                }
            }
            if ((sensor.x1>(slider.valueX[slider.positionTemp]-MAIN_SLIDER_RADIUS) && sensor.x1<(slider.valueX[slider.positionTemp]+MAIN_SLIDER_RADIUS) && 
                sensor.y1>(slider.valueY[slider.positionTemp]-MAIN_SLIDER_RADIUS) && sensor.y1<(slider.valueY[slider.positionTemp]+MAIN_SLIDER_RADIUS)) ||
                slider.touch == 1){ // касание ползунка
                sensor.touch = 0;
                slider.touch = 1;
                a = 1000;
                b = 0;
                for (i=0; i<slider.VALUE_LEN; i++){
                    x = slider.valueX[i] - sensor.x1;
                    if (x < 0) x = -x;
                    y = slider.valueY[i] - sensor.y1;
                    if (y < 0) y = -y;
                    
                    c = sqrt(std::pow((float)x,2) + std::pow((float)y,2));
                    
                    if (c < a){
                        a = c;
                        b = i;
                    }
                }
                slider.position = b;
            }
            if (sensor.touch == 1){ // касание пустоты(подсказка)
                for (x=0; x<6; x++) str[0][x] = 0;
                a = text.decToString(str[0], slider.values[slider.positionTemp]);
                if (slider.positionTemp < (slider.VALUE_LEN/2)) a=SLIDER_FONT.width*(a-1);
                else a = 0;
                for (f=1; f>=0; f-=SLIDER_HELP_STEP){
                    i = (uint32_t)((float)((display.COLOR_TEMP>>16)&0xFF)*f)<<16;
                    i += (uint32_t)((float)((display.COLOR_TEMP>>8)&0xFF)*f)<<8;
                    i += (uint32_t)((float)(display.COLOR_TEMP&0xFF)*f);
                    text.writeStringOver(slider.valueX[slider.positionTemp]-SLIDER_FONT.width/2-a,slider.valueY[slider.positionTemp]-SLIDER_FONT.height/2,str[0],SLIDER_FONT,i,display.COLOR_BACK,0);
                }
                for (f=0; f<=1; f+=SLIDER_HELP_STEP){
                    i = (uint32_t)((float)((display.COLOR_TEMP>>16)&0xFF)*f)<<16;
                    i += (uint32_t)((float)((display.COLOR_TEMP>>8)&0xFF)*f)<<8;
                    i += (uint32_t)((float)(display.COLOR_TEMP&0xFF)*f);
                    text.writeStringOver(slider.valueX[slider.positionTemp]-SLIDER_FONT.width/2-a,slider.valueY[slider.positionTemp]-SLIDER_FONT.height/2,str[0],SLIDER_FONT,i,display.COLOR_BACK,0);
                }
                 for (f=1; f>=0; f-=SLIDER_HELP_STEP){
                    i = (uint32_t)((float)((display.COLOR_TEMP>>16)&0xFF)*f)<<16;
                    i += (uint32_t)((float)((display.COLOR_TEMP>>8)&0xFF)*f)<<8;
                    i += (uint32_t)((float)(display.COLOR_TEMP&0xFF)*f);
                    text.writeStringOver(slider.valueX[slider.positionTemp]-SLIDER_FONT.width/2-a,slider.valueY[slider.positionTemp]-SLIDER_FONT.height/2,str[0],SLIDER_FONT,i,display.COLOR_BACK,0);
                }
                for (f=0; f<=1; f+=SLIDER_HELP_STEP){
                    i = (uint32_t)((float)((display.COLOR_TEMP>>16)&0xFF)*f)<<16;
                    i += (uint32_t)((float)((display.COLOR_TEMP>>8)&0xFF)*f)<<8;
                    i += (uint32_t)((float)(display.COLOR_TEMP&0xFF)*f);
                    text.writeStringOver(slider.valueX[slider.positionTemp]-SLIDER_FONT.width/2-a,slider.valueY[slider.positionTemp]-SLIDER_FONT.height/2,str[0],SLIDER_FONT,i,display.COLOR_BACK,0);
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
            if (sensor.x1>(BUTTON_DOWN_LEFT_X) && sensor.x1<(BUTTON_DOWN_LEFT_X+BUTTON_DOWN_LEFT_X_SIZE) && 
                sensor.y1>(BUTTON_DOWN_LEFT_Y) && sensor.y1<(BUTTON_DOWN_LEFT_Y+BUTTON_DOWN_LEFT_Y_SIZE)){  // отпускание времени
                if (sensor.getTickTouch() < 500){
                    if (clock.mode == clock.MODE_TIME){
                        clock.mode = clock.MODE_WORKTIME;
                    }
                    else{
                        clock.mode = clock.MODE_TIME;
                    }
                }
            }
        }
        
        if (slider.mode == 1 || slider.touch == 1){
            if (heater.isPreheater){
                if (heater.isWarmingUp == TRUE){
                    heater.setup.warmUpTSetpoint = slider.values[slider.position];
                }
                else{
                    heater.setup.preheaterTSetpoint = slider.values[slider.position];
                }
            }
            else {
                if (heater.setup.workBy==3){
                    heater.setup.airHeaterPowerSetPoint = slider.values[slider.position];
                }
                else{
                    heater.setup.airHeaterTSetPoint = slider.values[slider.position];
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
void Air::buttonPress(void)
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
//-----------------------------------------------------
void Air::buttonCheck(uint8_t mode)
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
//-----------------------------------------------------
