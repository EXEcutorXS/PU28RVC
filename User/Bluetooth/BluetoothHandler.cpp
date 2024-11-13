/******************************************************************************
* ООО ДД-информ
* Самара
* 
* Программисты: 
* 
* 7.10.2024
* Описание: Обработка пакетов данных Bluetooth
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "BluetoothHandler.h"
#include "bluetooth.h"
#include "core.h"
#include "hcu.h"
#include "system.h"
#include "clock.h"
#include "display.h"
#include "Error.h"
#include "pgn_rvc.h"
#include "temperature.h"
#include "setup.h"

#define		DAYTIME_MAX			1260
#define		NIGHTTIME_MAX		1430
#define		DAYTIME_DUR_MIN		120

BltHandler_C blt;
//-----------------------------------------------------
BltHandler_C::BltHandler_C(void)
{
    core.ClassInit(this,sizeof(this));
}
//-----------------------------------------------------
void BltHandler_C::initialize(void)
{
	if (!isBleAccept) {
		setNewId();
		setNewKey();
	}
	
	isBleCancel = false;
	isBleSendKey = false;
	isBleAcceptPage = false;
	BLE_PAUSE_PERIOD = 0;
	packNum = 0;
	bluetooth.initialize((char*)"Timberline 1.5");
//	DaySetpoint = 0;
//	NightSetpoint = 0;
	isAirOn = air.isAirOn;
}
//-----------------------------------------------------
void BltHandler_C::handler(void)
{
    uint8_t group;
    uint8_t i, x;
    static uint32_t lastSendTick=0;
    static uint32_t timer1000=0;
	
    if ((core.getTick()-timer1000) >= 1000)
    {
        timer1000 = core.getTick();
        counterBle = counterBleTemp;
        counterBleTemp = 0;
        counterBleSend = counterBleSendTemp;
        counterBleSendTemp = 0;
		receivedMessages= receivedMsgCounter;
		receivedMsgCounter=0;
		sentMessages = sentMsgCounter;
		sentMsgCounter=0;
    }

    counterBleTemp++;

    bluetooth.handler();                                                    // связь через Bluetooth

    if (!bluetooth.isConnected)
    {
        isBleAccept = false;
        isBleCancel = false;
        isBleSendKey = false;
        BLE_SEND_SHORT_PERIOD = 1;
        BLE_SEND_LONG_PERIOD = 1;
        isBleAcceptPage = false;
		Group1PacketCount = 0;
		Group2PacketCount = 0;
		Group3PacketCount = 0;
		Group7PacketCount = 0;
        return;
    }

    if (bluetooth.isCommand)                                            // прием команд
    {
		receivedMsgCounter++;
        bluetooth.isCommand = FALSE;
        group = bluetooth.bufRx[0];
        for (i=0; i<20; i++)
        {
            buf[i] = bluetooth.bufRx[i];
        }

        if (isBleAccept == true || group == bluetooth.TYPE_TIME)
        {
			updateGroup = group;
            if (group == 1) { // «Текущие параметры»   
				Group1PacketCount++;
//				bool cels = buf[19]&0x01;     // фаренгейт/цельсий	
//				if (cels == display.setup.celsius) 
//				{
//					if (buf[1]>=7 && buf[1]<=32) {
//						DaySetpoint = buf[1];
//					}
//					if (buf[6]>=7 && buf[6]<=32) {
//						NightSetpoint = buf[6];
//					}
//				} else {				
//					if (display.setup.celsius)
//					{
//						if (buf[1]>=45 && buf[1]<=90) {
//							DaySetpoint = core.farToCel(buf[1]);
//						}
//						if (buf[6]>=45 && buf[6]<=90) {
//							NightSetpoint = core.farToCel(buf[6]);
//						}
//					} else {
//						if (buf[1]>=7 && buf[1]<=32) {
//							DaySetpoint = core.celToFar(buf[1]);
//						}
//						if (buf[6]>=7 && buf[6]<=32) {
//							NightSetpoint = core.celToFar(buf[6]);
//						}
//					}
//				}
				if (display.setup.celsius)
				{
					if (buf[1]>=7 && buf[1]<=32) {
						DaySetpoint = buf[1];
					}
					if (buf[6]>=7 && buf[6]<=32) {
						NightSetpoint = buf[6];
					}
				} else {
					if (buf[1]>=7 && buf[1]<=32) {
						DaySetpoint = core.celToFar(buf[1]);
					}
					if (buf[6]>=7 && buf[6]<=32) {
						NightSetpoint = core.celToFar(buf[6]);
					}
				}

//				if ((display.setup.scheduleMode&1) == 0) {
//					if (DaySetFromPhone != buf[1]) {
//						DaySetFromPhone = buf[1];
//						NightSetFromPhone = DaySetFromPhone;
//						NightSetpoint = DaySetpoint;
//					} else 						
//					if (NightSetFromPhone != buf[6]) {
//						NightSetFromPhone = buf[6];
//						DaySetFromPhone = NightSetFromPhone;
//						DaySetpoint = NightSetpoint;
//					}						
//				} else {
//					DaySetFromPhone = buf[1];
//					NightSetFromPhone = buf[6];
//				}
				air.isFHeaterOn = (buf[17]&(1<<0))!=0;
				air.isEHeaterOn = (buf[17]&(1<<1))!=0;

				air.isWaterOn = (buf[17]&(1<<2))!=0;          // кнопки по битам, разбор воды
				if (air.isWaterOn) hcu.timerOffDomesticWater = core.getTick();

                if (buf[17]&(1<<4)) {
                    error.ClearErrors();
				}
				hcu.needUpdate_f = true;
				hcu.UpdateSetpoints(isAirOn, DaySetpoint, NightSetpoint);
            } else
            if (group == 2) { // «Текущие параметры»    //air.isPanelSensor |= 2;
				Group2PacketCount++;
				hcu.fanManual = (buf[1]>>7)&1;
				hcu.fanAuto = !hcu.fanManual;
				hcu.fanPower = buf[1]&0x7F;
//                hcu.stateZone0 = buf[15]&1;
				isAirOn = buf[15]&1;
				if (((buf[17]>>3)&1) != 0) {
					display.setup.scheduleMode = true;
				} else {
					display.setup.scheduleMode = false;	
					air.isDay = true;
					air.isSelectDay = false;
					air.isSelectNight = false;
				}
				hcu.UpdateSetpoints(isAirOn, DaySetpoint, NightSetpoint);
				hcu.needUpdate_f = true;
				setup.UpdateFanAutoGraph();
            } else
            if (group == 3) { // «Настройки»
				Group3PacketCount++;
				hcu.needUpdate_f = true;
				if (buf[1] < 3) clock.mode = buf[1]; // показывать часы в спящем режиме
				//
                if (buf[2]<24) air.dayTimeH = buf[2];                          // начало дня часы
                if (buf[3]<60) air.dayTimeM = buf[3];                         // начало дня минуты
				uint16_t daytime = air.dayTimeH*60+air.dayTimeM;
				if (daytime > DAYTIME_MAX) {
					daytime = DAYTIME_MAX;
				}
                if (buf[4]<24) air.nightTimeH = buf[4];                        // конец дня часы
                if (buf[5]<60) air.nightTimeM = buf[5];                        // конец дня минуты
				uint16_t nighttime = air.nightTimeH*60+air.nightTimeM;
				if (nighttime > NIGHTTIME_MAX) nighttime = NIGHTTIME_MAX;
				if ((nighttime - daytime) < DAYTIME_DUR_MIN) {
					nighttime = daytime+DAYTIME_DUR_MIN;
					if (nighttime > NIGHTTIME_MAX) {
						nighttime = NIGHTTIME_MAX;
						daytime = nighttime - DAYTIME_DUR_MIN;

					}
				}
				air.dayTimeH = daytime/60;
				air.dayTimeM = daytime%60;
				air.nightTimeH = nighttime/60;
				air.nightTimeM = nighttime%60;
				//
				uint16_t time = buf[6]*256+buf[7];
				if (time < 30) time = 30;
				if (time > 60) time = 60;
				hcu.durationDomesticWater = time; // минуты // ограничение длительности включения кнопки разбора воды
				time = (buf[8]*256+buf[9])*60;
				if (time < 60) time = 60;
				if (time > 7200) time = 7200;
                hcu.durationSystem = time;         // ограничение длительности включения системы в минутах
                if (buf[10]<3) display.setup.celsius = (buf[10]&3)-1;                  // фаренгейт/цельсий
                if (buf[11]<3) display.setup.h12 = (buf[11]&3)-1;                      // 12-ти часовая система
				display.setup.fahrenheit = !display.setup.celsius;
				air.isPanelSensor = buf[12]&1;
				setup.UpdateTimeSetGraph();
				setup.UpdateDayTimeGraph();
				setup.UpdateScheduleModeGraph();
				setup.UpdateCelsiusGraph();
//                if (buf[14]<51 && buf[14]>3) Control.UnderfloorSetpointTemp = buf[14];
//                if (buf[15]<81) Control.EngineSetpointTemp = buf[15];
//                if ((buf[16]*256+buf[17])>9 && (buf[16]*256+buf[17])<1451) Control.EngineTimeLimit = buf[16]*256+buf[17];
//                if (buf[18]>1 && buf[18]<11) Control.UnderfloorHysteresis=buf[18];
            } else
            if (group == 5) { // «Установка подключения»
                if (bluetooth.bufRx[1] == 1) {

                }
                else if (bluetooth.bufRx[1] == 2) {

                }
                else if (bluetooth.bufRx[1] == 0xAA) {
                    *(__IO uint32_t*) (NVIC_VectTab_RAM+1020) = 0x00000000;
                    NVIC_SystemReset();
                }
                else if (bluetooth.bufRx[1] == 0xA5) {
//                    Monitor.BootLoaderToWarning ();
                    *(__IO uint32_t*) (NVIC_VectTab_RAM+1020) = 0x0016AA55;
                    NVIC_SystemReset();
                }
            } else
            if (group == 7) { // «Время устройства»
				Group7PacketCount++;
				bool setf = false;
				if (clock.weekDay!=buf[4]) {
					clock.weekDay=buf[4];
					setf = true;
				}
				if (clock.hour!=buf[5]) {
					clock.hour=buf[5];
					setf = true;
				}
				if (clock.minute!=buf[6]) {
					clock.minute=buf[6];
					setf = true;
				}
				if (clock.second!=buf[7]) {
					clock.second=buf[7];
					setf = true;
				}
                if (setf) clock.timeRegulate();          
            }
        }
        if (isBleAccept==false && group == bluetooth.TYPE_ID) {

            if ((core.getTick() - timerCancel) >= 11000) {    // 10000
                for (i=0; i<16; i++) {
                    newId[i] = bluetooth.bufRx[i+1]; // первые 8 id телефона // вторые 8 id пульта(которые высланы были в 9 пакете)
                }
                if (!isBleAcceptPage) { // 

                    bool newDevice;
                    for (i=0; i<16; i++) {
                        newDevice = false;
                        for (x=0; x<8; x++) {
                            if (newId[x] != *(__IO uint8_t*)(BLE_ID_ADDRESS+16+i*16+x)) newDevice = true;
                        }
                        if (newDevice == false) break;
                    }
                    if (newDevice == false) {
                        for (x=0; x<8; x++) {
                            if (newId[8+x] != *(__IO uint8_t*)(BLE_ID_ADDRESS+8+x)) newDevice = true;
                        }
                    }
                    if (newDevice == true) {
                        timerCancel = core.getTick();
                        isBleCancel = false;
						screen_visible_old = SCREEN_VISIBLE_AIR;
                        screen_visible = SCREEN_VISIBLE_BLE_CONNECT;			
						
                    }
                    else {
                        isBleAccept = true;
                        isBleSendKey = false;
                    }
                    isBleAcceptPage = true;
                }
            }
        }
    }
	if (isBleAccept)
	{
		if (((core.getTick()-lastSendTick) >=100) || (updateGroup > 0))
		{
			lastSendTick=core.getTick();
			uint16_t valU16 = 0;
			for(uint8_t i=0; i<20; i++) {
				buf[i] = 0; // очищаем буфер
			}
			if (updateGroup > 0)
			{
				if (updateGroup == 1) packNum = 2;
				else if (updateGroup == 2) packNum = 3;
				else if (updateGroup == 3) packNum = 1;
	//			else if (updateGroup == 8) packNum = 0;
	//			else if (updateGroup == 9) packNum = 4;
				updateGroup = 0;
			}
			int16_t tempr = air.temperatureActual; // температура зоны
			if (air.isPanelSensor&0x01) { // если температура пульта, то передаем ее
				tempr = temperature.panel;
			}
			switch(packNum)
			{
			case 0:
				buf[0] = 8; // «Версии ПО и моточасы»

				buf[1] = hcu.heaterVersion[0];                // версия ПО подогревателя
				buf[2] = hcu.heaterVersion[1];                // версия ПО подогревателя
				buf[3] = hcu.heaterVersion[2];                // версия ПО подогревателя
				buf[4] = hcu.heaterVersion[3];                // версия ПО подогревателя

				buf[5] = *(__IO uint8_t*)(CRC_FIRMWARE_ADDRESS+6+0);                 // версия ПО пульта
				buf[6] = *(__IO uint8_t*)(CRC_FIRMWARE_ADDRESS+6+1);                 // версия ПО пульта
				buf[7] = *(__IO uint8_t*)(CRC_FIRMWARE_ADDRESS+6+2);                 // версия ПО пульта
				buf[8] = *(__IO uint8_t*)(CRC_FIRMWARE_ADDRESS+6+3);                 // версия ПО пульта

				buf[9]  = hcu.version[0];          // версия ПО центрального блока
				buf[10] = hcu.version[1];          // версия ПО центрального блока
				buf[11] = hcu.version[2];          // версия ПО центрального блока
				buf[12] = hcu.version[3];          // версия ПО центрального блока
			
				valU16 = hcu.heaterTotalOperatingTime/3600;
				buf[13] = (valU16>>8)&0xFF; // моточасы подогревателя
				buf[14] = valU16&0xFF;    // моточасы подогревателя
	//			mh = blablabla/3600;
//				buf[15] = (valU16>>8)&0xFF; // моточасы системы
//				buf[16] = valU16&0xFF;    // моточасы системы
				buf[15] = 0xFF; // моточасы системы
				buf[16] = 0xFF;    // моточасы системы
				break;
			case 1:
				buf[0] = 3;  // «Настройки» // bluetooth.TYPE_SETUP
			
				buf[1] = clock.mode;   // показывать часы в спящем режиме
				buf[2] = air.dayTimeH;
				buf[3] = air.dayTimeM;
				buf[4] = air.nightTimeH;
				buf[5] = air.nightTimeM;
				valU16 = hcu.durationDomesticWater; // минуты
				buf[6] = (valU16>>8) & 0xff; // ограничение длительности включения кнопки разбора воды
				buf[7] = valU16 & 0xff;
				valU16 = hcu.durationSystem/60; // переводим минуты в часы
				buf[8] = (valU16>>8)&0xFF;
				buf[9] = valU16 &0xFF;
				buf[10] = (display.setup.celsius+1) & 0x03; // 1-fahre // 2-cels
				buf[11] = (display.setup.h12+1) & 0x03; // 1 -24 // 2-12
			    buf[12] = air.isPanelSensor&1;
				buf[13] = 0; //Control.UnderfloorHeatingConnected|(Control.EnginePreheatConnected<<1);
				buf[14] = 20; //Control.UnderfloorSetpointTemp;
				buf[15] = 40; //Control.EngineSetpointTemp;
				buf[16] = 0xff; //Control.EngineTimeLimit>>8;
				buf[17] = 0xff; //Control.EngineTimeLimit;
				buf[18] = 2; //Control.UnderfloorHysteresis;
				break;
			case 2:
				buf[0] = 1; // «Текущие параметры»    // bluetooth.TYPE_STATUS
				for(uint8_t i=1; i<5; i++) {
					buf[1+i] = 0;
					buf[6+i] = 0;
				}
				if (air.isAirOn)
				{
					DaySetpoint=hcu.airHeaterTSetPoint[1];
					NightSetpoint=hcu.airHeaterTSetPoint[0];
				} else {
					if (DaySetpoint == 0) {
						DaySetpoint = hcu.airHeaterTSetPoint[1];
						NightSetpoint = hcu.airHeaterTSetPoint[0];
					}
				}
				if ((display.setup.scheduleMode & 1) == 0) {
					NightSetpoint = DaySetpoint;
				}
				if (display.setup.celsius & 0x01) 
				{
					buf[1]=DaySetpoint;
					buf[6]=NightSetpoint;
				} else {
					buf[1]= core.farToCel(DaySetpoint);
					buf[6]= core.farToCel(NightSetpoint);
				}
//				DaySetToPhone = buf[1];
//				NightSetToPhone = buf[6];
//				isAirOn = air.isAirOn;
//				if (display.setup.celsius & 0x01) 
//				{
//					buf[1]=hcu.airHeaterTSetPoint[0];
//					buf[6]=hcu.airHeaterTSetPoint[1];
//				} else {
//					buf[1]= core.celToFar(hcu.airHeaterTSetPoint[0]);
//					buf[6]= core.celToFar(hcu.airHeaterTSetPoint[1]);
//				}
				
				if (display.setup.celsius & 0x01) {
					buf[11] = tempr+40; //Температура (зона 1) (на +40 от фактического)
					buf[18] = tempr+40; //Уличная температура + 40
				} else {
					buf[11] = core.farToCel(tempr)+40; //Температура (зона 1) (на +40 от фактического)
					buf[18] = core.farToCel(tempr)+40; //Уличная температура + 40
				}
				
				for(uint8_t i=0; i<4; i++) {
					buf[12+i] = 0xff;
				}
				buf[16] = 0xff;
	//			buf[17] = hcu.stateHeater&0x01;
	//			buf[17] |= (hcu.stateAch&0x01)<<1;
				buf[17] = air.isFHeaterOn !=0;
				buf[17] |= ((air.isEHeaterOn !=0)<<1);
				buf[17] |= ((air.isWaterOn != 0)<<2); // кнопки по битам, разбор воды
	//            buf[17] |= (Control.UnderfloorButtonState!=0)<<5;
	//            buf[17] |= (Control.EnginePreheatButtonState!=0)<<6;

				buf[19] = display.setup.celsius; // 1-цельсии
				break;
			case 3:
				buf[0] = 2; // «Текущие параметры»        // bluetooth.TYPE_STATUS
				buf[1] = (hcu.fanManual<<7)&0x80;            // вентилятор ручной/авто, зона 1
				buf[1] |= hcu.fanPower&0x7f;               // процент ШИМа вентилятора, зона 1

	//            buf[1] = (Zone.FanManual_f[0]<<7)&0x80;            // вентилятор ручной/авто, зона 1
	//            buf[1] |= Zone.ManualPersent[0]&0x7f;               // процент ШИМа вентилятора, зона 1
	//            buf[2] = (Zone.FanManual_f[1]<<7)&0x80;            // вентилятор ручной/авто, зона 2
	//            buf[2] |= Zone.ManualPersent[1]&0x7f;               // процент ШИМа вентилятора, зона 2
	//            buf[3] = (Zone.FanManual_f[2]<<7)&0x80;            // вентилятор ручной/авто, зона 3
	//            buf[3] |= Zone.ManualPersent[2]&0x7f;               // процент ШИМа вентилятора, зона 3
	//            buf[4] = (Zone.FanManual_f[3]<<7)&0x80;            // вентилятор ручной/авто, зона 4
	//            buf[4] |= Zone.ManualPersent[3]&0x7f;               // процент ШИМа вентилятора, зона 4
	//            buf[5] = (Zone.FanManual_f[4]<<7)&0x80;            // вентилятор ручной/авто, зона 5
	//            buf[5] |= Zone.ManualPersent[4]&0x7f;               // процент ШИМа вентилятора, зона 5
				buf[6] = 0xff;
				buf[7]=hcu.faultCodeHcu;  //Ошибка 1
				buf[8]=hcu.faultCodeHeater;  //Ошибка 2
				buf[9]=hcu.faultCodePanel;  //Ошибка 3
				buf[10]=hcu.faultCode; //Ошибка 4
				buf[11]=0; //Ошибка 5
	//            buf[7]=Error.FaultCodeBuf[0];  //Ошибка 1
	//            buf[8]=Error.FaultCodeBuf[1];  //Ошибка 2
	//            buf[9]=Error.FaultCodeBuf[2];  //Ошибка 3
	//            buf[10]=Error.FaultCodeBuf[3]; //Ошибка 4
	//            buf[11]=Error.FaultCodeBuf[4]; //Ошибка 5
				buf[12] = 1; // Zone.Connected_f[0] | (Zone.Connected_f[1]<<4);
				buf[13] = 0; //Zone.Connected_f[2] | (Zone.Connected_f[3]<<4);
				buf[14] = 0; //Zone.Connected_f[4];
				buf[15] = air.isAirOn; //(Zone.State[0]&3) | ((Zone.State[1]&3)<<2) | ((Zone.State[2]&3)<<4) | ((Zone.State[3]&3)<<6);
				buf[16] = 0; //Zone.State[4]&3;
				buf[17] = air.isDay + (air.isSelectDay<<1) +(air.isSelectNight<<2)+(display.setup.scheduleMode<<3);
				buf[18] = 0;
				buf[19] = 0;
				break;
			case 4:
				if (isBleSendKey)
				{
					isBleSendKey = false;

					buf[0] = 9; // «Состояние подключения»
					buf[1] = isBleAccept;
					buf[2] = isBleCancel;
	//                buf[3] = hcu.version[0]; // версия ПО HCU
	//                buf[4] = hcu.version[1];
	//                buf[5] = hcu.version[2];
	//                buf[6] = hcu.version[3];
					buf[5] = *(__IO uint8_t*)(CRC_FIRMWARE_ADDRESS+6+0);                 // версия ПО пульта
					buf[6] = *(__IO uint8_t*)(CRC_FIRMWARE_ADDRESS+6+1);                 // версия ПО пульта
					buf[7] = *(__IO uint8_t*)(CRC_FIRMWARE_ADDRESS+6+2);                 // версия ПО пульта
					buf[8] = *(__IO uint8_t*)(CRC_FIRMWARE_ADDRESS+6+3);                 // версия ПО пульта
	//                buf[3] = *(__IO uint8_t*)(CRC_VERSION_ADR+6+0);
	//                buf[4] = *(__IO uint8_t*)(CRC_VERSION_ADR+6+1);
	//                buf[5] = *(__IO uint8_t*)(CRC_VERSION_ADR+6+2);
	//                buf[6] = *(__IO uint8_t*)(CRC_VERSION_ADR+6+3);
					buf[7] = 0;
					buf[8] = 0;
					buf[9] = 0;
					buf[10] = 0;
					buf[11] = 0;                // резерв
					for (x=0; x<8; x++)
					{
						buf[12+x] = *(__IO uint8_t*)(BLE_ID_ADDRESS+8+x);                // ключ (id BLE устройства)
					}
				}
				break;
			default:
				break;
			
			}
			bluetooth.sendArray(buf);
			sentMsgCounter++;
			packNum++;
			if (packNum >= 15) packNum = 0;
		}
	}
}
//-----------------------------------------------------
void BltHandler_C::addNewId(uint8_t *id)
{
  uint32_t a, N;
  uint8_t i, x, array[256+16+16+64];
  bool isEmpty;

  fmc_unlock();
  while(fmc_flag_get(FMC_FLAG_BANK0_BUSY) == SET)
    {
    }
  for (i=0; i<16; i++)
    {
      isEmpty = true;
      for (x=0; x<16; x++)
        {
          if (*(__IO uint8_t*)(BLE_ID_ADDRESS+16+16+i*16+x) != 0xFF) isEmpty = false;
        }
      if (isEmpty == true)
        {
          for (a=0; a<8; a+=4)
            {
              N = id[0+a];
              N += id[1+a]<<8;
              N += id[2+a]<<16;
              N += id[3+a]<<24;
              fmc_word_program(BLE_ID_ADDRESS+16+16+i*16+a, N);
            }
          fmc_lock();
          return;
        }
    }
  for (a=0; a<(256+16+16+64); a++)
    {
      array[a] = *(__IO uint8_t*)(BLE_ID_ADDRESS+a);
    }
  fmc_page_erase(BLE_ID_ADDRESS);
  while(fmc_flag_get(FMC_FLAG_BANK0_BUSY) == SET)
    {
    }
  for (a=32; a<(240+16+16); a++)
    {
      array[a] = array[a+16];
    }
  for (a=0; a<8; a++)
    {
      array[240+a+16+16] = id[a];
    }
  for (a=0; a<(256+16+16+64); a+=4)
    {
      N = array[0+a];
      N += array[1+a]<<8;
      N += array[2+a]<<16;
      N += array[3+a]<<24;
      fmc_word_program(BLE_ID_ADDRESS+a, N);
    }
  fmc_lock();
}
//-----------------------------------------------------
void BltHandler_C::setNewId(void)
{
  uint32_t a, N;
  uint8_t i, x, array[256+16+16+64];

  if (*(__IO uint8_t*)(BLE_ID_ADDRESS+5) == ' ' &&
      *(__IO uint8_t*)(BLE_ID_ADDRESS+6) == ' ' &&
      *(__IO uint8_t*)(BLE_ID_ADDRESS+7) == ' ')
    {

      fmc_unlock();
      while(fmc_flag_get(FMC_FLAG_BANK0_BUSY) == SET)
        {
        }
      for (a=0; a<(256+16+16+64); a++)
        {
          array[a] = *(__IO uint8_t*)(BLE_ID_ADDRESS+a);
        }
      fmc_page_erase(BLE_ID_ADDRESS);
      while(fmc_flag_get(FMC_FLAG_BANK0_BUSY) == SET)
        {
        }
      // ID
      for (a=0; a<3; a++)
        {
          x = 0;
          for (i=0; i<2; i++)
            {
              x |= temperature.array[a*2+i] & 0x0F;
              x = x<<4;
            }
          x = (x+0x20) & 0x3F;
          array[a+5] = x;
        }
      // KEY
      for (a=0; a<8; a++)
        {
          x = 0;
          for (i=0; i<4; i++)
            {
              x |= temperature.array[a*2+i] & 0x03;
              x = x<<2;
            }
          array[a+16] = x;
        }
      for (a=0; a<(256+16+16+64); a+=4)
        {
          N = array[0+a];
          N += array[1+a]<<8;
          N += array[2+a]<<16;
          N += array[3+a]<<24;
          fmc_word_program(BLE_ID_ADDRESS+a, N);
        }
      fmc_lock();
    }
}
//-----------------------------------------------------
void BltHandler_C::setNewKey(void)
{
  uint32_t a, N;
  uint8_t i;


  for (a=0; a<8; a++)
    {
      if (*(__IO uint8_t*)(BLE_ID_ADDRESS+8+a) != 0xFF)
        {
          return;
        }
    }

  fmc_unlock();
  while(fmc_flag_get(FMC_FLAG_BANK0_BUSY) == SET)
    {
    }
  for (a=0; a<2; a++)
    {
      N = 0;
      for (i=0; i<32; i++)
        {
          N |= temperature.array[a*6+i] & 0x01;
          N = N<<1;
        }
      fmc_word_program(BLE_ID_ADDRESS+8+a*4, N);
    }
  fmc_lock();
}
//-------------------------------------------------------------

