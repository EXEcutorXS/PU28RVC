//
//    Модуль связи блока электронного с пультом (компьютером, стендом...)
//    по последовательному каналу связи USART
//    Версия протокола обмена 4

/* Includes ------------------------------------------------------------------*/
#include "hcu.h"
#include "usart.h"
#include "system.h"
#include "error.h"
#include "temperature.h"
#include "display.h"
#include "slider.h"
#include "main.h"
#include "pgn_rvc.h"
#include "rvc.h"

Hcu hcu;
//-----------------------------------------------------
Hcu::Hcu(void)
{
   clearErrorRequest = false; 
}

void Hcu::initialise(void)
{
lockTimer=100000;
	LastRecPackCheckTick=0;
	restartCounter=0;
}
//-----------------------------------------------------
void Hcu::handler(void)
{
//    static uint32_t timerTimeOut = 0;
    static uint32_t timerRequest = 0;
	static uint32_t timerReceiveHold = 0;
	static uint32_t Code14Timer = 0;
    static bool isLinkError = false;
    static uint32_t timerLinkError = 0;
	static uint32_t ReceivedPacketCounterOld = 0;
    uint8_t i;
    uint32_t val;
	
	if ((core.getTick() - Code14Timer) >= 1800000) {
        Code14Timer = core.getTick();
        if(code14Counter) code14Counter--;
    }
//    if ((core.getTick() - timerTimeOut) >= 200) {
//        timerTimeOut = core.getTick();
//    }
	usart.processReceivedData();
		
	if (needUpdate_f)
	{
		timerReceiveHold = core.getTick();
		usart.isProcessPacket = false; // перед парсингом чтоб не обновлять старыми данными
	} else {		
		if ((core.getTick() - timerReceiveHold) >= 150) {	
			parsing();	
		} else {
			usart.isProcessPacket = false; 
		}
	}
	
	 if (usart.isTransmission == false && clearErrorRequest) {		 
		 clearErrorRequest = false;
            usart.packetOut[1] = 3;      //это команда, нужен ответ
			usart.packetOut[2] = 0;      //Длина...данных нет, только команда
            usart.packetOut[3] = 0;
            usart.packetOut[4] = 5;
            usart.startTransmission();
    }
	 
	if (core.getTick() > 5000)
	{
		if (((core.getTick() - timerRequest) >= 1000)||(needUpdate_f)) { //запрос на передачу данных по изменению в случае потери связи первые 3 секунды молчим чтобы получить данные из HCU
			timerRequest = core.getTick();
			if (usart.isTransmission == false) {
				usart.packetOut[1] = 3;      //это команда, нужен ответ
				usart.packetOut[3] = 0;
				usart.packetOut[4] = 15;
				i = 5;
				usart.packetOut[i++] = air.isFHeaterOn;
				usart.packetOut[i++] = air.isEHeaterOn;
				usart.packetOut[i++] = air.isWaterOn;
				usart.packetOut[i++] = air.isAirOn;
				if (display.setup.celsius & 0x01){
					usart.packetOut[i++] = airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)];
				}
				else {
					usart.packetOut[i++] = core.farToCel(airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)]);
				}
				usart.packetOut[i++] = fanManual;
				usart.packetOut[i++] = fanPower;
				usart.packetOut[i++] = pumpOn;
				if (display.setup.celsius & 0x01){
					usart.packetOut[i++] = temperature.panel;
				}
				else {
					usart.packetOut[i++] = core.farToCel(temperature.panel);
				}
				usart.packetOut[i++] = air.isPanelSensor;
				uint8_t rvcTemp=255;
				if (rvc.externalTemperatureProvided)
				{
					rvcTemp=rvc.externalTemperature+75;
					if (rvcTemp==255) rvcTemp=254; // 255 - no data
				}
				usart.packetOut[i++] = rvcTemp;
				usart.packetOut[i++] = display.setup.tempShift; //Temperature Adjustment
				
				usart.packetOut[2] = i-5;      //длина
				
				//Запоминаем последнюю команду и всемё её выдачи
				usart.lastSendedState = (air.isFHeaterOn!=0) | 
										((air.isEHeaterOn!=0)<<1) | 
										((air.isWaterOn!=0)<<2) | 
										((air.isAirOn!=0)<<3) | 
										((fanManual!=0)<<4) | 
										((pumpOn!=0)<<5);
				usart.lastCommandSendTick = core.getTick();
				//Начинаем передачу
				usart.startTransmission();
				if (needUpdate_f) {
					usart.isProcessPacket = false; 
					needUpdate_f = false;
				}
			}
		} 
	} else 	{
		if (core.getTick() > 2000)
		{
			if ((core.getTick() - timerRequest) >= 700) { //запрос  данных 
				timerRequest = core.getTick();
			//	if (usart.isTransmission == false) {
					usart.packetOut[0] = 0xAA;  
					usart.packetOut[1] = 3;      //это команда, нужен ответ
					usart.packetOut[2] = 0;      // число данных
					usart.packetOut[3] = 0;      // cmd1
					usart.packetOut[4] = 15;     // cmd2
					usart.startTransmission();
			//	}
			}
		}
	}		 
//    parsing();
    
    if (faultCodeHcu) faultCode = faultCodeHcu;
    else if (faultCodeHeater) faultCode = faultCodeHeater;
    else if (faultCodePanel) faultCode = faultCodePanel;
    else faultCode = 0;
    if (faultCode == 0) error.codeOld = 0;
    
    if ((faultCodeHcu == 48) && (isLinkError == false)){
        isLinkError = true;
        counterLink++;
        timerLinkError = core.getTick();
    }
    if ((faultCodeHcu == 0) && (isLinkError == true)){
        isLinkError = false;
        val = (core.getTick() - timerLinkError)/1000;
        if (val > secondsLink) secondsLink = val;
    }
	checkPump();
		
	if (core.getTick() > 10000){
		if (core.getTick()-LastReceivedPacketTick > 30000) {
			RestoreConnection();
		}
		if ((core.getTick()-LastRecPackCheckTick) > 30000)
		{
			LastRecPackCheckTick=core.getTick();
			if ((ReceivedByHCUPacketCounter==ReceivedPacketCounterOld)&&(!Ignore_ReceivedByHCUPacketCounter_f)) {
				RestoreConnection();
			}
			else{
				ReceivedPacketCounterOld=ReceivedByHCUPacketCounter;
			}
		}
	}
}
//-----------------------------------------------------
void Hcu::RestoreConnection(void)
{
	//NVIC_SystemReset();
//	uint8_t flag = 1;
}
//----------------
void Hcu::UpdateSetpoints(uint8_t airState, float daySetpoint, float nightSetpoint)
{
	if (airState == 0)
	{
		air.isAirOn = 0;
		slider.position = 0;
		hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)] = (float)slider.values[slider.position];
	} else {
		air.isAirOn = 1;
		hcu.airHeaterTSetPoint[1] = daySetpoint;
		hcu.airHeaterTSetPoint[0] = nightSetpoint;
		slider.setPosition((uint16_t)hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)]);
	}
}
//-----------------------------------------------------
void Hcu::parsing(void)
{
    uint16_t  Command;

    int16_t val;
    bool unlocked = (core.getTick()-lockTimer) > 5000;
    
    if (usart.isProcessPacket){
        Command = usart.packetIn[2]*256+usart.packetIn[3];
		usart.lastReceivedTick=core.getTick();
        
            switch(Command) {
                case 15:
                    if (usart.packetIn[1] >= 36){
						LastReceivedPacketTick=core.getTick();
					    hcu.ReceivedByPanelPacketCounter++;
                        if (unlocked){
                            if (air.isFHeaterOn != usart.packetIn[4] ){
                                air.isFHeaterOn = usart.packetIn[4];
                            }
                            if (air.isEHeaterOn != usart.packetIn[5]){
                                air.isEHeaterOn = usart.packetIn[5];
                            }
                            if (air.isWaterOn != usart.packetIn[6]){
                                air.isWaterOn = usart.packetIn[6];
                            }
                            if (air.isAirOn != usart.packetIn[7]){
                                air.isAirOn = usart.packetIn[7];
                                
                                if (air.isAirOn && !usart.packetIn[7]){
                                    slider.position = 0;
                                    hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)] = slider.values[slider.position];
                                }
                                air.isAirOn = usart.packetIn[7];
                            }
                            if (display.setup.celsius & 0x01){
                                val = (usart.packetIn[8]<<8)+usart.packetIn[9];
                                canPGNRVC.tankTemperaturex10C = val;
                                hcu.temperatureTank = val/10.0;

                                val = (usart.packetIn[10]<<8)+usart.packetIn[11];
                                canPGNRVC.heatExchangerTemperaturex10C = val;
                                hcu.temperatureExchanger = val/10.0;

                                val = (usart.packetIn[12]<<8)+usart.packetIn[13];
                                canPGNRVC.airTemperaturex10C = val;
                                air.temperatureActual = val/10.0;

                                val = (usart.packetIn[14]<<8)+usart.packetIn[15];
                                canPGNRVC.heaterTemperaturex10C = val;
                                hcu.temperatureHeater = val/10.0;

                            }
                            else{
                                val = (usart.packetIn[8]<<8)+usart.packetIn[9];
                                canPGNRVC.tankTemperaturex10C = val;
                                hcu.temperatureTank = core.celToFar(val/10.0);

                                val = (usart.packetIn[10]<<8)+usart.packetIn[11];
                                canPGNRVC.heatExchangerTemperaturex10C = val;
                                hcu.temperatureExchanger = core.celToFar(val/10.0);
    
                                val = (usart.packetIn[12]<<8)+usart.packetIn[13];
                                canPGNRVC.airTemperaturex10C = val;
                                air.temperatureActual = core.celToFar(val/10.0);

                                val = (usart.packetIn[14]<<8)+usart.packetIn[15];
                                canPGNRVC.heaterTemperaturex10C = val;
                                hcu.temperatureHeater = core.celToFar(val/10.0);

                            }                       
                            
                            hcu.stateHeater = usart.packetIn[16] ;
                            hcu.stateAch = usart.packetIn[17];
                            hcu.statePump = usart.packetIn[18];
                            hcu.stateZone0 = usart.packetIn[19];
                            hcu.stateFuelPump = usart.packetIn[20];
                            hcu.fanManual = usart.packetIn[21];
                            hcu.fanAuto = !hcu.fanManual;
                            hcu.fanPower = usart.packetIn[22];
						}
                            //hcu.pumpOn = usart.packetIn[23];
                            hcu.faultCodeHcu = usart.packetIn[24];
                            hcu.faultCodeHeater = usart.packetIn[25];
                            hcu.heaterVersion[0] = usart.packetIn[26];
                            hcu.heaterVersion[1] = usart.packetIn[27];
                            hcu.heaterVersion[2] = usart.packetIn[28];
                            hcu.heaterVersion[3] = usart.packetIn[29];
                            hcu.isHeaterVersion=true;
                            hcu.heaterTotalOperatingTime = usart.packetIn[30]<<24;
                            hcu.heaterTotalOperatingTime += usart.packetIn[31]<<16;
                            hcu.heaterTotalOperatingTime += usart.packetIn[32]<<8;
                            hcu.heaterTotalOperatingTime += usart.packetIn[33];
                            hcu.version[0] = usart.packetIn[34];
                            hcu.version[1] = usart.packetIn[35];
                            hcu.version[2] = usart.packetIn[36];
                            hcu.version[3] = usart.packetIn[37];
                            hcu.isVersion=true;
                            hcu.pressure = usart.packetIn[38];
                            hcu.voltage = (float)usart.packetIn[39]/10.0f;
                            rvc.newState.FanCurrentSpeed=usart.packetIn[40];
							if (usart.packetIn[1] >= 43){
								Ignore_ReceivedByHCUPacketCounter_f = false;
								uptime = usart.packetIn[41]*256+usart.packetIn[42];
								ReceivedByHCUPacketCounter = usart.packetIn[43]*256+usart.packetIn[44];				
								restartCounter=usart.packetIn[45]*256+usart.packetIn[46];
							} else {
								Ignore_ReceivedByHCUPacketCounter_f = true;
							}								
	
                    }
					   usart.linkCnt=0;
                    break;
                case 22:  //запуск загрузчика
                    if (usart.packetIn[1] == 2 
                     && usart.packetIn[4] == 0xAA
                     && usart.packetIn[5] == 0x55) {
                        *(__IO uint32_t*) (NVIC_VectTab_RAM+BOOT_FLAG_SHIFT) = 0x0016AA55;
						 *(__IO uint32_t*) (NVIC_VectTab_RAM+BOOT_FLAG_SHIFT2) = 0x0016AA55;
                        NVIC_SystemReset();
                    }
                    break;
                default:
                    break;
            }
		
        
        usart.isProcessPacket = false;
    }
}
//-----------------------------------------------------
void Hcu::checkPump(void) 
{
    if (hcu.pumpOn == 1){
        if ((core.getTick() - hcu.timerPumpOn) > 600000){
            hcu.pumpOn = 2;
        }
    }
}
//-----------------------------------------------------
