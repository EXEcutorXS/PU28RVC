/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BLE_CONNECT_H
#define __BLE_CONNECT_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"

/* Defines ------------------------------------------------------------------*/

/* Classes ------------------------------------------------------------------*/
class BleConnect
{
    public:
        BleConnect(void);
        void viewScreen(void);
        uint8_t viewHandler(void);
        
    private:
        
};
extern BleConnect bleConnect;

/* Info ------------------------------------------------------------------*/

#endif /* __BLE_CONNECT_H */
