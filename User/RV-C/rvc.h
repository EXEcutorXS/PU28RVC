/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RVC_H
#define __RVC_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"

/* Classes ------------------------------------------------------------------*/
class RVCModule
{
    public:
        RVCModule(void);
        void handler(void);
        void TransmitMessage(void);
        void ProcessMessage(uint8_t MsgNum);

    private:
};

extern RVCModule rvc;

#endif /* __RVC_H */
