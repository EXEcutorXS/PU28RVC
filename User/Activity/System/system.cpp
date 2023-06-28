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
#include "system.h"
#include "setup.h"      // исправить, нарушение иерархии классов!!!
#include "error.h"
// Objects
#include "usart.h"
#include "hcu.h"
#include "start_timer.h"
#include "slider.h"
#include "checkbox.h"
#include "clock.h"
#include "pgn_rvc.h"
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
#include "font.h"
#include "rvc.h"
// Library
#include <math.h>

System air(45, 50, 90, 45, BUTTON_CENTRAL_IMAGE, BUTTON_CENTRAL_IMAGE, 5, 10);
//-----------------------------------------------------
System::System(uint8_t offVal, uint8_t minVal, uint8_t maxVal, uint8_t startVal, uint8_t offImage, uint8_t onImage, uint8_t smallStep, uint8_t bigStep)
{
    OFF_VALUE = offVal;
    MIN_VALUE = minVal;
    MAX_VALUE = maxVal;
    START_VALUE = startVal;
    OFF_IMAGE = offImage;
    ON_IMAGE = onImage;
    SMALL_STEP = smallStep;
    BIG_STEP = bigStep;
    value = START_VALUE;
    temperatureActual = -100;
    temperatureActualOld = ~temperatureActual;
}
//-----------------------------------------------------
void System::viewScreen(bool isFirst)
{
    bool result, isClear=true;
    uint32_t i;
    uint8_t a;
    
    ///error.codeOld = 0;
    hcu.stageOld = 0xFF;
    temperature.oldValue = 127;
    isWaterOnOld = !isWaterOn;
    isFHeaterOnOld = !isFHeaterOn;
    isEHeaterOnOld = !isEHeaterOn;
    temperatureActualOld = ~temperatureActual;
    
    if (isFirst == true){
        canvas.writeFillRect(0,0,320,240,display.COLOR_BACK);   // вывод заднего фона
        slider.viewScreen();
        canvas.loadImageEffect(BUTTON_SYSTEM_X,BUTTON_SYSTEM_Y,ON_IMAGE,BUTTON_SYSTEM_STEP,0);      // отображение значка системы
        viewMode();
        slider.setMinMax(OFF_VALUE, MIN_VALUE, MAX_VALUE, SMALL_STEP, BIG_STEP); // ставим пределы шкалы и шаг отрисовки сетки
        value = hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)];
        if (air.isAirOn){
            for (i=0; i<slider.VALUE_LEN; i++){                // ставим ползунок по настройке
                if (slider.values[i] == value) break;
            }
            if (i < slider.VALUE_LEN) slider.position = i;
            else slider.position = 0;
        }
        else slider.position = 0;
        slider.positionTempOld = 0;
        while(true){
            result = slider.viewGridSens(slider.position,BACKGROUND_IMAGE,isClear);    // красная шкала
            isClear = false;
            if (result == false) a = sensorCheck();
            else break;
            if (a==3 || a==4) break;
        }
        slider.drawDigGrid();
        canvas.loadImageEffect(275,5,BUTTON_SETUP_IMAGE,BUTTON_SETUP_STEP,1);               // отображение значка настроек
        
        if ((isDay|isSelectDay)&(!isSelectNight)){
            canvas.loadImageEffect(BUTTON_DAY_NIGHT_X,BUTTON_DAY_NIGHT_Y,TEXT_DAY_IMAGE,BUTTON_SETUP_STEP,0);
        }
        else{
            canvas.loadImageEffect(BUTTON_DAY_NIGHT_X,BUTTON_DAY_NIGHT_Y,TEXT_NIGHT_IMAGE,BUTTON_SETUP_STEP,0);
        }
        if (isBleAccept == true){
            canvas.loadImageEffect(BUTTON_BLE_X,BUTTON_BLE_Y,TEXT_BLE_IMAGE,BUTTON_SETUP_STEP,0);
        }
    }
    else{
        viewMode();
        slider.setMinMax(OFF_VALUE, MIN_VALUE, MAX_VALUE, SMALL_STEP, BIG_STEP); // ставим пределы шкалы и шаг отрисовки сетки
        for (i=0; i<slider.VALUE_LEN; i++){                // ставим ползунок по настройке
            if (slider.values[i] == value) break;
        }
        if (i < slider.VALUE_LEN) slider.position = i;
        else slider.position = 0;
        slider.positionTempOld = 0;
        while(true){
            result = slider.viewGridSens(slider.position,BACKGROUND_IMAGE,isClear);    // красная шкала
            isClear = false;
            if (result == false) a = sensorCheck();
            else break;
            if (a==3 || a==4) break;
        }
        slider.drawDigGrid();
        //slider.viewDigGrid(slider.values[slider.position], 1);
        slider.positionTemp = slider.VALUE_LEN-1+SLIDER_STEP-1;
        slider.old = slider.position;
        slider.setVal(slider.positionTemp);
    }
    slider.timerSliderMin = core.getTick()+2500;
    viewTemperature(true);
}
//-----------------------------------------------------
uint8_t System::viewHandler(void)
{
    uint8_t a, result;
    
    this->viewTemperature(false);                   // отображение температуры
    a = slider.viewPosition();                      // программа визуализации настроечной шкалы
    if (a == 0){                                   // ползунок дополз до места назначения
        value = slider.values[slider.position];
        if (sensor.status){
            isAirOn = (slider.position != 0);
        }
        this->viewMode();                           // вывод на экран режима работы хоз.воды
        this->viewDuration();                       // вывод на экран режима выбора длительности работы
        this->buttonCheck(0);                       // обработка нажатия механической кнопки
        this->checkConnect();                       // проверка наличия связи
    }
    result = this->sensorCheck();                   // обработка касания сенсорного экрана
    startTimer.viewButton();                        // отображение кнопки таймеров
    return result;
}
//-----------------------------------------------------
void System::viewMessage(char *txt)
{
    uint8_t x, y, n, i;
    char str[20];
    
    n=0;
    i = 0;
    x = 0;
    while(1){
        if (txt[i] == ' ') x++;
        if (txt[i] == 0) break;
        i++;
    }
    if (x == 0) y = 130;
    else if (x == 1) y = 120;
    else if (x == 2) y = 110;
    for (x=0; x<19; x++) str[x] = 0;
    i = 0;
    while(1){
        if (txt[i] == ' ' || txt[i] == 0){
            text.writeStringOver(160-(Font_11x18.width*n/2),y-Font_11x18.height/2,str,Font_11x18,display.COLOR_MESSAGE,display.COLOR_BACK, 0);
            y += 20;
            n=0;
            for (x=0; x<19; x++) str[x] = 0;
            if (txt[i] == 0 || y >= 170) break;
            i++;
        }
        else{
            str[n] = txt[i++];
            if (str[n] == '_') str[n] = ' ';
            n++;
        }
    }
}
//-----------------------------------------------------
void System::viewMode(void)
{
    if (isWaterOnOld != isWaterOn){
        isWaterOnOld = isWaterOn;
        isChange = true;
        if (isWaterOn == false){
            canvas.loadImageEffect(5,5,BLACK_WATER_IMAGE,BUTTON_TIMER_STEP,1);                       // отображение значка подогрева воды
        }
        else{
            canvas.loadImageEffectRed(5,5,BLUE_WATER_IMAGE,BUTTON_TIMER_STEP);                       // отображение значка подогрева воды
        }
    }
    if (isFHeaterOnOld != isFHeaterOn){
        isFHeaterOnOld = isFHeaterOn;
        isChange = true;
        //if (heater.error){
        //       canvas.loadImageEffectInvert(BUTTON_LEFT_X,BUTTON_LEFT_Y,BLACK_FHEAT_IMAGE,BUTTON_TIMER_STEP);                       // отображение значка подогревателя
        //}
        /*else */if (isFHeaterOn == false){
            canvas.loadImageEffect(BUTTON_LEFT_X,BUTTON_LEFT_Y,BLACK_FHEAT_IMAGE,BUTTON_TIMER_STEP,1);                       // отображение значка подогревателя
        }
        else{
            canvas.loadImageEffectRed(BUTTON_LEFT_X,BUTTON_LEFT_Y,BLUE_FHEAT_IMAGE,BUTTON_TIMER_STEP);                       // отображение значка подогревателя
        }
    }
    if (isEHeaterOnOld != isEHeaterOn){
        isEHeaterOnOld = isEHeaterOn;
        isChange = true;
        if (isEHeaterOn == false){
            canvas.loadImageEffect(BUTTON_RIGHT_X,BUTTON_RIGHT_Y,BLACK_EHEAT_IMAGE,BUTTON_TIMER_STEP,1);                       // отображение значка ТЭНА
        }
        else{
            canvas.loadImageEffectRed(BUTTON_RIGHT_X,BUTTON_RIGHT_Y,BLUE_EHEAT_IMAGE,BUTTON_TIMER_STEP);                       // отображение значка ТЭНА
        }
    }
}
//-----------------------------------------------------
void System::viewDuration(void)
{
    char str[21];
    uint8_t n;
    
    if (isSetDuration == 1){
        if ((core.getTick() - timerSetDuration) > 5000){
            isSetDuration = 0;
            canvas.loadImageEffectPoly(104,74,BUTTON_CENTRAL_IMAGE,8,0);
        }
    }
    else if (isSetDuration == 2){
        isSetDuration = 0;
        canvas.loadImageEffectPoly(104,74,BUTTON_CENTRAL_IMAGE,8,0);
    }
    else if (isSetDuration == 3){
        isSetDuration = 1;
        canvas.loadImageEffectPoly(104,74,BUTTON_ERROR_IMAGE,8,0);
        
        if (durationVal < 60){
            n = text.charToString(str, "Starting for ");
            n += text.decToString(&str[n], durationVal);
            n += text.charToString(&str[n], "_min");
            viewMessage(str);
        }
        else if (durationVal < 1440){
            n = text.charToString(str, "Starting for ");
            n += text.decToString(&str[n], durationVal/60);
            n += text.charToString(&str[n], "_hour");
            viewMessage(str);
        }
        else{
            n = text.charToString(str, "Starting for ");
            n += text.decToString(&str[n], durationVal/60/24);
            n += text.charToString(&str[n], "_day");
            viewMessage(str);
        }   
    }
}
//-----------------------------------------------------
void System::viewDuration(bool isVisible, uint16_t durationVal)
{
    if (isVisible){
        timerSetDuration = core.getTick();
        isSetDuration = 3;
        this->durationVal = durationVal;
    }
    else{
        isSetDuration = 2;
    }
}
//-----------------------------------------------------
void System::viewTemperature(bool isReset)
{
    uint8_t n, w, i;
    static char str[15];
    int16_t val;
    static int16_t t;
    static uint32_t timer=0;
    static char oldStr[7];
    static uint8_t oldWidth = 0;
    int16_t temp;
    static uint8_t countFault = 0;
    char unit;
    bool res;
    int16_t tPanel, tHcu;
    
    if (core.getTick() < 5000) return;
    
    if ((core.getTick()-timer)>1000){
        timer=core.getTick();
        
        if (isReset == true){
            oldWidth = 0;
        }
        
        if (display.setup.celsius & 0x01){
            unit = 'C';
            tPanel = temperature.panel;
            tHcu = temperatureActual;
        }
        else{
            unit = 'F';
            tPanel = core.farToCel(temperature.panel);
            tHcu = core.farToCel(temperatureActual);
        }
        
		if (rvc.externalTemperatureProvided)
		{
			if (display.setup.celsius & 0x01)
				temp = rvc.externalTemperature;
			else
				temp = core.celToFar(rvc.externalTemperature);
		}
        else if (air.isPanelSensor & 0x01){
            temp = temperature.panel;
            if (tPanel >= 127){
                if (countFault < 3) countFault++;
                else{
                    countFault = 0;
                    hcu.faultCodePanel = 51;
                    if (tHcu > -100 && tHcu < 127){
                        air.isPanelSensor = 0+2;
                        setup.isChange = true;
                    }
                }
            }
            else if (tPanel <= -100){
                if (countFault < 3) countFault++;
                else{
                    countFault = 0;
                    hcu.faultCodePanel = 52;
                    if (tHcu > -100 && tHcu < 127){
                        air.isPanelSensor = 0+2;
                        setup.isChange = true;
                    }
                }
            }
        }
        else{
            temp = temperatureActual;
            if (tHcu >= 127){
                if (countFault < 3) countFault++;
                else{
                    countFault = 0;
                    hcu.faultCodePanel = 41;
                    if (tPanel > -100 && tPanel < 127){
                        air.isPanelSensor = 1+2;
                        setup.isChange = true;
                    }
                }
            }
            else if (tHcu <= -100){
                if (countFault < 3) countFault++;
                else{
                    countFault = 0;
                    hcu.faultCodePanel = 42;
                    if (tPanel > -100 && tPanel < 127){
                        air.isPanelSensor = 1+2;
                        setup.isChange = true;
                    }
                }
            }
        }
        ///*
        val = temp;
        n=0;
        for (i=0; i<6; i++) str[i] = 0;
        n += text.decToString(&str[n], val);
        str[n++] = '~';
        str[n++] = unit;
        
        w = Font_16x26.width*n;
        res = false;
        for (i=0; i<6; i++){
            if (oldStr[i] != str[i]){
                res = true;
            }
        }
        
        if (t != val || isReset == true || res == true){
            t = val;
            if (oldWidth != w){
                oldWidth = w;
                i = 5;
                canvas.writeFillRect((160-Font_16x26.width*i/2),190,Font_16x26.width*i,26,0);
                for (i=0; i<6; i++) oldStr[i] = 0;
            }
            if (n > 5 || 
                (unit == 'C' && val >= 127) || 
                (unit == 'F' && val >= 260)){
                w = Font_16x26.width*2;
                str[0] = '-';
                str[1] = '-';
                str[2] = 0;
                canvas.writeFillRect((160-Font_16x26.width*5/2),190,Font_16x26.width*5,26,0);
					text.writeOneDigit((160-w/2)+Font_16x26.width*0,190,str[0],rvc.externalTemperatureProvided?display.COLOR_TEMP_RVC:display.COLOR_TEMP, false);
                text.writeOneDigit((160-w/2)+Font_16x26.width*1,190,str[1],rvc.externalTemperatureProvided?display.COLOR_TEMP_RVC:display.COLOR_TEMP, false);
            }
            else{
                for (i=0; i<6; i++){
                    if (oldStr[i] != str[i] || rvc.externalTemperatureProvidedChanged){
                        text.writeOneDigit((160-w/2)+Font_16x26.width*i,190,str[i],rvc.externalTemperatureProvided?display.COLOR_TEMP_RVC:display.COLOR_TEMP, false);
                    }
                }
				rvc.externalTemperatureProvidedChanged = false;
            }
            for (i=0; i<6; i++) oldStr[i] = str[i];
        }
        //*/
        /*
        if (temperatureActualOld != temp){
            temperatureActualOld = temp;
            n = text.decToString(str, (int32_t)temp);
            str[n++] = '~';
            str[n++] = unit;
            str[n] = 0;
            //n++;
            w = n*16;
            if (n < 6 && temp > -100 && temp < 127){
                x = Font_16x26.width*n;
                ///text.writeStringOver(160-oldLen/2,192,oldStr,Font_16x26,display.COLOR_BACK,display.COLOR_BACK,0);
                text.writeDigit(160-oldLen/2,190,oldStr,display.COLOR_BACK, true);
                oldLen = x;
                for (i=0; i<6; i++) oldStr[i] = str[i];
                ///text.writeStringOver(160-x/2,192,str,Font_16x26,display.COLOR_TEMP,display.COLOR_BACK,0);
                text.writeDigit(160-x/2,190,str,display.COLOR_TEMP, false);
            }
            else{
                text.writeDigit(160-oldLen/2,190,oldStr,display.COLOR_BACK, true);
                text.writeDigit(160-16,190,"--",display.COLOR_TEMP, true);
                oldLen = Font_16x26.width*2;
                for (i=0; i<6; i++) oldStr[i] = 0;
                oldStr[0] = '-';
                oldStr[1] = '-';
            }
        }
        */
    }
}
//-----------------------------------------------------
uint8_t System::sensorCheck(void)
{
    uint8_t result = 0;
    int16_t s;
    uint32_t i;
    float f;
    int32_t x, y, a, b, c;
    char str[2][7];
    
    if (sensor.status == 1 && sensor.touch == 0){   // касание сенсорного экрана
        sensor.touch = 1;
        
        if (1){
            if (slider.touch==0 && 
                sensor.x1>(BUTTON_SYSTEM_X+10) && sensor.x1<(BUTTON_SYSTEM_X+BUTTON_SYSTEM_SIZE-20) && 
                sensor.y1>(BUTTON_SYSTEM_Y+10) && sensor.y1<(BUTTON_SYSTEM_Y+BUTTON_SYSTEM_SIZE-20)){   // касание кнопки системы
                sensor.touch = 3;
                if (sensor.isTouchElement == FALSE){
                    sensor.isTouchElement = TRUE;
                    if (isSetDuration){
                        result = 2;
                        setup.numberScreen = 5;
                    }
                    else{
                        result = 1;
                    }
                }
            }
            else if (slider.touch==0 && 
                sensor.x1>(BUTTON_DEC_X-BUTTON_DEC_SIZE) && sensor.x1<(BUTTON_DEC_X+BUTTON_DEC_SIZE) && 
                sensor.y1>(BUTTON_DEC_Y-BUTTON_DEC_SIZE) && sensor.y1<(BUTTON_DEC_Y+BUTTON_DEC_SIZE)){  // касание кнопки декремента
                sensor.touch = 0;
                slider.timerSliderMin = core.getTick()+1000;
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
            else if (slider.touch==0 && 
                sensor.x1>(BUTTON_INC_X-BUTTON_INC_SIZE) && sensor.x1<(BUTTON_INC_X+BUTTON_INC_SIZE) && 
                sensor.y1>(BUTTON_INC_Y-BUTTON_INC_SIZE) && sensor.y1<(BUTTON_INC_Y+BUTTON_INC_SIZE)){  // касание кнопки инкремента
                sensor.touch = 0;
                slider.timerSliderMin = core.getTick()+1000;
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
            else if (slider.touch==0 && 
                sensor.x1<(BUTTON_TIMER_X+BUTTON_TIMER_SIZE) && 
                sensor.y1<(BUTTON_TIMER_Y+BUTTON_TIMER_SIZE)){  // касание кнопки подогрева воды
                sensor.touch = 2;
                isWaterOn = !isWaterOn;
                if (hcu.durationDomesticWater < 300){
                    viewDuration(isWaterOn, hcu.durationDomesticWater);
                    if (isWaterOn) hcu.timerOffDomesticWater = core.getTick();
                }
				//canPGNRVC.msgWaterHeater2();
            }
            else if (slider.touch==0 && 
                sensor.x1>(BUTTON_UP_RIGHT_X) &&
                sensor.y1<(BUTTON_UP_RIGHT_Y+BUTTON_UP_RIGHT_SIZE)){  // касание кнопки настроек
                sensor.touch = 0;
                if (sensor.isTouchElement == FALSE){
                    sensor.isTouchElement = TRUE;
                    result = 2;
                }
            }
            else if (slider.touch==0 && 
                sensor.x1<(BUTTON_LEFT_X+BUTTON_LEFT_SIZE) && 
                sensor.y1>(BUTTON_LEFT_Y-10)){  // касание кнопки отопителя
                sensor.touch = 2;
                isFHeaterOn = !isFHeaterOn;
                if (hcu.durationSystem < 7200){
                    viewDuration(isFHeaterOn, hcu.durationSystem);
                    if (isFHeaterOn) hcu.timerOffSystem = core.getTick();
                }
                canPGNRVC.msgWaterHeater();
            }
            else if (slider.touch==0 && 
                sensor.x1>(BUTTON_RIGHT_X-10) && 
                sensor.y1>(BUTTON_RIGHT_Y-10)){  // касание кнопки ТЭНА
                sensor.touch = 2;
                isEHeaterOn = !isEHeaterOn;
                if (hcu.durationSystem < 7200){
                    viewDuration(isEHeaterOn, hcu.durationSystem);
                    if (isEHeaterOn) hcu.timerOffSystem = core.getTick();
                }
                canPGNRVC.msgWaterHeater();
            }
            else if (slider.touch==0 && 
                sensor.x1>(BUTTON_DAY_NIGHT_X) && sensor.x1<(BUTTON_DAY_NIGHT_X+BUTTON_DAY_NIGHT_X_SIZE) && 
                sensor.y1>(BUTTON_DAY_NIGHT_Y) && sensor.y1<(BUTTON_DAY_NIGHT_Y+BUTTON_DAY_NIGHT_Y_SIZE)){  // касание кнопки дня/ночи
                sensor.touch = 4;
                isResetSelectDayNight = true;
                timerResetSelectDayNight = core.getTick();
                if ((isSelectDay || isDay) && (!isSelectNight)){
                    isSelectDay = false;
                    isSelectNight = true;
                    canvas.loadImageEffect(BUTTON_DAY_NIGHT_X,BUTTON_DAY_NIGHT_Y,TEXT_NIGHT_IMAGE,BUTTON_SETUP_STEP,0);
                    slider.setPosition(hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)]);
                }
                else{
                    isSelectNight = false;
                    isSelectDay = true;
                    canvas.loadImageEffect(BUTTON_DAY_NIGHT_X,BUTTON_DAY_NIGHT_Y,TEXT_DAY_IMAGE,BUTTON_SETUP_STEP,0);
                    slider.setPosition(hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)]);
                }
            }
            
            
            else if ((sensor.x1>(slider.valueX[slider.positionTemp]-MAIN_SLIDER_RADIUS) && sensor.x1<(slider.valueX[slider.positionTemp]+MAIN_SLIDER_RADIUS) && 
                sensor.y1>(slider.valueY[slider.positionTemp]-MAIN_SLIDER_RADIUS) && sensor.y1<(slider.valueY[slider.positionTemp]+MAIN_SLIDER_RADIUS)) ||
                slider.touch == 1){ // касание ползунка
                sensor.touch = 0;
                slider.touch = 1;
                slider.timerSliderMin = core.getTick()+100;
                a = 1000;
                b = 0;
                for (i=0; i<slider.VALUE_LEN; i++){
                    x = slider.valueX[i] - sensor.x1;
                    if (x < 0) x = -x;
                    y = slider.valueY[i] - sensor.y1;
                    if (y < 0) y = -y;
                    
                    c = sqrt((float)x*(float)x + (float)y*(float)y);
                    
                    if (c < a){
                        a = c;
                        b = i;
                    }
                }
                slider.position = b;
            }
            else if (sensor.touch == 1){ // касание пустоты(подсказка)
                if (slider.values[slider.positionTemp] < slider.minValue){
                    
                }
                else{
                    for (x=0; x<6; x++) str[0][x] = 0;
                    a = text.decToString(str[0], slider.values[slider.positionTemp]);
                    if (slider.positionTemp < (slider.VALUE_LEN/2)) a=SLIDER_FONT.width*(a-1);
                    else a = 0;
                    for (f=1; f>=0; f-=SLIDER_HELP_STEP){
                        i = (uint32_t)((float)((display.COLOR_TEMP>>16)&0xFF)*f)<<16;
                        i += (uint32_t)((float)((display.COLOR_TEMP>>8)&0xFF)*f)<<8;
                        i += (uint32_t)((float)(display.COLOR_TEMP&0xFF)*f);
                        if (slider.positionTemp < slider.VALUE_LEN) text.writeStringOver(slider.valueX[slider.positionTemp]-SLIDER_FONT.width/2-a,slider.valueY[slider.positionTemp]-SLIDER_FONT.height/2,str[0],SLIDER_FONT,i,display.COLOR_BACK,0);
                    }
                    for (f=0; f<=1; f+=SLIDER_HELP_STEP){
                        i = (uint32_t)((float)((display.COLOR_TEMP>>16)&0xFF)*f)<<16;
                        i += (uint32_t)((float)((display.COLOR_TEMP>>8)&0xFF)*f)<<8;
                        i += (uint32_t)((float)(display.COLOR_TEMP&0xFF)*f);
                        if (slider.positionTemp < slider.VALUE_LEN) text.writeStringOver(slider.valueX[slider.positionTemp]-SLIDER_FONT.width/2-a,slider.valueY[slider.positionTemp]-SLIDER_FONT.height/2,str[0],SLIDER_FONT,i,display.COLOR_BACK,0);
                    }
                     for (f=1; f>=0; f-=SLIDER_HELP_STEP){
                        i = (uint32_t)((float)((display.COLOR_TEMP>>16)&0xFF)*f)<<16;
                        i += (uint32_t)((float)((display.COLOR_TEMP>>8)&0xFF)*f)<<8;
                        i += (uint32_t)((float)(display.COLOR_TEMP&0xFF)*f);
                        if (slider.positionTemp < slider.VALUE_LEN) text.writeStringOver(slider.valueX[slider.positionTemp]-SLIDER_FONT.width/2-a,slider.valueY[slider.positionTemp]-SLIDER_FONT.height/2,str[0],SLIDER_FONT,i,display.COLOR_BACK,0);
                    }
                    for (f=0; f<=1; f+=SLIDER_HELP_STEP){
                        i = (uint32_t)((float)((display.COLOR_TEMP>>16)&0xFF)*f)<<16;
                        i += (uint32_t)((float)((display.COLOR_TEMP>>8)&0xFF)*f)<<8;
                        i += (uint32_t)((float)(display.COLOR_TEMP&0xFF)*f);
                        if (slider.positionTemp < slider.VALUE_LEN) text.writeStringOver(slider.valueX[slider.positionTemp]-SLIDER_FONT.width/2-a,slider.valueY[slider.positionTemp]-SLIDER_FONT.height/2,str[0],SLIDER_FONT,i,display.COLOR_BACK,0);
                    }
                }
            }
        }
    }
    else if (sensor.status == 0){   // отпускание сенсорного экрана
        if (sensor.isTouchElement == TRUE){
            
        }
        
        if (slider.mode == 1 || slider.touch == 1){
            if (slider.values[slider.position] > OFF_VALUE){
                hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)] = slider.values[slider.position];
                air.isAirOn = true;
            }
            else{
                air.isAirOn = false;
            }
            hcu.lockTimer = core.getTick();
        }
        
        sensor.touch = 0;
        //slider.touchOld = slider.touch;
        slider.touch = 0;
        slider.mode = 0;
        core.setTimer(0);
        sensor.isTouchElement = FALSE;
    }
    
    if (sensor.touch != 0){
        hcu.lockTimer = core.getTick();
    }
    
    return result;
}
//-----------------------------------------------------
void System::buttonPress(void)
{
    
}
//-----------------------------------------------------
void System::buttonCheck(uint8_t mode)
{
    if ((sensor.button==1 && sensor.status==0) || 
        (sensor.button==1 && sensor.status==1 && sensor.touch<2 && 
         sensor.x1>120 && sensor.x1<200 && sensor.y1>90 && sensor.y1<170)){
        core.delayMs(1); 
        
        if (gpio_input_bit_get(GPIOA, GPIO_PIN_0) != RESET){     // нажатие механической кнопки
            sensor.button = 0;
            if (!mode)this->buttonPress();
        }
        else sensor.button = 0;
    }
    ///if (gpio_input_bit_get(GPIOA, GPIO_PIN_0) != RESET){     // нажатие механической кнопки
    ///    display.setTimer(display.setup.timeout*1000);
    ///    if (sensor.button==0 && sensor.status==0){
    ///        if (!mode)this->buttonPress();
    ///    }
    ///}
}
//-----------------------------------------------------
void System::checkConnect(void)
{
    if (usart.linkCnt == 40){
        if (hcu.isConnect != hcu.CONNECT_STATUS_ERROR){
            hcu.faultCodeHcu = 20;
        }
        hcu.isConnect = hcu.CONNECT_STATUS_ERROR;
    }
    /*else if(hcu.answerTimeOut == 0){
        if (hcu.isConnect == hcu.CONNECT_STATUS_SEARCH){
            //canvas.writeFillRect(104, 74, 112, 112, display.COLOR_BACK);
        }
        hcu.isConnect = hcu.CONNECT_STATUS_OK;
    }    */
}
//-----------------------------------------------------
void System::checkDayNight(void)
{
    uint16_t dayTime = dayTimeH*60+dayTimeM;
    uint16_t nightTime = nightTimeH*60+nightTimeM;
    uint16_t time = clock.hour*60+clock.minute;
    
    if (isResetSelectDayNight){
        if ((core.getTick()-timerResetSelectDayNight) > 10000){
            isResetSelectDayNight = false;
            isSelectDay = false;
            isSelectNight = false;
            if (isDay){
                if (screen_visible == SCREEN_VISIBLE_AIR) canvas.loadImageEffect(BUTTON_DAY_NIGHT_X,BUTTON_DAY_NIGHT_Y,TEXT_DAY_IMAGE,BUTTON_SETUP_STEP,0);
                slider.setPosition(hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)]);
            }
            else{
                if (screen_visible == SCREEN_VISIBLE_AIR) canvas.loadImageEffect(BUTTON_DAY_NIGHT_X,BUTTON_DAY_NIGHT_Y,TEXT_NIGHT_IMAGE,BUTTON_SETUP_STEP,0);
                slider.setPosition(hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)]);
            }
        }
    }
    
    if (time >= dayTime && time < nightTime){
        // дневной режим работы
        if (isDay == false){
            isDay = true;
            if (screen_visible == SCREEN_VISIBLE_AIR) canvas.loadImageEffect(BUTTON_DAY_NIGHT_X,BUTTON_DAY_NIGHT_Y,TEXT_DAY_IMAGE,BUTTON_SETUP_STEP,0);
            slider.setPosition(hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)]);
        }
    }
    else{
        // ночной режим работы
        if (isDay == true){
            isDay = false;
            if (screen_visible == SCREEN_VISIBLE_AIR) canvas.loadImageEffect(BUTTON_DAY_NIGHT_X,BUTTON_DAY_NIGHT_Y,TEXT_NIGHT_IMAGE,BUTTON_SETUP_STEP,0);
            slider.setPosition(hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)]);
        }
    }
}
//-----------------------------------------------------
void System::changeScale(uint8_t offVal, uint8_t minVal, uint8_t maxVal, uint8_t startVal, uint8_t offImage, uint8_t onImage, uint8_t smallStep, uint8_t bigStep)
{
    OFF_VALUE = offVal;
    MIN_VALUE = minVal;
    MAX_VALUE = maxVal;
    START_VALUE = startVal;
    OFF_IMAGE = offImage;
    ON_IMAGE = onImage;
    SMALL_STEP = smallStep;
    BIG_STEP = bigStep;
    value = START_VALUE;
    temperatureActual = -100;
    temperatureActualOld = ~temperatureActual;
}
//-----------------------------------------------------
