/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BOOT_H
#define __BOOT_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"

/* Classes ------------------------------------------------------------------*/
class Boot
{
    public:
        void receiveInterrupt(uint8_t);
        void viewScreen(void);
        void handler(void);
    
        char mode;
        uint8_t rxFlag;
        uint8_t command;
        uint8_t image_bytes[256], 
            image_point1, 
            image_point2, 
            image_point;
        uint16_t image_w, 
            image_h;
        uint32_t image_addr, 
            image_count, 
            image_numb;
    
    private:

};
extern Boot boot;

#endif /* __BOOT_H */
