/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SEARCH_H
#define __SEARCH_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Defines ------------------------------------------------------------------*/

/* Classes ------------------------------------------------------------------*/
class Search
{
    public:
        Search(void);
        void viewScreen(void);
        uint8_t viewHandler(void);
        void viewConnect(bool isView);
        uint8_t sensorCheck(void);
        
    private:
        
};
extern Search search;

/* Info ------------------------------------------------------------------*/

#endif /* __SEARCH_H */
