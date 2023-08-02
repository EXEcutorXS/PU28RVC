/******************************************************************************
* ООО Теплостар
* Самара
* 
* Программисты: Клюев А.А.
* 
* 10.05.2019
* Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
// Activity
#include "main.h"
#include "error.h"
#include "setup.h"      // исправить, нарушение иерархии классов!!!
// Objects
#include "usart.h"
#include "hcu.h"
#include "start_timer.h"
#include "slider.h"
#include "checkbox.h"
#include "clock.h"
// Drivers
#include "temperature.h"
#include "core.h"
#include "can.h"
#include "memory.h"
#include "display.h"
#include "sensor.h"
#include "canvas.h"
#include "text.h"
// Library
#include <math.h>

Error error;
//-----------------------------------------------------
Error::Error(void)
{
    core.ClassInit(this,sizeof(this));
}
//-----------------------------------------------------
void Error::viewScreen(void)
{
    char str[28];
    uint8_t n;
    
    canvas.writeFillRect(0,0,320,240,display.COLOR_BACK);                               // вывод заднего фона
    n = text.charToString(str, "System fault");
    str[n++] = 0;
    text.writeString(160-11*(n/2),10,str,Font_11x18,display.WHITE,display.COLOR_BACK);
    display.switchOn();
}
//-----------------------------------------------------
uint8_t Error::viewHandler(void)
{
    uint8_t result;
    
    this->viewMode();                           // вывод на экран режима работы подогревателя
    result = this->sensorCheck();               // обработка касания сенсорного экрана
    if (result==1 && hcu.faultCode==48) {
        //---//measure.setVal((char*)HCU_CANCEL_RECOVERY, (uint8_t)(!measure.getVal((char*)HCU_CANCEL_RECOVERY)));
    }
    if (hcu.faultCode == 0) result = 1;
    
    return result;
}
//-----------------------------------------------------
void Error::viewError(uint8_t code)
{
    uint8_t x, y, n, i, j;
    char str[20];
    
    for (j=0; j<28; j++){
        if (errors[j].code == code){
            n=0;
            i = 0;
            x = 0;
            while(1){
                if (errors[j].text[i] == ' ') x++;
                if (errors[j].text[i] == 0) break;
                i++;
            }
            if (x == 0) y = 130;
            else if (x == 1) y = 120;
            else if (x == 2) y = 110;
            for (x=0; x<19; x++) str[x] = 0;
            i = 0;
            while(1){
                if (errors[j].text[i] == ' ' || errors[j].text[i] == 0){
                    text.writeStringOver(160-(Font_11x18.width*n/2),y-Font_11x18.height/2,str,Font_11x18,display.COLOR_ERROR,display.COLOR_BACK, 0);
                    y += 20;
                    n=0;
                    for (x=0; x<19; x++) str[x] = 0;
                    if (errors[j].text[i] == 0 || y >= 170) break;
                    i++;
                }
                else{
                    str[n] = errors[j].text[i++];
                    if (str[n] == '_') str[n] = ' ';
                    n++;
                }
            }
            return;
        }
    }
    
    n=0;
    for (x=0; x<19; x++) str[x] = 0;
    n += text.decToString(&str[n], code);
    text.writeStringOver(160-(Font_11x18.width*5/2),120-Font_11x18.height/2,"ERROR",Font_11x18,display.COLOR_ERROR,display.COLOR_BACK,0);
    
    text.writeStringOver(160-(Font_16x26.width*3/2),146-Font_16x26.height/2,"   ",Font_16x26,display.COLOR_ERROR,display.COLOR_BACK,0);
    text.writeStringOver(160-(Font_16x26.width*n/2),146-Font_16x26.height/2,str,Font_16x26,display.COLOR_ERROR,display.COLOR_BACK,0);
}
//-----------------------------------------------------
void Error::viewMode(void)
{
    char str[28];
    uint8_t n;
    
    if (hcu.faultCode != error.codeOld){
        error.codeOld = hcu.faultCode;
        if (hcu.faultCode > 0){
            n = text.charToString(str, "System fault, code ");
            n += text.decToString(&str[n], hcu.faultCode);
            str[n++] = ' ';
            str[n++] = ' ';
            str[n++] = 0;
            text.writeString(160-11*(n/2),10,str,Font_11x18,display.WHITE,display.COLOR_BACK);
            
            canvas.loadImageEffect(104,74,BUTTON_ERROR_IMAGE,8,0);
            viewError(hcu.faultCode);
        }
    }
}
//-----------------------------------------------------
uint8_t Error::sensorCheck(void)
{
    uint8_t result = 0;
    
    if (sensor.status == 1 && sensor.touch == 0){   // касание сенсорного экрана
        sensor.touch = 1;
        
        if (1){
            if (slider.touch==0 && 
                sensor.x1>120 && sensor.x1<200 && sensor.y1>90 && sensor.y1<170){   // касание кнопки запуска/остановки
                sensor.touch = 2;
                buttonPress();
                result = 1;
            }
        }
        
    }
    else if (sensor.status == 0){   // отпускание сенсорного экрана
        
        sensor.touch = 0;
        slider.touch = 0;
        slider.mode = 0;
        core.setTimer(0);
        sensor.isTouchElement = FALSE;
    }
    return result;
}
//-----------------------------------------------------
void Error::buttonPress(void)
{
    hcu.faultCodePanel = 0;
    if (hcu.isConnect==hcu.CONNECT_STATUS_OK && core.getTimer()==0){
        if (hcu.faultCode!=0 && hcu.isError==0){
            hcu.stageOld = 0;
            hcu.isError = 1;
        }
        else{
            hcu.isError = 0;
            //heater.errorOld = 0;
            //heater.error = 0;
        }
                    
    }
    else if (hcu.isConnect==hcu.CONNECT_STATUS_ERROR){
        ///hcu.linkCnt = 1;
        hcu.isConnect = hcu.CONNECT_STATUS_SEARCH;
        hcu.faultCodeHcu = 0;
        usart.answerTimeOut = 1;
        usart.linkCnt = 1;
        error.codeOld = ~hcu.faultCodeHcu;
    }
}
//-----------------------------------------------------
