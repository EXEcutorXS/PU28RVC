/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RVC_H
#define __RVC_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Classes ------------------------------------------------------------------*/
typedef struct rvcState_t
{
bool HeaterState;
bool ElementState;
bool DomesticWaterPriority;
bool Solenoid;
	
uint8_t PumpState;
uint8_t errorCode;
uint8_t FanManual;
uint8_t FanSpeed;
float setpointDay;
float setpointNight;
float currentSetpoint;


}rvcState_t;

class RVCModule
{
    public:
        RVCModule(void);
        void handler(void);
        void TransmitMessage(void);
        void ProcessMessage(uint8_t MsgNum);
		rvcState_t oldState;
		rvcState_t newState;
		bool externalTemperatureProvided;
		float externalTemperature;
		
    private:
		uint32_t lastExtTempGetTick;		

};

extern RVCModule rvc;

#endif /* __RVC_H */
