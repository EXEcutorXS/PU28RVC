/******************************************************************************
  ООО Адверс
  Самара

  Программисты: .

  17.07.2020
  Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "rvc.h"
#include "core.h"
#include "canext.h"
#include "pgn_rvc.h"
#include "monitor.h"
#include "adc.h"

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
    if (canExt.RxMsgBuf[i].Received_f) {
      this->ProcessMessage(i);
      canExt.RxMsgBuf[i].Received_f = false;
    }
  }
}
//-----------------------------------------------------
void RVCModule::TransmitMessage(void)
{
  static uint32_t timer1s = core.getTick();
  static uint32_t timer2s = core.getTick() + 30;
  static uint32_t timer3s = core.getTick() + 60;
  static uint32_t timer4s = core.getTick() + 90;
  static uint8_t MsgNum1s = 0;
  static uint8_t MsgNum2s = 0;
  static uint8_t MsgNum3s = 0;
  static uint8_t MsgNum4s = 0;

  if ((core.getTick() - timer1s) >= 166) {                         //периодичность отправки = 1000мс/кол-во сообщений   =   1000 / 6 = 166
    timer1s = core.getTick();

    MsgNum1s++;
    switch (MsgNum1s) {
      case 1: canPGNRVC.msgFurnace(0);
        break;
      case 2: canPGNRVC.msgFurnace(1);
        break;
      case 3: canPGNRVC.msgFurnace(2);
        break;
      case 4: canPGNRVC.msgFurnace(3);
        break;
      case 5: canPGNRVC.msgFurnace(4);
        break;
      default: canPGNRVC.msgDiagnosticMessage();
        MsgNum1s = 0;
    }
  }

  if ((core.getTick() - timer2s) >= 166) {                         //периодичность отправки = 2000мс/кол-во сообщений   =   2000 / 12 = 166
    timer2s = core.getTick();
    MsgNum2s++;
    switch (MsgNum2s) {
      case 1: canPGNRVC.msgWaterHeater();
        break;
      case 2: canPGNRVC.msgThermostat1(0);
        break;
      case 3: canPGNRVC.msgThermostat1(1);
        break;
      case 4: canPGNRVC.msgThermostat1(2);
        break;
      case 5: canPGNRVC.msgThermostat1(3);
        break;
      case 6: canPGNRVC.msgThermostat1(4);
        break;
      case 7: canPGNRVC.msgThermostat2(0);
        break;
      case 8: canPGNRVC.msgThermostat2(1);
        break;
      case 9: canPGNRVC.msgThermostat2(2);
        break;
      case 10: canPGNRVC.msgThermostat2(3);
        break;
      case 11: canPGNRVC.msgThermostat2(4);
        break;

      default: canPGNRVC.msgCirculationPumpStatus();
        MsgNum2s = 0;
    }

  }


  if ((core.getTick() - timer3s) >= 1000) {
    timer3s = core.getTick();
    MsgNum3s++;
    if (MsgNum3s >= 14) MsgNum3s = 0;
    canPGNRVC.msgThermostatSchedule1(MsgNum3s / 3, MsgNum3s % 3);
  }



  if ((core.getTick() - timer4s) >= 1000) {
    timer4s = core.getTick();

    canPGNRVC.msgAmbientTemp(MsgNum4s);
    MsgNum4s++;
    if (MsgNum4s > 4)
      MsgNum4s = 0;
  }
}
//-----------------------------------------------------
//обработка пакета команды
void RVCModule::ProcessMessage(uint8_t MsgNum)
{
  uint8_t ZN, B;
  int16_t Ts;
  uint32_t DGN = (canExt.RxMsgBuf[MsgNum].RxMsg.ExtId >> 8) & 0x1FFFF;
  switch (DGN) {
    case 0x1FFF6: //Water Heater Command
      if (canExt.RxMsgBuf[MsgNum].RxMsg.Data[0] == 1) {
        switch (canExt.RxMsgBuf[MsgNum].RxMsg.Data[1]) {
          case 0: //Выключить все
            Monitor.btnHeater_f = false;
            Monitor.btnElement_f = false;
            Monitor.SetButtonStatus(BTN_HEATER, HEATER_OFF);
            Monitor.SetButtonStatus(BTN_ELEMENT, ELEMENT_OFF);
            break;
          case 1: //Включить подогреваель
            Monitor.ClearErrorsDisplay();
            Monitor.btnHeater_f = true;
            Monitor.SetButtonStatus(BTN_HEATER, HEATER_ON);
            break;
          case 2: //Включить ТЭН
            Monitor.ClearErrorsDisplay();
            Monitor.btnElement_f = true;
            Monitor.SetButtonStatus(BTN_ELEMENT, ELEMENT_ON);
            break;
          case 3: //Включить подогреваель и ТЭН
            Monitor.ClearErrorsDisplay();
            Monitor.btnHeater_f = true;
            Monitor.btnElement_f = true;
            Monitor.SetButtonStatus(BTN_HEATER, HEATER_ON);
            Monitor.SetButtonStatus(BTN_ELEMENT, ELEMENT_ON);
            break;
        }
      }
      break;
    case 0x1FFE3: //Furnace Command
      ZN = canExt.RxMsgBuf[MsgNum].RxMsg.Data[0] - 1;
      if (ZN < ZONE_COUNT) {
        if (Zone.Connected_f[ZN]) {
          B = canExt.RxMsgBuf[MsgNum].RxMsg.Data[1] & 3;
          if (B < 2) {
            Zone.FanManual_f[ZN] = B;
            if (Zone.FanManual_f[ZN]) {
              Zone.ManualPersent[ZN] = canExt.RxMsgBuf[MsgNum].RxMsg.Data[2] / 2;
            }
          }
          Monitor.ChangeFanAuto(ZN); //
        }
      }
      break;
    case 0x1FEF9: //Thermostat Command
      ZN = canExt.RxMsgBuf[MsgNum].RxMsg.Data[0] - 1;
      if (ZN < ZONE_COUNT) {
        B = canExt.RxMsgBuf[MsgNum].RxMsg.Data[1] & 3;
        if (B == 2 || B == 0) { // 0-off // 2-heat
          if (B != 0) B = 1;
          Zone.State[ZN] = B;
          Monitor.updateButZoneOnOff(ZN); //
          Ts = (canExt.RxMsgBuf[MsgNum].RxMsg.Data[4] << 8) + canExt.RxMsgBuf[MsgNum].RxMsg.Data[3];
          Ts = (int16_t)(Ts / 32 - 273);
          if ((canExt.RxMsgBuf[MsgNum].RxMsg.Data[4] << 8) + canExt.RxMsgBuf[MsgNum].RxMsg.Data[3] != 0xFFFF)
          {
            if (Monitor.IsNowADay())
              Zone.TsetpointDay[ZN] = Ts;
            else
              Zone.TsetpointNight[ZN] = Ts;
          }
          if (ZN == Zone.Selected) Monitor.SetTempSlyders();

        }
      }
    case 0x1FF9C: //Ambient Temperature  (6<<26)+($1ff<<16)+($9c<<8)+96
      ZN = canExt.RxMsgBuf[MsgNum].RxMsg.Data[0] - 1;
      if (ZN < ZONE_COUNT) {
        Ts = (canExt.RxMsgBuf[MsgNum].RxMsg.Data[2] << 8) + canExt.RxMsgBuf[MsgNum].RxMsg.Data[1];
        Ts = (int16_t)(Ts * 0.03125 - 273);              //Температура в зоне
        adc.MeasureTzone_f = false;
      }
      break;
    case 0x1FEF5: //Thermostat schedule command 1 //Supports only day/nignt schedule instances
      ZN = canExt.RxMsgBuf[MsgNum].RxMsg.Data[0] - 1;
      if (ZN < ZONE_COUNT) {
        Ts = (canExt.RxMsgBuf[MsgNum].RxMsg.Data[5] << 8) + canExt.RxMsgBuf[MsgNum].RxMsg.Data[4];
        Ts = (int16_t)(Ts / 32 - 273);
        if (canExt.RxMsgBuf[MsgNum].RxMsg.Data[1] == 0)
        {
          if ((canExt.RxMsgBuf[MsgNum].RxMsg.Data[5] << 8) + canExt.RxMsgBuf[MsgNum].RxMsg.Data[4] != 0xFFFF)
            Zone.TsetpointNight[ZN] = Ts;
          if (canExt.RxMsgBuf[MsgNum].RxMsg.Data[2] < 24)
            Monitor.rtc.Night.Hours = canExt.RxMsgBuf[MsgNum].RxMsg.Data[2];
          if (canExt.RxMsgBuf[MsgNum].RxMsg.Data[3] < 60)
            Monitor.rtc.Night.Min = canExt.RxMsgBuf[MsgNum].RxMsg.Data[3];
        }
        if (canExt.RxMsgBuf[MsgNum].RxMsg.Data[1] == 1)
        {
          if ((canExt.RxMsgBuf[MsgNum].RxMsg.Data[5] << 8) + canExt.RxMsgBuf[MsgNum].RxMsg.Data[4] != 0xFFFF)
            Zone.TsetpointDay[ZN] = Ts;
          if (canExt.RxMsgBuf[MsgNum].RxMsg.Data[2] < 24)
            Monitor.rtc.Morning.Hours = canExt.RxMsgBuf[MsgNum].RxMsg.Data[2];
          if (canExt.RxMsgBuf[MsgNum].RxMsg.Data[3] < 60)
            Monitor.rtc.Morning.Min = canExt.RxMsgBuf[MsgNum].RxMsg.Data[3];
        }
      }


  }//switch()
}
