#include "main.h"
#include "system.h"
#include "backup.h"
#include "hcu.h"

Backup backup;

void Backup::init()
{
    RCU_APB1EN |= RCU_APB1EN_PMUEN;
    RCU_APB1EN |= RCU_APB1EN_BKPIEN;

	
	initFlag = (uint16_t*)0x40006C04;
	heaterButtonFlag = (uint16_t*)0x40006C08;
	elementButtonFlag =(uint16_t*)0x40006C0C;
	airOnFlag =(uint16_t*)0x40006C10;
	waterButtonFlag  =(uint16_t*)0x40006C14;
	pumpManualButtonFlag  =(uint16_t*)0x40006C18;
	fanAutoButtonFlag  =(uint16_t*)0x40006C1C;
	dayTimeSetpoint  =(uint16_t*)0x40006C20;
	nightTimeSetpoint  =(uint16_t*)0x40006C24;
	reloadCounter  =(uint16_t*)0x40006C28;
	systemElapsedMinutes  =(uint16_t*)0x40006C40;
	waterElapsedSeconds  =(uint16_t*)0x40006C44;
	pumpElapsedSeconds  =(uint16_t*)0x40006C48;
	lastErrors1  =(uint16_t*)0x40006C4C;
	lastErrors2 =  (uint16_t*)0x40006C50;
	lastErrors3  =(uint16_t*)0x40006C54;
	lastErrors4  =(uint16_t*)0x40006C58;
	lastErrors5  =(uint16_t*)0x40006C5C;
	
    if (*initFlag==0xC0FF)
    {
        (*reloadCounter)++;
		restoreData();
    }
	    PMU_CTL    |= PMU_CTL_BKPWEN;
}

void Backup::handler()
{

    *initFlag=0xC0FF;
    *heaterButtonFlag = air.isFHeaterOn?0xC0FF:0;
    *elementButtonFlag = air.isEHeaterOn?0xC0FF:0;
    *airOnFlag = air.isAirOn?0xC0FF:0;
    *waterButtonFlag = air.isWaterOn?0xC0FF:0;
    *pumpManualButtonFlag = hcu.pumpOn?0xC0FF:0;
	if ((air.isEHeaterOn || air.isFHeaterOn) && hcu.durationSystem<7200)
		*systemElapsedMinutes = (core.getTick() - hcu.timerOffSystem )/60000;
	else
		*systemElapsedMinutes=0;
	if (hcu.pumpOn)
		*pumpElapsedSeconds = (core.getTick() - hcu.timerPumpOn)/1000;
	else
		*pumpElapsedSeconds=0;
	if (air.isWaterOn)
    *waterElapsedSeconds = (core.getTick()-hcu.timerOffDomesticWater)/1000;
	else
		*waterElapsedSeconds = 0;
}

void Backup::restoreData()
{
    air.isFHeaterOn = *heaterButtonFlag==0xC0FF;
    air.isEHeaterOn = *elementButtonFlag==0xC0FF;
    air.isAirOn = *airOnFlag == 0xC0FF;
    air.isWaterOn = *waterButtonFlag == 0xC0FF;
    hcu.pumpOn = *pumpManualButtonFlag == 0xC0FF;
    if (air.isFHeaterOn || air.isEHeaterOn)
		hcu.timerOffSystem = core.getTick()-*systemElapsedMinutes*60000;
	if (hcu.pumpOn)
		hcu.timerPumpOn = core.getTick()-*pumpElapsedSeconds*1000;
	if (air.isWaterOn)
		hcu.timerOffDomesticWater = core.getTick()-*waterElapsedSeconds*1000;
}

void Backup::addErrorToLog(uint16_t error)
{
*lastErrors5=*lastErrors4;
*lastErrors4=*lastErrors3;
*lastErrors3=*lastErrors2;
*lastErrors2=*lastErrors1;
*lastErrors1 = error;
}
