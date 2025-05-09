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
#include "main.h"
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
#include "canvas.h"

#define BKP_VALUE    0x32F0

extern bool saveSetupFlag;

RVCModule rvc;
//-----------------------------------------------------
RVCModule::RVCModule(void)
{
    core.ClassInit(this,sizeof(this));
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
    newState.PumpState = 0x00;
    if (hcu.statePump)
        newState.PumpState = 0x01;
    if (hcu.pumpOn&1)
        newState.PumpState = 0x05;


    newState.HeaterState = air.isFHeaterOn;
    newState.ElementState = air.isEHeaterOn;
    newState.DomesticWaterPriority = air.isWaterOn;
    newState.errorCode = hcu.faultCode;
    newState.Solenoid = hcu.stateZone0;
    newState.DayStartHour = air.dayTimeH;
    newState.DayStartMinute = air.dayTimeM;
    newState.NightStartHour = air.nightTimeH;
    newState.NightStartMinute = air.nightTimeM;
    newState.WaterDuration = hcu.durationDomesticWater;
    newState.SystemDuration = hcu.durationSystem;
    newState.ZoneEnabled = air.isAirOn;
    newState.UsePanelSensor = air.isPanelSensor;
    newState.scheduleMode = display.setup.scheduleMode&1;


    int16_t setpoint, temp;
    setpoint = hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)];
    if(display.setup.celsius)
        newState.currentSetpoint=setpoint;
    else
        newState.currentSetpoint=core.farToCel(setpoint);

    if (newState.SystemDuration!=oldState.SystemDuration || newState.WaterDuration!=oldState.WaterDuration)
    {
        oldState.WaterDuration = newState.WaterDuration;
        oldState.SystemDuration = newState.SystemDuration;

        canPGNRVC.msgTimersSetupStatus();
    }

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

    newState.FanManualMode = hcu.fanManual&1;
    newState.FanManualSpeed = hcu.fanPower;

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

    if (oldState.FanManualMode!=newState.FanManualMode || oldState.FanCurrentSpeed!=newState.FanCurrentSpeed)
    {
        oldState.FanManualMode = newState.FanManualMode;
        oldState.FanCurrentSpeed = newState.FanCurrentSpeed;
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

    if (oldState.Solenoid!=newState.Solenoid
            || oldState.FanManualSpeed!=newState.FanManualSpeed
            || oldState.UsePanelSensor!=newState.UsePanelSensor) //0x84
    {
        oldState.Solenoid = newState.Solenoid;
        oldState.FanManualSpeed = newState.FanManualSpeed;
        oldState.UsePanelSensor = newState.UsePanelSensor;

        canPGNRVC.msgExtMessage();
    }



    if (newState.DayStartHour!=oldState.DayStartHour||newState.DayStartMinute!=oldState.DayStartMinute)
    {
        oldState.DayStartHour=newState.DayStartHour;
        oldState.DayStartMinute=newState.DayStartMinute;

        canPGNRVC.msgThermostatSchedule1(1);
    }

    if (newState.NightStartHour!=oldState.NightStartHour||newState.NightStartMinute!=oldState.NightStartMinute)
    {
        oldState.NightStartHour=newState.NightStartHour;
        oldState.NightStartMinute=newState.NightStartMinute;

        canPGNRVC.msgThermostatSchedule1(0);
    }

    if (newState.ZoneEnabled !=oldState.ZoneEnabled ||
            newState.scheduleMode != oldState.scheduleMode)
    {
        oldState.ZoneEnabled = newState.ZoneEnabled;
        oldState.scheduleMode = newState.scheduleMode;

        canPGNRVC.msgThermostat1();
    }

    //Disabling temperature override after 1 minute
    if (core.getTick()-lastExtTempGetTick > 60000)
    {
        externalTemperatureProvided = false;
        externalTemperatureProvidedChanged = true;
    }

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
    if ((core.getTick()-lastMsgSendTick3) >= 500) {                             // 1000ms/msg count = 1000 / 2 = 1000
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
            if (display.setup.scheduleMode)
                canPGNRVC.msgThermostatSchedule1(0);
            break;
        case 7:
            if (display.setup.scheduleMode)
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
    uint32_t timer;
    uint16_t temp;
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

        timer = unixTime.calToTimer();
        rtc_counter_set(timer);
        bkp_write_data(BKP_DATA_0, BKP_VALUE);
        break;

    case 0x1FF9C: //Ambient temperature
        if (D[0]!=1) return;
        if (!externalTemperatureProvided)
            externalTemperatureProvidedChanged = true;
        externalTemperatureProvided = true;

        externalTemperature = (D[1]+D[2]*256)/32.0-273.0;
        lastExtTempGetTick = core.getTick();
        break;

    case 0x1FFF6: //Water Heater Command         //automatic and test modes are not supported
        instance = D[0];
        if (instance == 1) {
            switch(D[1]&0xF) {
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
            }
            if (D[2] < 201) {
                hcu.fanPower = D[2]/2;
            }
        }
        break;
    case 0x1FEF9: //Thermostat Command
        temp = (D[4]<<8)+D[3];
        if (D[0] != 1) return;
        if ((D[1]&0xF)==0)
            air.isAirOn = false;
        if ((D[1]&0xF)==2 || (D[1]&0xF)==3)
            air.isAirOn = true;
        redrawSlider=true;


        if( temp != 0xFFFF) {

            double precisionTemp = temp;

            precisionTemp = (precisionTemp/32.0-273.0);
            if (!(display.setup.celsius & 0x01)) {
                precisionTemp = core.celToFar(precisionTemp)+0.5f;
            }
            Ts = (uint16_t)precisionTemp;

            if (!air.isDay || !display.setup.scheduleMode) {
                hcu.airHeaterTSetPoint[0]=Ts;
                if (air.isAirOn) {
                    redrawSlider=true;
                }
            }
            if (air.isDay || !display.setup.scheduleMode)
				{
                hcu.airHeaterTSetPoint[1]=Ts;
                if (air.isAirOn) {
                    redrawSlider=true;
                }
            }
        }
        if (((D[1]>>6)&3)<2 && ((display.setup.scheduleMode&1)!=((D[1]>>6)&1)))
        {
            display.setup.scheduleMode = ((D[1]>>6)&3)|2;
            saveSetupFlag=1;
			if (screen_visible != SCREEN_VISIBLE_AIR) break;
			air.ScheduleModeView();
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
                    if (air.isAirOn) {
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
                    if (air.isAirOn) {
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
    case 0x1FE96: //Circulation pums command
        if (D[0]==1)
        {
            if ((D[1]&0xF)==0)
            {
                hcu.pumpOn = 2;
            }
            if ((D[1]&0xF)==5)
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
            if((D[1] & 3) < 2)
            {
                air.isWaterOn = D[1] & 3;
                if (air.isWaterOn) hcu.timerOffDomesticWater = core.getTick();
            }

            if (((D[1]>>2) & 3) < 2)
                air.isPanelSensor = ((D[1]>>2) & 3);
            break;
        case 0x89:
            if ((D[1]+D[2]*256)!=0xFFFF)
            {
                uint16_t systemDuration = D[1]+D[2]*256;
                if (systemDuration<60) systemDuration=60;
                if (systemDuration>7200) systemDuration=7200;
                hcu.durationSystem = systemDuration;
            }

            if ((D[3])!=0xFF)
            {
                uint16_t waterDuration = D[3];
                if (waterDuration<30) waterDuration=30;
                if (waterDuration>60) waterDuration=60;
                hcu.durationDomesticWater = waterDuration;
            }
            canPGNRVC.msgTimersSetupStatus();
            break;
        }
    }
    if (redrawSlider)
        slider.setPosition((uint16_t)hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)]);

}
