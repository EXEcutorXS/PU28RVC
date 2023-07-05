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
bool ZoneEnabled;
bool UsePanelSensor;
	
uint8_t PumpState;
uint8_t errorCode;
uint8_t FanManualMode;
uint8_t FanManualSpeed;
uint8_t FanCurrentSpeed;

	uint8_t DayStartHour;
	uint8_t DayStartMinute;
	uint8_t NightStartHour;
	uint8_t NightStartMinute;
	
	uint16_t SystemDuration;
	uint16_t WaterDuration;
	
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
		bool externalTemperatureProvidedChanged;
		float externalTemperature;
		
    private:
		uint32_t lastExtTempGetTick;		

};

extern RVCModule rvc;

#endif /* __RVC_H */
