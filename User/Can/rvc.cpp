/******************************************************************************
* ��� ������
* ������
*
* ������������: .
*
* 17.07.2020
* ��������:
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

#define BKP_VALUE    0x32F0 

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

    if (core.getTick()-lastExtTempGetTick > 60000)
        externalTemperatureProvided = false;


}
//-----------------------------------------------------
void RVCModule::TransmitMessage(void)
{
    static uint32_t lastMsgSendTick1 = core.getTick();
    static uint32_t lastMsgSendTick2 = core.getTick()+200; //Shift to spread messages
    static uint32_t lastMsgSendTick3 = core.getTick()+300; //Shift to spread messages
    static uint8_t msgNum1 = 1;
    static uint8_t msgNum2 = 1;
    static uint8_t msgNum3 = 1;

    // every second messages
    if ((core.getTick()-lastMsgSendTick3) >= 1000) {                             // 1000ms/msg count = 1000 / 1 = 1000
        lastMsgSendTick3 = core.getTick();
        switch(msgNum3)
        {
        case 1:
            canPGNRVC.msgTimers();
            break;
        default:
            msgNum3=0;
            break;
        }
        msgNum3++;
    }
    //5 second messages
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
        case 10:
            canPGNRVC.msgExtMessage();
            break;
		case 11:
            canPGNRVC.msgTimersSetupStatus();
            break;

        default:
            canPGNRVC.msgDiagnosticMessage();
            msgNum1=0;
        }
        msgNum1++;
    }

    //60 second messages
    if ((core.getTick()-lastMsgSendTick2)>=20000) {                             // 60000ms/msg count = 60000 / 3 = 20000
        lastMsgSendTick2 = core.getTick();
        switch(msgNum2)
        {
        case 1:
            canPGNRVC.msgHeaterInfo();
            break;
        case 2:
            canPGNRVC.msgPanelInfo();
            break;

        default:
            canPGNRVC.msgHcuInfo();
            msgNum2=0;
        }
        msgNum2++;
    }
}
//-----------------------------------------------------
//��������� ������ �������
void RVCModule::ProcessMessage(uint8_t MsgNum)
{
    int8_t instance, B;
    int16_t Ts;
    uint32_t DGN = (can.RxMsgBuf[MsgNum].RxMsg.rx_efid >> 8) & 0x1FFFF;
    uint8_t mesLen= can.RxMsgBuf[MsgNum].RxMsg.rx_dlen;
    bool redrawSlider=false;
    if (mesLen!=8) return;
    hcu.lockTimer = core.getTick();
    uint8_t* D = can.RxMsgBuf[MsgNum].RxMsg.rx_data;
    switch(DGN) {
    case 0x1FFFF: //Date time
        unixTime.year = D[0]+2000;
        if (D[1]>0&&D[1]<13) unixTime.mon = D[1];
        if (D[2]>0&&D[2]<32) unixTime.mday = D[2];
        if (D[3]>0&&D[3]<8) unixTime.wday = D[3];
        if (D[4]<24) unixTime.hour = D[4];
        if (D[5]<60) unixTime.min = D[5];
        if (D[6]<60) unixTime.sec = D[6];
		    // allow access to BKP domain
    rcu_periph_clock_enable(RCU_PMU);
    pmu_backup_write_enable();
    // wait for RTC registers synchronization
    rtc_register_sync_wait();
    rtc_lwoff_wait();
    // wait until last write operation on RTC registers has finished
    rtc_lwoff_wait();
    
    uint32_t timer = unixTime.calToTimer();
    rtc_counter_set(timer);
    bkp_write_data(BKP_DATA_0, BKP_VALUE);
        break;

    case 0x1FF9C: //Date time
        if (D[0]!=1) return;
        externalTemperatureProvided = true;
        externalTemperature = (D[1]+D[2]*256)/32.0-273.0;
        lastExtTempGetTick = core.getTick();
        break;

    case 0x1FFF6: //Water Heater Command         //automatic and test modes are not supported
        instance = D[0];
        if (instance == 1) {
            switch(D[1]) {
            case 0: //��������� ���
                air.isFHeaterOn = false;
                air.isEHeaterOn = false;
                break;
            case 1: //�������� ������������
                air.isFHeaterOn = true;
                air.isEHeaterOn = false;
                hcu.timerOffSystem = core.getTick();
                break;
            case 2: //�������� ���
                air.isFHeaterOn = false;
                air.isEHeaterOn = true;
                hcu.timerOffSystem = core.getTick();
                break;
            case 3: //�������� ������������ � ���
                air.isFHeaterOn = true;
                air.isEHeaterOn = true;
                hcu.timerOffSystem = core.getTick();
                break;
            }
        }
        break;
    case 0x1FFE3: //Furnace Command
        instance = D[0];
        if (instance == 1) {
            B = D[1]&3;
            if (B < 2) {
                hcu.fanManual = B | 2;
                hcu.fanAuto = !B | 2;
                if (D[2] < 201) {
                    hcu.fanPower = D[2]/2;
                }
            }
        }
        break;
    case 0x1FEF9: //Thermostat Command
        instance = D[0];
        if (instance == 1) {
            B = D[1]&0xF;
            if (B==2 || B==0 || B==3) { // 0-off // 2-heat 3-auto

                if (B==0) {
                    air.isAirOn[0] = false;
                    air.isAirOn[1] = false;
                }
                else {
                    air.isAirOn[0] = true;
                    air.isAirOn[1] = true;
                }
                redrawSlider=true;
            }
            uint16_t temp = (D[4]<<8)+D[3];
            if( temp != 0xFFFF) {
                //If it's day we change day time setpoint,otherwise - night time

                double precisionTemp = temp;

                precisionTemp = (precisionTemp/32.0-273.0);
                if (!(display.setup.celsius & 0x01)) {
                    precisionTemp = core.celToFar(precisionTemp)+0.5f;
                }
                Ts = (uint16_t)precisionTemp;

                if (!air.isDay) {
                    hcu.airHeaterTSetPoint[0]=Ts;
                    if (air.isAirOn[0]) {
                        redrawSlider=true;
                    }
                }
                else {
                    hcu.airHeaterTSetPoint[1]=Ts;
                    if (air.isAirOn[1]) {
                        redrawSlider=true;
                    }
                }
            }
        }
        break;


    case 0x1FEF5: //Thermostat schedule command 1 //Supports only day/nignt schedule instances
        instance = D[0];
        if (instance == 1) {
            float preciseTemp = (D[5] << 8) + D[4];
            preciseTemp = (preciseTemp / 32 - 273);
            if (!(display.setup.celsius & 0x01)) {
                preciseTemp = core.celToFar(preciseTemp)+0.5f;
            }
            Ts = (int16_t)preciseTemp;
            if (D[1] == 0) {
                if ((D[5] << 8) + D[4] != 0xFFFF) {
                    hcu.airHeaterTSetPoint[0] = Ts;
                    if (air.isAirOn[0]) {
                        redrawSlider=true;
                    }
                }
                if (D[2] < 24) {
                    air.nightTimeH = D[2];
                }
                if (D[3] < 60) {
                    air.nightTimeM = D[3];
                }
            }
            else if (D[1] == 1) {
                if ((D[5] << 8) + D[4] != 0xFFFF) {
                    hcu.airHeaterTSetPoint[1] = Ts;
                    if (air.isAirOn[1]) {
                        redrawSlider=true;
                    }
                }
                if (D[2] < 24) {
                    air.dayTimeH = D[2];
                }
                if (D[3] < 60) {
                    air.dayTimeM = D[3];
                }
            }

        }
        break;
    case 0x1FE96:
        if (D[0]==1)
        {
            if (D[1]==0)
            {
                hcu.pumpOn = 2;
            }
            if (D[1]==1)
            {
                hcu.pumpOn = 3;
                hcu.timerPumpOn = core.getTick();
            }
        }

        break;

    case 0x1EF65:
        switch(D[0])
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
            hcu.clearErrorRequest=true;
            break;

        case 0x83:
            if((D[1] & 3)!=3)
            {
                if ((D[1] & 3) == 0)
                    air.isWaterOn = false;
                if ((D[1] & 3) == 1)
                {
                    air.isWaterOn = true;
                    hcu.timerOffDomesticWater = core.getTick();
                }
            }
			canPGNRVC.msgWaterHeater2();
            break;
        case 0x89:
            if ((D[1]+D[2]*256)!=0xFFFF)
            {
                uint16_t systemLimitation = D[1]+D[2]*256;
                if (systemLimitation<60) systemLimitation=60;
                if (systemLimitation>7200) systemLimitation=7200;
                hcu.durationSystem = systemLimitation;
            }

            if ((D[3])!=0xFF)
            {
                uint16_t waterLimitation = D[3];
                if (waterLimitation<30) waterLimitation=30;
                if (waterLimitation>60) waterLimitation=60;
                hcu.durationDomesticWater = waterLimitation;
            }
			canPGNRVC.msgTimersSetupStatus();
            break;
        }

    }
    if (redrawSlider)
        slider.setPosition(hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)]);
}
