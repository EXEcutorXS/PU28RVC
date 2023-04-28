/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TEXT_H
#define __TEXT_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"
#include "font.h"

/* Classes ------------------------------------------------------------------*/
class Text
{
    public:
        Text(void);
        void writeChar(uint16_t x, uint16_t y, char ch, FontDef font, uint32_t color, uint32_t bgcolor);
        void writeCharOver(uint16_t x, uint16_t y, char ch, FontDef font, uint32_t color, uint32_t bgcolor, uint8_t mask);
        void writeString(uint16_t x, uint16_t y, const char* str, FontDef font, uint32_t color, uint32_t bgcolor);
        void writeStringCenter(uint16_t x, uint16_t y, const char* str, FontDef font, uint32_t color, uint32_t bgcolor);
        uint8_t getLenString(const char* str);
        void writeDigit(uint16_t x, uint16_t y, const char* str, uint32_t color, uint8_t isOver);
        void writeOneDigit(uint16_t x, uint16_t y, char str, uint32_t color, uint8_t isOver);
        void writeStringOver(int16_t x, int16_t y, char* str, FontDef font, uint32_t color, uint32_t bgcolor, uint8_t mask);
        void writeStringEnd(uint16_t x, uint16_t y, char* str, FontDef font, uint32_t color, uint32_t bgcolor);
        uint8_t decToString(char* str, int32_t D);
        uint8_t decToStringFix(char* str, int32_t D, uint8_t i);
        uint8_t floatToString (char* str, float X, uint8_t N);
        uint8_t charToString (char* str, const char* chr);
    
        char message[128];
        
    private:
        uint8_t codeAdd;
        
        
        
        
};
extern Text text;

/* Info ------------------------------------------------------------------*/

#endif /* __TEXT_H */
