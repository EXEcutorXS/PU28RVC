/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ERROR_H
#define __ERROR_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Defines ------------------------------------------------------------------*/

/* Classes ------------------------------------------------------------------*/
class Error
{
    public:
        Error(void);
        void viewScreen(void);
        uint8_t viewHandler(void);
        void viewError(uint8_t code);
        void viewMode(void);
        uint8_t sensorCheck(void);
        void buttonPress(void);
        void ClearErrors(void);
        uint8_t codeOld;
        
    private:
        
};
extern Error error;

/* Info ------------------------------------------------------------------*/

#endif /* __ERROR_H */
