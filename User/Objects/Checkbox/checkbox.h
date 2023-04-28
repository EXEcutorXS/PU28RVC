/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CHECKBOX_H
#define __CHECKBOX_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Classes ------------------------------------------------------------------*/
class Checkbox
{
    public:
        Checkbox(void);
        void draw(uint16_t x, uint16_t y, const char* str, char* flag);
        void draw(uint16_t x, uint16_t y, const char* str, bool flag);
        
        static const int SELECTOR_STEP = 408;
        static const int COLOR_TEXT = 0xE0E0E0;
        static const int COLOR_ON = 0xFFFFFF;
        static const int COLOR_OFF = 0xC0C0C0;
        static const int COLOR_BACK = 0x000000;
};
extern Checkbox checkbox;

/* Info ------------------------------------------------------------------*/

#endif /* __CHECKBOX_H */
