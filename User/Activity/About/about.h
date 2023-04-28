/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ABOUT_H
#define __ABOUT_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Defines ------------------------------------------------------------------*/

/* Classes ------------------------------------------------------------------*/
class About
{
    public:
        About(void);
        void viewScreen(void);
        uint8_t viewHandler(void);
        uint8_t sensorCheck(void);
        
    private:
        
};
extern About about;

/* Info ------------------------------------------------------------------*/

#endif /* __ABOUT_H */
