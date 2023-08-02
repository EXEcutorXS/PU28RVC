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
#include "about.h"
#include "setup.h"      // исправить, нарушение иерархии классов!!!
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
// Library
#include <math.h>

About about;
//-----------------------------------------------------
About::About(void)
{
    core.ClassInit(this,sizeof(this));
}
//-----------------------------------------------------
void About::viewScreen(void)
{
    /*
    uint8_t y=45;
    const uint8_t X = 45;
    const uint8_t STEP_Y = 25;
    
    canvas.writeFillRect(0,0,320,240,display.COLOR_BACK);                               // вывод заднего фона
    text.writeString(160-11*12.5,10,"Your source for solutions",Font_11x18,display.RED,display.COLOR_BACK);
    
    canvas.drawRoundRect(5, 35, 316, 204, display.COLOR_BACK, display.LIGHTGREY, display.COLOR_BACK);
    canvas.drawRoundRect(5, 35, 315, 203, display.COLOR_BACK, display.WHITE, display.COLOR_BACK);
    
    text.writeString(X,y,"Telephone:",Font_11x18,display.RED,display.COLOR_BACK);
    y += STEP_Y;
    text.writeString(X,y,"360-608-0916",Font_11x18,display.WHITE,display.COLOR_BACK);
    y += STEP_Y+10;
    text.writeString(X,y,"E-mail:",Font_11x18,display.RED,display.COLOR_BACK);
    y += STEP_Y;
    text.writeString(X,y,"info@elwellcorp.com",Font_11x18,display.WHITE,display.COLOR_BACK);
    y += STEP_Y+10;
    text.writeString(X,y,"Address:",Font_11x18,display.RED,display.COLOR_BACK);
    y += STEP_Y;
    text.writeString(X,y,"680-C South 28th St.",Font_11x18,display.WHITE,display.COLOR_BACK);
    y += STEP_Y;
    text.writeString(X,y,"Washougal, WA 98671",Font_11x18,display.WHITE,display.COLOR_BACK);
    y += STEP_Y;
    
    display.switchOn();
    */
    
    display.turnOff();                                              // отключение дисплея
    canvas.loadImage(0,0,ABOUT_IMAGE);                              // загрузка начального изображения
    display.switchOn();                                             // включение дисплея
}
//-----------------------------------------------------
uint8_t About::viewHandler(void)
{
    uint8_t result;
    
    result = this->sensorCheck();               // обработка касания сенсорного экрана
    
    return result;
}
//-----------------------------------------------------
uint8_t About::sensorCheck(void)
{
    uint8_t result = 0;
    
    if (sensor.status == 1 && sensor.touch == 0){   // касание сенсорного экрана
        sensor.touch = 1;
        
        if (1){
            if (slider.touch==0/* && 
                sensor.x1>120 && sensor.x1<200 && sensor.y1>90 && sensor.y1<170*/){   // касание кнопки запуска/остановки
                sensor.touch = 2;
                result = 1;;
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
