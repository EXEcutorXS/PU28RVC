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
#include "search.h"
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

extern bool isTest;
Search search;
//-----------------------------------------------------
Search::Search(void)
{
 core.ClassInit(this,sizeof(this));   
}
//-----------------------------------------------------
void Search::viewScreen(void)
{
    canvas.writeFillRect(0,0,320,240,display.COLOR_BACK);                               // вывод заднего фона
    text.writeString(160-11*10,10,"Connecting to system",Font_11x18,display.WHITE,display.COLOR_BACK);
    text.writeString(0,240-26,"Test",Font_16x26,display.WHITE,display.COLOR_BACK);
    text.writeString(320-16*5,240-26,"Boot",Font_16x26,display.WHITE,display.COLOR_BACK);
    display.switchOn();
}
//-----------------------------------------------------
uint8_t Search::viewHandler(void)
{
    uint8_t result=0;
    
    this->viewConnect(true);                    // индикация процесса ожидания соединения с подогревателем
    result = this->sensorCheck();               // обработка касания сенсорного экрана
    
    return result;
}
//-----------------------------------------------------
void Search::viewConnect(bool isView)
{
    /*#define TAIL_NUM    20
    int8_t a, b;
    uint32_t c;
    static uint8_t i=0, e=0;
    static float x[TAIL_NUM], y[TAIL_NUM], f;
    static float rad = 3.14;
    float k;*/
    
    if (usart.linkCnt == 40){
        if (hcu.isConnect != hcu.CONNECT_STATUS_ERROR){
            hcu.faultCodeHcu = 20;
        }
        hcu.isConnect = hcu.CONNECT_STATUS_ERROR;
    }
    else if(usart.linkCnt == 0){
        if (hcu.isConnect == hcu.CONNECT_STATUS_SEARCH){
            canvas.writeFillRect(104, 74, 112, 112, display.COLOR_BACK);
        }
        hcu.isConnect = hcu.CONNECT_STATUS_OK;
    }
    
    if (hcu.isConnect != hcu.CONNECT_STATUS_SEARCH) return;
    if (!isView) return;
    /*
    if (hcu.answerTimeOut > 1) k = 1.0;
    else k = 0.0;
    
    x[i] = sin(rad)*48+160-3;
    y[i] = cos(rad)*48+130-3;
    i++;
    if (i >= TAIL_NUM) i = 0; 
    a=i;
    
    if (e < TAIL_NUM) e++;
    else
    {
        for (b=0; b<TAIL_NUM; b++){
            f = ((float)b/(TAIL_NUM-1))*k;
            c = (uint8_t)(0xFF*f)<<16;
            c += (uint8_t)(0xFF*f)<<8;
            c += (uint8_t)(0xFF*f);
            text.writeStringOver(x[a],y[a],"~",Font_7x10,c,display.COLOR_BACK,0);
            a++;
            if (a >= TAIL_NUM) a = 0;
        }
    }
    rad -= 0.3;
    if (rad < -3.14) rad = 3.14;
    */
}
//-----------------------------------------------------
uint8_t Search::sensorCheck(void)
{
    uint8_t result = 0;
    
    if (sensor.status == 1 && sensor.touch == 0){   // касание сенсорного экрана
        sensor.touch = 1;
        
        if (1){
            if (slider.touch==0 && 
                sensor.x1>(BUTTON_RIGHT_X) && sensor.x1<(BUTTON_RIGHT_X+BUTTON_RIGHT_SIZE) && 
                sensor.y1>(BUTTON_RIGHT_Y) && sensor.y1<(BUTTON_RIGHT_Y+BUTTON_RIGHT_SIZE)){  // касание кнопки бутлоадера
                sensor.touch = 0;
                *(__IO uint32_t*) (NVIC_VectTab_RAM + BOOT_FLAG_SHIFT) = 0x0016AA55;
                NVIC_SystemReset();
            }
            if (slider.touch==0 && 
                sensor.x1>(BUTTON_LEFT_X) && sensor.x1<(BUTTON_LEFT_X+BUTTON_LEFT_SIZE) && 
                sensor.y1>(BUTTON_LEFT_Y) && sensor.y1<(BUTTON_LEFT_Y+BUTTON_LEFT_SIZE)){  // касание кнопки тестового режима
                sensor.touch = 1;
                hcu.isConnect = hcu.CONNECT_STATUS_OK;
                hcu.faultCodeHcu = 0;
				isTest=true;
                //hcu.answerTimeOut = 0;
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
