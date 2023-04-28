/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SENSOR_H
#define __SENSOR_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Classes ------------------------------------------------------------------*/
class Sensor
{
    public:
        void initialize(void);
        uint8_t handler(void);
        void initButton(void);
        void testProg(void);
        uint32_t getTickTouch(void);
        void resetTickTouch(void);
        void extiIrqHandler(void);
        void extiBtnHandler(void);
        uint8_t write(uint8_t* reg, uint8_t address, uint8_t len);
    
        bool isPress;
        uint8_t button;
        uint8_t touch;
        uint8_t buttonMode;
        uint32_t tickWhenTouching;
        uint32_t tickWhenPress;
        uint32_t timerPress;
        bool isTouchElement;

        uint8_t gesture;
        uint8_t status;
        uint8_t periodMonitor;
        int16_t x1, y1;
        int16_t x2, y2;
        uint8_t event1, event2;
        
        uint8_t screenSetup;
        
        static const int WIDTH = 320;
        static const int HEIGHT = 240;
        
    private:
        uint8_t read(uint8_t* reg, uint8_t address, uint8_t len);
        
    
        uint8_t p1[6], p2[6];
        uint32_t backStep;
};
extern Sensor sensor;

#endif /* __SENSOR_H */
