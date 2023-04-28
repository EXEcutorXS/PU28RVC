/******************************************************************************
* ООО Адверс
* Самара
*
* Программисты: .
*
* 17.07.2020
* Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "rvc.h"
#include "core.h"
#include "can.h"
#include "pgn_rvc.h"
#include "hcu.h"
#include "unix_time.h"
#include "system.h"
#include "display.h"
#include "slider.h"
#include "hcu.h"
#include "temperature.h"

RVCModule rvc;
//-----------------------------------------------------
RVCModule::RVCModule(void)
{
}
//-----------------------------------------------------
void RVCModule::handler(void)
{
    this->TransmitMessage();

    for (uint8_t i = 0; i < MSG_BUF_SIZE; i++) {
        if (can.RxMsgBuf[i].Received_f) {
            this->ProcessMessage(i);
            can.RxMsgBuf[i].Received_f = false;
        }
    }

    //updating newState
    if (hcu.pumpOn) {
        // Test
        newState.PumpState = 0x05;
    }
    else {
        newState.PumpState = hcu.statePump&0x01;
    }

    newState.HeaterState = air.isFHeaterOn;
    newState.ElementState = air.isEHeaterOn;
    newState.DomesticWaterPriority = air.isWaterOn;
    newState.errorCode = hcu.faultCode;
	newState.Solenoid = hcu.stateZone0;
	
    int16_t setpoint, temp;
    setpoint = hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)];
    if(display.setup.celsius)
        newState.currentSetpoint=setpoint;
    else
        newState.currentSetpoint=core.farToCel(setpoint);
    if (display.setup.celsius & 0x01)
    {
        newState.setpointNight = hcu.airHeaterTSetPoint[0];
        newState.setpointDay = hcu.airHeaterTSetPoint[1];
    }
    else
    {
        newState.setpointNight = core.farToCel(hcu.airHeaterTSetPoint[0]);
        newState.setpointDay = core.farToCel(hcu.airHeaterTSetPoint[1]);
    }
    if (air.isPanelSensor & 0x01) {
        temp = temperature.panel;
    }
    else {
        temp = air.temperatureActual;
    }

    newState.FanManual = hcu.fanManual;

    if (!(display.setup.celsius & 0x01)) {
        setpoint = core.farToCel(setpoint);
        temp = core.farToCel(temp);
    }
    if ((!air.isAirOn[(air.isDay|air.isSelectDay)&(!air.isSelectNight)]) || ((air.isEHeaterOn&0x01)==0 && (air.isFHeaterOn&0x01)==0)) {
        newState.FanSpeed = 0;
    }
    else if (hcu.fanManual)
        newState.FanSpeed = hcu.fanPower;
    else
    {
        if (temp >= setpoint+1) newState.FanSpeed = 0;
        else if (temp >= setpoint-1) newState.FanSpeed = 35;
        else if (temp >= setpoint-2) newState.FanSpeed = 50;
        else if (temp >= setpoint-3) newState.FanSpeed = 70;
        else if (temp >= setpoint-4) newState.FanSpeed = 85;
        else newState.FanSpeed = 100;
    }


    //Send updated info to CAN
    if (oldState.PumpState!=newState.PumpState)
    {
        oldState.PumpState=newState.PumpState;
        canPGNRVC.msgCirculationPumpStatus();
    }

    if (oldState.HeaterState!=newState.HeaterState || oldState.ElementState!=newState.ElementState)
    {
        oldState.HeaterState=newState.HeaterState;
        oldState.ElementState=newState.ElementState;
        canPGNRVC.msgWaterHeater();
    }

    if (oldState.DomesticWaterPriority!=newState.DomesticWaterPriority)
    {
        oldState.DomesticWaterPriority = newState.DomesticWaterPriority;
        canPGNRVC.msgWaterHeater2();
    }

    if (oldState.errorCode!=newState.errorCode)
    {
        oldState.errorCode = newState.errorCode;
        canPGNRVC.msgDiagnosticMessage();
    }

    if (oldState.FanManual!=newState.FanManual || oldState.FanSpeed!=newState.FanSpeed)
    {
        oldState.FanManual = newState.FanManual;
        oldState.FanSpeed = newState.FanSpeed;
        canPGNRVC.msgFurnace();
    }

    if (oldState.currentSetpoint!=newState.currentSetpoint)
    {
        oldState.currentSetpoint = newState.currentSetpoint;
        canPGNRVC.msgThermostat1();
        canPGNRVC.msgThermostat2();
    }

    if (oldState.setpointDay!=newState.setpointDay)
    {
        oldState.setpointDay = newState.setpointDay;
        canPGNRVC.msgThermostatSchedule1(1);
    }

    if (oldState.setpointNight!=newState.setpointNight)
    {
        oldState.setpointNight = newState.setpointNight;
        canPGNRVC.msgThermostatSchedule1(0);
    }
	
	if (oldState.Solenoid!=newState.Solenoid)
	{
		oldState.Solenoid = newState.Solenoid;
		
		canPGNRVC.msgExtMessage();
		
	}



}
//-----------------------------------------------------
void RVCModule::TransmitMessage(void)
{
    static uint32_t lastMsgSendTick1 = core.getTick();
	static uint32_t lastMsgSendTick2 = core.getTick()+200; //Shift to spread messages
    static uint8_t msgNum1 = 0;
	static uint8_t msgNum2 = 0;

    if ((core.getTick()-lastMsgSendTick1) >= 416) {                             // 5000ms/msg count = 5000 / 12 = 416
        lastMsgSendTick1 = core.getTick();
        switch(msgNum1)
        {
        case 1:
            canPGNRVC.msgFurnace();
            break;
        case 2:
            canPGNRVC.msgWaterHeater();
            break;
        case 3:
            canPGNRVC.msgWaterHeater2();
            break;
        case 4:
            canPGNRVC.msgThermostat1();
            break;
        case 5:
            canPGNRVC.msgThermostat2();
            break;
        case 6:
            canPGNRVC.msgThermostatSchedule1(0);
            break;
        case 7:
            canPGNRVC.msgThermostatSchedule1(1);
            break;
        case 8:
            canPGNRVC.msgCirculationPumpStatus();
            break;
        case 9:
            canPGNRVC.msgAmbientTemp();
            break;
		case 10: canPGNRVC.msgExtMessage();
			break;
		case 11: canPGNRVC.msgTimers();
			break;
        default:
            canPGNRVC.msgDiagnosticMessage();
            msgNum1=0;
        }
        msgNum1++;
    }
	
	if ((core.getTick()-lastMsgSendTick2)>=60000) {                             // 60000ms/msg count = 5000 / 12 = 416
        lastMsgSendTick2 = core.getTick();
        switch(msgNum2)
        {
        case 1:
            break;
        
        default:
			canPGNRVC.msgHeaterInfo();
            msgNum2=0;
        }
        msgNum2++;
    }
}
//-----------------------------------------------------
//обработка пакета команды
void RVCModule::ProcessMessage(uint8_t MsgNum)
{
    int8_t instance, B;
    int16_t Ts;
    uint32_t DGN = (can.RxMsgBuf[MsgNum].RxMsg.rx_efid >> 8) & 0x1FFFF;
    uint8_t mesLen= can.RxMsgBuf[MsgNum].RxMsg.rx_dlen;
    if (mesLen!=8) return;
    hcu.lockTimer = core.getTick();
    switch(DGN) {
    case 0x1FFF6: //Water Heater Command         //automatic and test modes are not supported
        instance = can.RxMsgBuf[MsgNum].RxMsg.rx_data[0];
        if (instance == 1) {
            switch(can.RxMsgBuf[MsgNum].RxMsg.rx_data[1]) {
            case 0: //Выключить все
                air.isFHeaterOn = false;
                air.isEHeaterOn = false;
                break;
            case 1: //Включить подогреваель
                air.isFHeaterOn = true;
                air.isEHeaterOn = false;
                hcu.timerOffSystem = core.getTick();
                break;
            case 2: //Включить ТЭН
                air.isFHeaterOn = false;
                air.isEHeaterOn = true;
                hcu.timerOffSystem = core.getTick();
                break;
            case 3: //Включить подогреваель и ТЭН
                air.isFHeaterOn = true;
                air.isEHeaterOn = true;
                hcu.timerOffSystem = core.getTick();
                break;
            }
        }
        break;
    case 0x1FFE3: //Furnace Command
        instance = can.RxMsgBuf[MsgNum].RxMsg.rx_data[0];
        if (instance == 1) {
            B = can.RxMsgBuf[MsgNum].RxMsg.rx_data[1]&3;
            if (B < 2) {
                hcu.fanManual = B | 2;
                hcu.fanAuto = !B | 2;
                if (can.RxMsgBuf[MsgNum].RxMsg.rx_data[2] < 201) {
                    hcu.fanPower = can.RxMsgBuf[MsgNum].RxMsg.rx_data[2]/2;
                }
            }
        }
        break;
    case 0x1FEF9: //Thermostat Command
        instance = can.RxMsgBuf[MsgNum].RxMsg.rx_data[0];
        if (instance == 1) {
            B = can.RxMsgBuf[MsgNum].RxMsg.rx_data[1]&0xF;
            if (B==2 || B==0 || B==3) { // 0-off // 2-heat 3-auto
				
                if (B==0) {
                    air.isAirOn[0] = false;
                    air.isAirOn[1] = false;
                }
                else {
                    air.isAirOn[0] = true;
                    air.isAirOn[1] = true;
                }
			}
			if (((can.RxMsgBuf[MsgNum].RxMsg.rx_data[4]<<8)+can.RxMsgBuf[MsgNum].RxMsg.rx_data[3]) != 0xFFFF){
                //If it's day we change day time setpoint,otherwise - night time
				double precisionTemp = (can.RxMsgBuf[MsgNum].RxMsg.rx_data[4]<<8)+can.RxMsgBuf[MsgNum].RxMsg.rx_data[3];
                
                precisionTemp = (Ts/32.0-273.0);
                if (!(display.setup.celsius & 0x01)) {
                    precisionTemp = core.celToFar(precisionTemp);
                }
				Ts = (uint16_t)precisionTemp;
				
                if (!air.isDay) {
					hcu.airHeaterTSetPoint[0]=Ts;
                    if (air.isAirOn[0]) {
                        slider.setPosition(hcu.airHeaterTSetPoint[0]);
                    }
                }
                else {
					hcu.airHeaterTSetPoint[1]=Ts;
                    if (air.isAirOn[1]) {
                        slider.setPosition(hcu.airHeaterTSetPoint[1]);
                    }
                }
            }
        }
		break;


    case 0x1FEF5: //Thermostat schedule command 1 //Supports only day/nignt schedule instances
        instance = can.RxMsgBuf[MsgNum].RxMsg.rx_data[0];
        if (instance == 1) {
            Ts = (can.RxMsgBuf[MsgNum].RxMsg.rx_data[5] << 8) + can.RxMsgBuf[MsgNum].RxMsg.rx_data[4];
            Ts = (int16_t)(Ts / 32 - 273);
            if (!(display.setup.celsius & 0x01)) {
                Ts = core.celToFar(Ts);
            }
            if (can.RxMsgBuf[MsgNum].RxMsg.rx_data[1] == 0) {
                if ((can.RxMsgBuf[MsgNum].RxMsg.rx_data[5] << 8) + can.RxMsgBuf[MsgNum].RxMsg.rx_data[4] != 0xFFFF) {
                    hcu.airHeaterTSetPoint[0] = Ts;
                    if (air.isAirOn[0]) {
                        slider.setPosition(hcu.airHeaterTSetPoint[0]);
                    }
                }
                if (can.RxMsgBuf[MsgNum].RxMsg.rx_data[2] < 24) {
                    air.nightTimeH = can.RxMsgBuf[MsgNum].RxMsg.rx_data[2];
                }
                if (can.RxMsgBuf[MsgNum].RxMsg.rx_data[3] < 60) {
                    air.nightTimeM = can.RxMsgBuf[MsgNum].RxMsg.rx_data[3];
                }
            }
            else if (can.RxMsgBuf[MsgNum].RxMsg.rx_data[1] == 1) {
                if ((can.RxMsgBuf[MsgNum].RxMsg.rx_data[5] << 8) + can.RxMsgBuf[MsgNum].RxMsg.rx_data[4] != 0xFFFF) {
                    hcu.airHeaterTSetPoint[1] = Ts;
                    if (air.isAirOn[1]) {
                        slider.setPosition(hcu.airHeaterTSetPoint[1]);
                    }
                }
                if (can.RxMsgBuf[MsgNum].RxMsg.rx_data[2] < 24) {
                    air.dayTimeH = can.RxMsgBuf[MsgNum].RxMsg.rx_data[2];
                }
                if (can.RxMsgBuf[MsgNum].RxMsg.rx_data[3] < 60) {
                    air.dayTimeM = can.RxMsgBuf[MsgNum].RxMsg.rx_data[3];
                }
            }
            //slider.setPosition(hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)]);
        }
        break;
    case 0x1FE96:
        if (can.RxMsgBuf[MsgNum].RxMsg.rx_data[0]==1)
        {
            if (can.RxMsgBuf[MsgNum].RxMsg.rx_data[1]==0)
            {
                hcu.pumpOn = 2;
            }
            if (can.RxMsgBuf[MsgNum].RxMsg.rx_data[1]==1)
            {
                hcu.pumpOn = 3;
                hcu.timerPumpOn = core.getTick();
            }
        }

        break;

    case 0x1EF65:
        switch(can.RxMsgBuf[MsgNum].RxMsg.rx_data[0])
        {
        case 0x81: 
            hcu.faultCodePanel = 0;
            if (hcu.faultCode!=0 && hcu.isError==0) {
                hcu.stageOld = 0;
                hcu.isError = 1;
            }
            else {
                hcu.isError = 0;
            }
            break;
        case 0x83: 
            if((can.RxMsgBuf[MsgNum].RxMsg.rx_data[1] & 3)!=3)
            {
                if ((can.RxMsgBuf[MsgNum].RxMsg.rx_data[1] & 3) == 0) 
                    air.isWaterOn = false;
                if ((can.RxMsgBuf[MsgNum].RxMsg.rx_data[1] & 3) == 1) 
                {
                    air.isWaterOn = true;
                    hcu.timerOffDomesticWater = core.getTick();
                }
            }
            break;
        }
    }
}
