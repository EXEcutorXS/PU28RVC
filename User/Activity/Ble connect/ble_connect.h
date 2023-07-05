/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BLE_CONNECT_H
#define __BLE_CONNECT_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Defines ------------------------------------------------------------------*/

/* Classes ------------------------------------------------------------------*/
class BleConnect
{
    public:
        BleConnect(void);
        void viewScreen(void);
        uint8_t viewHandler(void);
        
    private:
        static const int CHECKBOXS_S = 10;
    
        static const int CHECKBOX1_X = 35;
        static const int CHECKBOX1_Y = 85;
        static const int CHECKBOX1_H = 40;
        static const int CHECKBOX1_W = 200;
        
        static const int CHECKBOX2_X = 35;
        static const int CHECKBOX2_Y = (CHECKBOX1_Y+80);
        static const int CHECKBOX2_H = 40;
        static const int CHECKBOX2_W = 200;
        
};
extern BleConnect bleConnect;

/* Info ------------------------------------------------------------------*/

#endif /* __BLE_CONNECT_H */
