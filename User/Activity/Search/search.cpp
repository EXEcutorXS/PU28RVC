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

    if (usart.linkCnt == LINK_ERROR_TIME){
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
