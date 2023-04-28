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
#include "sleep.h"
#include "system.h"
#include "error.h"
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
#include "usart.h"
#include "can.h"
#include "memory.h"
#include "display.h"
#include "sensor.h"
#include "canvas.h"
#include "text.h"

Sleep sleep;
//-----------------------------------------------------
Sleep::Sleep(void)
{
    
}
//-----------------------------------------------------
uint8_t Sleep::checkSleepMode(void)
{
    if (display.setup.displaySleep == 1){   // сон возможен только при разрешении
        if (this->handler() == 0){
            if (display.getTimer() == 0){
                return 1;
            }
        }
    }
    return 0;   // условий для сна нет
}
//-----------------------------------------------------
void Sleep::viewScreen(void)
{
    if (display.setup.viewClock == 1){        // включаем часы 
        this->isViewClock = true;
        clock.weekDayOld = 0xFF;
        clock.sensorClockOld = clock.sensorClock;
        clock.sensorClock = 0;
        clock.viewAnClock(2);
    }
    else {                            // отключаем дисплей
        this->isViewClock = false;
        clock.sensorClockOld = clock.sensorClock;
        display.turnOff();
        display.setLight(0);
        
        setLowPower(true);
    }
    this->isWakeUp = false;
}
//-----------------------------------------------------
uint8_t Sleep::handler(void)
{
    uint8_t result = 0;
    static uint8_t stageTemp = 0;
    
    result = sensor.handler()/* & sensor.status*/;                      // касание сенсорного экрана
  
    if (gpio_input_bit_get(GPIOA, GPIO_PIN_0) != RESET){     // нажатие механической кнопки
        result = 1;
    }
    if (hcu.faultCode != error.codeOld){
        result = 1;
    }
    if (hcu.stage != stageTemp){
        stageTemp = hcu.stage;
        result = 1;
    }
    if (screen_visible == SCREEN_VISIBLE_SLEEP){
        if (this->isViewClock == true){
            clock.viewAnClockSecond();
            clock.viewAnClockMinute();
            clock.viewAnClockHour(&clock.isPm, &clock.weekDay, 2);
            if (clock.hour >= 12) clock.isPm = 1;
            else clock.isPm = 0;
            clock.positionHour = clock.hour-(clock.isPm*12);
            clock.positionMinute = clock.minute;
            clock.positionSecond = clock.second;
            clock.viewCenterDigClock(3);
            core.delayMs(20);
        }
    }
    if (this->isWakeUp){
        this->isWakeUp = false;
        result = 1;
    }
    
    return result;
}
//-----------------------------------------------------
void Sleep::setLowPower(bool isEnable)
{
    if (isEnable){
        isLowPower = true;
        SystemCoreClockUpdate();
        uint32_t ahb_presc = RCU_CFG0 & RCU_CFG0_AHBPSC;
        switch(ahb_presc){
            case RCU_AHB_CKSYS_DIV1:
                SystemCoreClock = SystemCoreClock;
                break;
            case RCU_AHB_CKSYS_DIV2:
                SystemCoreClock = SystemCoreClock / 2;
                break;
            case RCU_AHB_CKSYS_DIV4:
                SystemCoreClock = SystemCoreClock / 4;
                break;
            case RCU_AHB_CKSYS_DIV8:
                SystemCoreClock = SystemCoreClock / 8;
                break;
            case RCU_AHB_CKSYS_DIV16:
                SystemCoreClock = SystemCoreClock / 16;
                break;
            case RCU_AHB_CKSYS_DIV64:
                SystemCoreClock = SystemCoreClock / 64;
                break;
            case RCU_AHB_CKSYS_DIV128:
                SystemCoreClock = SystemCoreClock / 128;
                break;
            case RCU_AHB_CKSYS_DIV256:
                SystemCoreClock = SystemCoreClock / 256;
                break;
            case RCU_AHB_CKSYS_DIV512:
                SystemCoreClock = SystemCoreClock / 512;
                break;
        }
        SysTick_Config(SystemCoreClock/1000);
        NVIC_SetPriority(SysTick_IRQn, 0);
        usart.changeBaudrate(usart.baudrate);
        //sensor.disableI2c();
    }
    else{
        isLowPower = false;
        SystemCoreClockUpdate();
        SysTick_Config(SystemCoreClock/1000);
        NVIC_SetPriority(SysTick_IRQn, 0);
        usart.changeBaudrate(usart.baudrate);
        display.setTimer(display.setup.timeout*1000);
        //sensor.enableI2c();
    }
}
//-----------------------------------------------------
