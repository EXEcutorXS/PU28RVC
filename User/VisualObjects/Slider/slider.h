/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SLIDER_H
#define __SLIDER_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"
#include "font.h"

/* Classes ------------------------------------------------------------------*/
class Slider
{
    public:
        Slider(void){
            position=VALUE_LEN-1;
            positionTemp = VALUE_LEN-1;
            positionTempOld = VALUE_LEN-1;
            distanceL = 50; // 25
            distanceM = 10; // 10
            distanceR = 60; // 40
            offset = 18;    // 18
        };
        void initialize(void);
        void viewScreen(void);
        bool setPosition(int16_t val);
        uint8_t viewPosition(void);
        void viewString(void);
        bool checkDigGrid(uint16_t pos);
        uint16_t getPosition(uint16_t* posX, uint16_t* posY, uint16_t pos);
        int8_t getCorrectXDigGrid(uint16_t pos, FontDef font, uint8_t len);
        void eraseDigGrid(void);
        void drawDigGrid(void);
        void viewDigLeft(uint16_t pos);
        void clearDigLeft(uint16_t pos);
        void viewDigRight(uint16_t pos);
        void clearDigRight(uint16_t pos);
        void textDigGrid(uint16_t pos, uint32_t color);
        uint8_t viewGrid(uint16_t pos, uint32_t N);
        uint8_t viewGridInvert(uint16_t pos, uint32_t N);
        bool viewGridSens(uint16_t pos, uint32_t N, bool isClear);
        void setVal(uint16_t pos);
        void draw(uint16_t x, uint16_t y, uint8_t z, char* text, uint16_t value, char unit, bool isDrawText, bool* isValue, bool isVisible);
        int16_t drawSlim(uint16_t x, uint16_t y, float z, const char* txt, uint16_t value, char unit, bool isDrawText, bool* isValue, bool isVisible, uint32_t color, uint8_t id);
        int16_t drawSlim(uint16_t x, uint16_t y, float z, const char* txt, bool isDrawText, bool* isValue, bool isVisible, uint32_t color, uint8_t id);
        void drawDouble(uint16_t x, uint16_t y, uint8_t z1, uint8_t z2, const char* txt, bool isDrawText, bool* isValue, uint8_t numSelectSlider);
        void draw(uint16_t x, uint16_t y, uint8_t z, const char* txt, bool isDrawText, bool* isValue, bool isVisible);
        bool setMinMax(int16_t offValue, uint8_t minValue, uint8_t maxValue, uint8_t smallStep, uint8_t bigStep);
        
        static const int VALUE_LEN = 400;   // было 504
        int16_t position;
        int16_t positionTemp;
        uint16_t positionTempOld;
        uint8_t touch, touchOld;
        uint8_t mode;
        int16_t old;
        uint16_t x[VALUE_LEN], y[VALUE_LEN];
        uint16_t valueX[VALUE_LEN], valueY[VALUE_LEN];
        int16_t values[VALUE_LEN];
        uint32_t timerSliderMin;
        uint8_t minValue;
    
    private:
        int8_t change;
        uint8_t maxValue;
        uint8_t stepValueSmall;
        uint8_t stepValueBig;
        uint8_t distanceL;
        uint8_t distanceM;
        uint8_t distanceR;
        uint8_t offset;
        bool isFirst;
        int16_t offValue;
};
extern Slider slider;

/* Info ------------------------------------------------------------------*/


#endif /* __SLIDER_H */
