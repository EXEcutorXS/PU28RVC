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
#include "canext.h"
//#include "stm32f2xx.h""
#include "adc.h"
#include "heater.h"
#include "zone.h"
#include "control.h"
#include "paramsname.h"
#include "errors.h"
#include "monitor.h"

CAN_PGN_RVC canPGNRVC;

//-----------------------------------------------------
CAN_PGN_RVC::CAN_PGN_RVC(void)
{
	
}
//-----------------------------------------------------
uint32_t CAN_PGN_RVC::generateAID(uint8_t priority, uint32_t DGN, uint8_t SA)
{
	return SA|(DGN&0x1FFFF)<<8|(priority&0x7)<<26;
}

void CAN_PGN_RVC::msgWaterHeater(void)
{
    uint8_t S=(Control.ElementState()<<1);
    if (Heater.Stage!=STAGE_Z) S=S+1;
    uint16_t Ts=(uint16_t)((Control.TliquidSetpoint+273)*32);
    uint16_t Tl=(uint16_t)((Heater.Tliquid+273)*32);
    uint8_t B6=1;
    if (adc.Ttank>=Control.TtankSetpoint) B6=0;                        //температура бака достигла уставки
    if (Heater.Stage==STAGE_H || Heater.Stage==STAGE_W) B6=4;
    B6=B6+(Control.ElementState()<<4);
    if (Heater.Stage==STAGE_P && Heater.Mode==P_MODE_4_WAITING) B6=B6+64;
    uint8_t B7=0;
    if (Heater.FaultCode==13) B7=1;
    if (Heater.FaultCode==15 || Heater.FaultCode==12) B7=B7+64;
    canExt.SendMessage(generateAID(6,0x1FFF7,88),1,S,Ts,Ts>>8,Tl,Tl>>8,B6,B7);
}
//-----------------------------------------------------
void CAN_PGN_RVC::msgCirculationPumpStatus(void)
{
    canExt.SendMessage(generateAID(6,0x1FE97,88),1,0xF0+Heater.PumpStatus_f,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);
}
//-----------------------------------------------------
void CAN_PGN_RVC::msgDiagnosticMessage(void)
{
    if (Heater.FaultCode==0 && Error.FaultCode==0) return;
    uint8_t OS, DSA, Err;
    if (Heater.FaultCode || Error.FaultCode) OS=4+64;                //64 - зажгли красную лампу
    else {
        if (Heater.Stage==STAGE_Z) OS=1;
        else OS=5;
    }
    if (Heater.FaultCode) {
        DSA=102;
        Err=Heater.FaultCode;
    }
    else {
        DSA=96;
        Err=Error.FaultCode;
    }
    canExt.SendMessage(generateAID(6,0x1FECA,88),OS,DSA,0,0,Err,128+0x7F,0xFF,0xFF);
}
//-----------------------------------------------------
void CAN_PGN_RVC::msgFurnace(uint8_t ZoneNum)
{
    if (ZoneNum>=ZONE_COUNT) return;

    if (Zone.Connected_f[ZoneNum])
        canExt.SendMessage(generateAID(6,0x1FFE4,88),ZoneNum+1,252+Zone.FanManual_f[ZoneNum],Zone.CurrentPercents[ZoneNum]*2,Zone.CurrentPercents[ZoneNum]*2,0,Zone.FansDTStart*10,0xFF,0xFF);
}
//-----------------------------------------------------
void CAN_PGN_RVC::msgThermostat1(uint8_t ZoneNum)
{
    if (ZoneNum>=ZONE_COUNT) return;

    uint8_t StateMask=0;
		uint8_t AlwaysOnMask = Zone.FanModeAlwaysOn[ZoneNum]*0x10;
    if (Zone.State[ZoneNum]>0) StateMask=2;

    if (Zone.Connected_f[ZoneNum]) {
        uint16_t W=(uint16_t)((Zone.GetTsetpoint(ZoneNum)+273)*32);
        canExt.SendMessage(generateAID(6,0x1FFE2,88),ZoneNum+1,0x40+StateMask+AlwaysOnMask,Zone.DefinedPercents[ZoneNum]*2,W,W>>8,0xFF,0xFF,0xFF);
    }
}

void CAN_PGN_RVC::msgThermostat2(uint8_t ZoneNum)
{
    if (ZoneNum>=ZONE_COUNT) return;
		
    if (Zone.Connected_f[ZoneNum]) {
			uint8_t currentThermostatSchedule=0;
			if (Monitor.IsNowADay())
					currentThermostatSchedule = 1;
			else
					currentThermostatSchedule = 0;
			if (Monitor.btnStorageMode_f)
				currentThermostatSchedule = 250;
        canExt.SendMessage(generateAID(6,0x1FEFA,88),ZoneNum+1,currentThermostatSchedule,3,0xFF,0xFF,0xFF,0xFF,0xFF);
    }
}

void CAN_PGN_RVC::msgThermostatSchedule1(uint8_t ZoneNum, uint8_t scheduleInstance)
{
		if (ZoneNum>=ZONE_COUNT)
			return;
		if (scheduleInstance>2)
			return;

		int temp=0;
		uint8_t startHour;
		uint8_t startMinute;
    if (Zone.Connected_f[ZoneNum]) {
			switch(scheduleInstance)
			{
			case 0: temp = Zone.TsetpointNight[ZoneNum]; 
				startHour = Monitor.rtc.Night.Hours;
				startMinute = Monitor.rtc.Night.Min;
				break; 
			case 1: temp = Zone.TsetpointDay[ZoneNum];
				startHour = Monitor.rtc.Morning.Hours;
				startMinute = Monitor.rtc.Morning.Min;
				break;
			default:
				scheduleInstance=250;
				startHour = 0xFF;
				startMinute = 0xFF;
				temp = 10;                                 //Storage Mode
			}
        uint16_t W=(uint16_t)((temp+273)*32);
        canExt.SendMessage(generateAID(6,0x1FEF7,88) ,ZoneNum+1,scheduleInstance,startHour,startMinute,W,W>>8,0xFF,0xFF);
    }
}

void CAN_PGN_RVC::msgThermostatSchedule2(uint8_t ZoneNum, uint8_t scheduleInstance)
{
		if (ZoneNum>=ZONE_COUNT)
			return;
		if (scheduleInstance>2)
			return;
		if (scheduleInstance==2)
			scheduleInstance=250;
    if (Zone.Connected_f[ZoneNum]) {
        canExt.SendMessage(generateAID(6,0x1FEF6,88),ZoneNum+1,scheduleInstance,0x55,0x15,0xFF,0xFF,0xFF,0xFF);
    }
}

void CAN_PGN_RVC::msgAmbientTemp(uint8_t ZoneNum)
{
    if (ZoneNum>=ZONE_COUNT) return;
		
    if (Zone.Connected_f[ZoneNum]) {
        uint16_t W=(uint16_t)((adc.Tzone[ZoneNum]+273)*32);
        canExt.SendMessage(generateAID(6,0x1FF9C,88),ZoneNum+1,W,W>>8,0xFF,0xFF,0xFF,0xFF,0xFF);
    }
}
