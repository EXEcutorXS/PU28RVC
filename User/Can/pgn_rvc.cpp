/******************************************************************************
* ООО Теплостар
* Самара
*
* Программисты:
*
* __.01.2021
* Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "pgn_rvc.h"
#include "core.h"
#include "can.h"
#include "hcu.h"
#include "display.h"
#include "system.h"
#include "temperature.h"
#include "rvc.h"
#include "main.h"

CAN_PGN_RVC canPGNRVC;

//-----------------------------------------------------
CAN_PGN_RVC::CAN_PGN_RVC(void)
{
	minutesSinceStart=0;
}
//-----------------------------------------------------
uint32_t CAN_PGN_RVC::generateAID(uint8_t priority, uint32_t DGN, uint8_t SA)
{
    return SA|(DGN&0x1FFFF)<<8|(priority&0x7)<<26;
}
//-----------------------------------------------------
void CAN_PGN_RVC::msgWaterHeater(void)
{
    uint8_t instance;
    uint8_t operatingMode;
    uint16_t setPointTemperature;
    uint16_t waterTemperature;
    uint8_t thermostatStatus;
    uint8_t burnerStatus;
    uint8_t acElementStatus;
    uint8_t highTemperatureLimitStatus;
    uint8_t failureToIgniteStatus;

    instance = 1;
    operatingMode = ((air.isEHeaterOn&0x01)<<1) | (air.isFHeaterOn&0x01);
    setPointTemperature = (90+273)*32;
    waterTemperature = (heatExchangerTemperaturex10C+2730)*3.2;
    thermostatStatus = 3;
    burnerStatus = hcu.stateHeater&0x01;
    acElementStatus = hcu.stateAch&0x01;
    highTemperatureLimitStatus = 3;
    failureToIgniteStatus = (hcu.faultCodeHeater==13);
    uint32_t pgn = generateAID(6, 0x1FFF7, 101);
    can.SendMessage(pgn,
                    instance,
                    operatingMode,
                    setPointTemperature,
                    setPointTemperature>>8,
                    waterTemperature,
                    waterTemperature>>8,
                    thermostatStatus | (burnerStatus<<2) | (acElementStatus<<4) | (highTemperatureLimitStatus<<6),
                    failureToIgniteStatus);
}

void CAN_PGN_RVC::msgWaterHeater2(void)
{
    can.SendMessage(generateAID(6, 0x1FE99, 101), 1, 0xFF, 0x3F+(!rvc.newState.DomesticWaterPriority<<6), 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
}
//-----------------------------------------------------
void CAN_PGN_RVC::msgCirculationPumpStatus(void)
{
    uint32_t pgn = generateAID(6, 0x1FE97, 101);
    can.SendMessage(pgn,
                    1,
                    0xF0 | rvc.newState.PumpState,
                    0xFF,
                    0xFF,
                    0xFF,
                    0xFF,
                    0xFF,
                    0xFF);
}
//-----------------------------------------------------
void CAN_PGN_RVC::msgDiagnosticMessage(void)
{
    uint8_t operatingStatus1;
    uint8_t operatingStatus2;
    uint8_t yellowLampStatus;
    uint8_t redLampStatus;
    uint8_t dsa;
    uint8_t spnMsb = 0xFF;
	uint8_t spnIsb = 0xFF;
	uint8_t spnLsb = 0xFF;
    uint8_t fmi;
    uint8_t occurenceCount;
    uint8_t reserved;
    uint8_t dsaExtension;
    uint8_t bankSelect;


    operatingStatus1 = !(hcu.faultCodeHcu || hcu.faultCodeHeater || hcu.faultCodePanel);  // "Enabled" if we don't have any error codes
    operatingStatus2 = hcu.stateHeater || hcu.stateAch || rvc.newState.FanCurrentSpeed; //Device is running if any of these units is active
    yellowLampStatus = 0;
    if (hcu.faultCodeHcu || hcu.faultCodeHeater || hcu.faultCodePanel)
        redLampStatus = 1;
    else
        redLampStatus = 0;

    
    fmi = 0xFF;
    occurenceCount = 0x7F;
    reserved = 1;
    if (hcu.faultCode!=0)
	{
        spnMsb = hcu.faultCode;
		spnIsb = 1;   //Instance
	}
	
    dsa = 101;//Water heater address
    dsaExtension = 0xFF;//Water heater address
    bankSelect = 0xFF;
    uint32_t pgn = generateAID(6, 0x1FECA, 101);
    can.SendMessage(pgn,
                    operatingStatus1 | (operatingStatus2<<2) | (yellowLampStatus<<4) | (redLampStatus<<6),
                    dsa,
                    spnMsb,
                    spnIsb,
                    ((spnLsb&7)<<5) | fmi,
                    occurenceCount | (reserved<<7),
                    dsaExtension,
                    bankSelect);

}
//-----------------------------------------------------
void CAN_PGN_RVC::msgFurnace()
{

    uint32_t pgn = generateAID(6, 0x1FFE4, 101);
    can.SendMessage(pgn,
                    1,
                    rvc.newState.FanManualMode | 0xFC,
                    rvc.newState.FanCurrentSpeed*2,
                    0xFF,
                    0xFF,
                    0xFF,
                    0xFF,
                    0xFF);

}
//-----------------------------------------------------
void CAN_PGN_RVC::msgThermostat1()
{
    uint8_t instance;
    uint8_t operatingMode;
    uint8_t fanMode;
    uint8_t scheduleMode;
    uint8_t fanSpeed;
    uint16_t setpointHeat;
    uint16_t setpointCool;


    instance = 1;
    if (air.isAirOn) {
        operatingMode = 2;
    }
    else {
        operatingMode = 0;
    }
    fanMode = 3;
    scheduleMode = 1;
    fanSpeed = 0xFF;

    setpointHeat = (uint16_t)((rvc.newState.currentSetpoint+273.0f)*32.0f);
    setpointCool = 0xFFFF;
    uint32_t pgn = generateAID(6, 0x1FFE2, 101);
    can.SendMessage(pgn,
                    instance,
                    operatingMode | (fanMode<<4) | (scheduleMode<<6),
                    fanSpeed,
                    setpointHeat,
                    setpointHeat>>8,
                    setpointCool,
                    setpointCool>>8,
                    0xFF);
}
//-----------------------------------------------------
void CAN_PGN_RVC::msgThermostat2()
{
    uint8_t currentScheduleInstance;
    uint8_t numberScheduleInstance;

    currentScheduleInstance = air.isDay;
    numberScheduleInstance = 2;     //total instances

    uint32_t pgn = generateAID(6, 0x1FEFA, 101);
    can.SendMessage(pgn,
                    1,
                    currentScheduleInstance,
                    numberScheduleInstance,
                    0xFF,
                    0xFF,
                    0xFF,
                    0xFF,
                    0xFF);

}
//-----------------------------------------------------
void CAN_PGN_RVC::msgThermostatSchedule1( uint8_t scheduleInstance)
{
    uint8_t instance;
    uint8_t startHour;
    uint8_t startMinute;
    float setpointHeat;
    uint16_t setpointCool;
    uint16_t preciseSetpoint;

    if (scheduleInstance < 2) {
        instance = 1;

        switch(scheduleInstance) {
        case 0:
            setpointHeat = rvc.newState.setpointNight;
            startHour = air.nightTimeH;
            startMinute = air.nightTimeM;
            break;
        case 1:
            setpointHeat = rvc.newState.setpointDay;
            startHour = air.dayTimeH;
            startMinute = air.dayTimeM;
            break;
        }

        preciseSetpoint = ((setpointHeat+273.0f)*32.0f);
        setpointCool = 0xFFFF;
        uint32_t pgn = generateAID(6, 0x1FEF7, 101);
        can.SendMessage(pgn,
                        instance,
                        scheduleInstance,
                        startHour,
                        startMinute,
                        preciseSetpoint,
                        preciseSetpoint>>8,
                        setpointCool,
                        setpointCool>>8);
    }
}

//-----------------------------------------------------
void CAN_PGN_RVC::msgAmbientTemp()
{
    uint8_t instance;
    uint16_t temperatureAmbient;


    instance = 1;
    int16_t temp;
    if (!air.isPanelSensor && airTemperaturex10C>-200 && airTemperaturex10C<600) {
		temp = canPGNRVC.airTemperaturex10C;
    }
    else
    {
		temp = canPGNRVC.panelSensorx10C;
    }
	
    temperatureAmbient = (temp+2730)*3.2;
    uint32_t pgn = generateAID(6, 0x1FF9C, 101);
    can.SendMessage(pgn,
                    instance,
                    temperatureAmbient,
                    temperatureAmbient>>8,
                    0xFF,
                    0xFF,
                    0xFF,
                    0xFF,
                    0xFF);

}

void CAN_PGN_RVC::msgExtMessage()
{

    uint16_t tankTemperature = (tankTemperaturex10C+2730)*3.2;
	uint16_t heaterTemperature = (heaterTemperaturex10C+2730)*3.2;
    uint32_t pgn = generateAID(6, 0x1EF65, 101);
    can.SendMessage(pgn,
                    0x84,
                    0xF0 | (rvc.newState.Solenoid&1) | ((rvc.newState.UsePanelSensor&1)<<2),
                    tankTemperature,
                    tankTemperature>>8,
                    heaterTemperature,
                    heaterTemperature>>8,
                    hcu.fanPower*2,
                    0xFF);

}

void CAN_PGN_RVC::msgTimers()
{
	uint32_t systemTimer;
	uint16_t waterTimer;
	uint16_t pumpTimer;
	
	if (!rvc.newState.HeaterState && !rvc.newState.ElementState)
		systemTimer=0;
	else if (hcu.durationSystem<7200)
		systemTimer = (uint32_t)hcu.durationSystem*60 - (core.getTick()-hcu.timerOffSystem)/1000;
	else
		systemTimer = 432000;
		
		
	if (air.isWaterOn)
		waterTimer = hcu.durationDomesticWater*60-(core.getTick()-hcu.timerOffDomesticWater)/1000;
	else
		waterTimer = 0;
	
	if (rvc.newState.PumpState==5)
		pumpTimer = 600 - (core.getTick()-hcu.timerPumpOn)/1000;
	else
		pumpTimer = 0;
	
    uint32_t pgn = generateAID(6, 0x1EF65, 101);
    can.SendMessage(pgn,
                    0x85,
                    systemTimer,
                    systemTimer>>8,
					systemTimer>>16,
                    waterTimer,
                    waterTimer>>8,
                    pumpTimer,
                    pumpTimer>>8);

}


void CAN_PGN_RVC::msgHeaterInfo()
{
	uint32_t systemTime = hcu.heaterTotalOperatingTime;
    uint32_t pgn = generateAID(6, 0x1EF65, 101);
    can.SendMessage(pgn,
                    0x86,
                    systemTime,
                    systemTime>>8,
                    systemTime>>16,
                    hcu.heaterVersion[0],
                    hcu.heaterVersion[1],
	                hcu.heaterVersion[2],
	                hcu.heaterVersion[3]);
}

void CAN_PGN_RVC::msgPanelInfo()
{
    uint32_t pgn = generateAID(6, 0x1EF65, 101);
    can.SendMessage(pgn,
                    0x87,
                    minutesSinceStart,
                    minutesSinceStart>>8,
                    minutesSinceStart>>16,
                    *(__IO uint8_t*)(CRC_FIRMWARE_ADDRESS+6),
                    *(__IO uint8_t*)(CRC_FIRMWARE_ADDRESS+7),
	                *(__IO uint8_t*)(CRC_FIRMWARE_ADDRESS+8),
	                *(__IO uint8_t*)(CRC_FIRMWARE_ADDRESS+9));
}

void CAN_PGN_RVC::msgHcuInfo()
{
    uint32_t pgn = generateAID(6, 0x1EF65, 101);
    can.SendMessage(pgn,
                    0x88,
                    0xFF,
                    0xFF,
                    0xFF,
                    hcu.version[0],
                    hcu.version[1],
	                hcu.version[2],
	                hcu.version[3]);
}

void CAN_PGN_RVC::msgTimersSetupStatus()
{
    uint32_t pgn = generateAID(6, 0x1EF65, 101);
    can.SendMessage(pgn,
                    0x8A,
                    hcu.durationSystem&0xFF,
					(hcu.durationSystem>>8)&0xFF,
                    hcu.durationDomesticWater,
                    0xFF,
                    0xFF,
	                0xFF,
	                0xFF);
}

