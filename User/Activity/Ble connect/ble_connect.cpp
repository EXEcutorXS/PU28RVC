/******************************************************************************
* ООО Теплостар
* Самара
* 
* Программисты: Клюев А.А.
* 
* 29.05.2019
* Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
// Activity
#include "main.h"
#include "ble_connect.h"
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
#include "bluetooth.h"
#include "memory.h"
#include "display.h"
#include "sensor.h"
#include "canvas.h"
#include "text.h"
// Library
#include <math.h>

BleConnect bleConnect;
//-----------------------------------------------------
BleConnect::BleConnect(void)
{
    
}
//-----------------------------------------------------
void BleConnect::viewScreen(void)
{
    canvas.writeFillRect(0,0,320,240,display.COLOR_BACK);                               // вывод заднего фона
    text.writeString(160-11*13,10,"Connecting to a new device",Font_11x18,display.WHITE,display.COLOR_BACK);
    display.switchOn();
}
//-----------------------------------------------------
uint8_t BleConnect::viewHandler(void)
{
    uint8_t result;
    
    const int CHECKBOXS_S = 10;
    
    const int CHECKBOX1_X = 35;
    const int CHECKBOX1_Y = 85;
    const int CHECKBOX1_H = 40;
    const int CHECKBOX1_W = 200;
    
    const int CHECKBOX2_X = 35;
    const int CHECKBOX2_Y = (CHECKBOX1_Y+80);
    const int CHECKBOX2_H = 40;
    const int CHECKBOX2_W = 200;
    
    checkbox.draw(CHECKBOX1_X, CHECKBOX1_Y, "Add new device", false);
    
    checkbox.draw(CHECKBOX2_X, CHECKBOX2_Y, "Cancel", false);
    
    if (sensor.status == 1){   // касание сенсорного экрана
        //sensor.touch = 1;
        if ((sensor.x1>=(CHECKBOX1_X-20) && sensor.x1<=(CHECKBOX1_X+CHECKBOX1_W) && sensor.y1>=(CHECKBOX1_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX1_Y+CHECKBOX1_H+CHECKBOXS_S))){ // касание чекбокса 1
            sensor.touch = 2;
            result = 1;
        }  
        if ((sensor.x1>=(CHECKBOX2_X-20) && sensor.x1<=(CHECKBOX2_X+CHECKBOX2_W) && sensor.y1>=(CHECKBOX2_Y-CHECKBOXS_S) && sensor.y1<=(CHECKBOX2_Y+CHECKBOX2_H+CHECKBOXS_S))){ // касание чекбокса 2
            sensor.touch = 2;
            result = 2;
        }
        
    }
    return result;
}
//-----------------------------------------------------
