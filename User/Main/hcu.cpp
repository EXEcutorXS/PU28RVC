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
    static uint32_t timerTimeOut = 0;
    static uint32_t timerRequest = 0;
	static uint32_t Code14Timer = 0;
    static bool isLinkError = false;
    static uint32_t timerLinkError = 0;
	  static uint32_t ReceivedPacketCounterOld = 0;
    uint8_t i;
    uint32_t val;

	
	if ((core.getTick() - Code14Timer) >= 3600000) {
        timerTimeOut = core.getTick();
        if(code14Counter) code14Counter--;
    }
	
    if ((core.getTick() - timerTimeOut) >= 200) {
        timerTimeOut = core.getTick();
        usart.processTimeOut();
    }

	 if (usart.isTransmission == false && clearErrorRequest) {
		 
		 clearErrorRequest = false;
            usart.packetOut[1] = 3;      //это команда, нужен ответ
			usart.packetOut[2] = 0;      //Длина...данных нет, только команда
            usart.packetOut[3] = 0;
            usart.packetOut[4] = 5;
            usart.startTransmission();
        }
	 
    if (((core.getTick() - timerRequest) >= 500) && (core.getTick()>3000)) { //запрос на передачу данных по изменению в случае потери связи первые 3 секунды молчим чтобы получить данные из HCU
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
		}
	}    
    usart.processReceivedData();    
    parsing();
    
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
		if (core.getTick()-LastReceivedPacketTick>30000)
					NVIC_SystemReset();
						
		if (core.getTick() > 10000){
			if ((core.getTick()-LastRecPackCheckTick) > 30000)
			{
				LastRecPackCheckTick=core.getTick();
				if (core.getTick()-LastReceivedPacketTick>30000)
					NVIC_SystemReset();
				
			
				if (ReceivedByHCUPacketCounter==ReceivedPacketCounterOld){
					  NVIC_SystemReset();
				    //usart.initialize();
				}
				else{
					  ReceivedPacketCounterOld=ReceivedByHCUPacketCounter;
				}
	  }
}
		
}
//-----------------------------------------------------
void Hcu::parsing(void)
{
    uint16_t  Command;
    uint8_t i;
    int16_t val;
    bool unlocked = (core.getTick()-lockTimer) > 5000;
    
    //uint8_t tempF, tempE, tempW, tempA, tempM, tempP;
    
    if (usart.isProcessPacket){
        Command = usart.packetIn[2]*256+usart.packetIn[3];
		usart.lastReceivedTick=core.getTick();
        
            switch(Command) {
                case 15:
                    if (usart.packetIn[1] >= 36){
											LastReceivedPacketTick=core.getTick();
						          hcu.ReceivedByPanelPacketCounter++;
                        if (unlocked){
                            i = 4;
                            if (air.isFHeaterOn != usart.packetIn[i] ){
                                air.isFHeaterOn = usart.packetIn[i];
                            }
                            i++;
                            
                            if (air.isEHeaterOn != usart.packetIn[i]){
                                air.isEHeaterOn = usart.packetIn[i];
                            }
                            i++;
                            
                            if (air.isWaterOn != usart.packetIn[i]){
                                air.isWaterOn = usart.packetIn[i];
                            }
                            i++;
                            
                            if (air.isAirOn != usart.packetIn[i]){
                                air.isAirOn = usart.packetIn[i];
                                
                                if (air.isAirOn && !usart.packetIn[i]){
                                    slider.position = 0;
                                    hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)] = slider.values[slider.position];
                                }
                                air.isAirOn = usart.packetIn[i];
                            }
                            i++;
                            
                            if (display.setup.celsius & 0x01){
                                val = (usart.packetIn[i]<<8)+usart.packetIn[i+1];
                                canPGNRVC.tankTemperaturex10C = val;
                                hcu.temperatureTank = val/10.0;
                                i+=2;
                                
                                val = (usart.packetIn[i]<<8)+usart.packetIn[i+1];
                                canPGNRVC.heatExchangerTemperaturex10C = val;
                                hcu.temperatureExchanger = val/10.0;
                                i+=2;
                                
                                val = (usart.packetIn[i]<<8)+usart.packetIn[i+1];
                                canPGNRVC.airTemperaturex10C = val;
                                air.temperatureActual = val/10.0;
                                i+=2;
                                
                                val = (usart.packetIn[i]<<8)+usart.packetIn[i+1];
                                canPGNRVC.heaterTemperaturex10C = val;
                                hcu.temperatureHeater = val/10.0;
                                i+=2;
                            }
                            else{
                                val = (usart.packetIn[i]<<8)+usart.packetIn[i+1];
                                canPGNRVC.tankTemperaturex10C = val;
                                hcu.temperatureTank = core.celToFar(val/10.0);
                                i+=2;
                                
                                val = (usart.packetIn[i]<<8)+usart.packetIn[i+1];
                                canPGNRVC.heatExchangerTemperaturex10C = val;
                                hcu.temperatureExchanger = core.celToFar(val/10.0);
                                i+=2;
                                
                                val = (usart.packetIn[i]<<8)+usart.packetIn[i+1];
                                canPGNRVC.airTemperaturex10C = val;
                                air.temperatureActual = core.celToFar(val/10.0);
                                i+=2;
                                
                                val = (usart.packetIn[i]<<8)+usart.packetIn[i+1];
                                canPGNRVC.heaterTemperaturex10C = val;
                                hcu.temperatureHeater = core.celToFar(val/10.0);
                                i+=2;
                            }
                            
                            
                            hcu.stateHeater = usart.packetIn[i] ;
                            i++;
                            
                            hcu.stateAch = usart.packetIn[i];
                            i++;
                            
                            hcu.statePump = usart.packetIn[i];
                            i++;
                            
                            hcu.stateZone0 = usart.packetIn[i];
                            i++;
                            
                            hcu.stateFuelPump = usart.packetIn[i];
                            i++;
                            
                            hcu.fanManual = usart.packetIn[i];
                            hcu.fanAuto = !hcu.fanManual;
                            i++;
                            
                            hcu.fanPower = usart.packetIn[i];
                            i++;
                            
                            //hcu.pumpOn = usart.packetIn[i];
                            i++;
                            
                            hcu.faultCodeHcu = usart.packetIn[i];
                            i++;
                            
                            hcu.faultCodeHeater = usart.packetIn[i];
                            i++;
                            
                            hcu.heaterVersion[0] = usart.packetIn[i++];
                            hcu.heaterVersion[1] = usart.packetIn[i++];
                            hcu.heaterVersion[2] = usart.packetIn[i++];
                            hcu.heaterVersion[3] = usart.packetIn[i++];
                            hcu.isHeaterVersion=true;
                            
                            hcu.heaterTotalOperatingTime = usart.packetIn[i++]<<24;
                            hcu.heaterTotalOperatingTime += usart.packetIn[i++]<<16;
                            hcu.heaterTotalOperatingTime += usart.packetIn[i++]<<8;
                            hcu.heaterTotalOperatingTime += usart.packetIn[i++];
                            
                            hcu.version[0] = usart.packetIn[i++];
                            hcu.version[1] = usart.packetIn[i++];
                            hcu.version[2] = usart.packetIn[i++];
                            hcu.version[3] = usart.packetIn[i++];
                            hcu.isVersion=true;
                            
                            hcu.pressure = usart.packetIn[i];
                            i++;
                            
                            hcu.voltage = usart.packetIn[i]/10.0;
                            i++;
                            rvc.newState.FanCurrentSpeed=usart.packetIn[i];
							i++;
							uptime = usart.packetIn[i]*256+usart.packetIn[i+1];
							i++;
							i++;
							ReceivedByHCUPacketCounter = usart.packetIn[i]*256+usart.packetIn[i+1];
							i++;
							i++;				
							restartCounter=usart.packetIn[i]*256+usart.packetIn[i+1];				
                        }
												/*
                        else{
                            
                            i = 4;
                            //if (air.isFHeaterOn != usart.packetIn[i] ){
                            //    air.isFHeaterOn = usart.packetIn[i];
                            //}
                            tempF = usart.packetIn[i];
                            i++;
                            
                            //if (air.isEHeaterOn != usart.packetIn[i]){
                            //    air.isEHeaterOn = usart.packetIn[i];
                            //}
                            tempE = usart.packetIn[i];
                            i++;
                            
                            //if (air.isWaterOn != usart.packetIn[i]){
                            //    air.isWaterOn = usart.packetIn[i];
                            //}
                            tempW = usart.packetIn[i];
                            i++;
                            
                            if (air.isAirOn != usart.packetIn[i]){
                            //    air.isAirOn = usart.packetIn[i];
                            //    
                            //    if (air.isAirOn && !usart.packetIn[i]){
                            //        slider.position = 0;
                            //        hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)] = slider.values[slider.position];
                            //    }
                            //    air.isAirOn = usart.packetIn[i];
                            }
                            tempA = usart.packetIn[i];
                            i++;
                            
                            //val = (usart.packetIn[i]<<8)+usart.packetIn[i+1];
                            //canPGNRVC.tankTemperaturex10C = val;
                            //hcu.temperatureTank = val/10.0;
                            i+=2;
                            
                            //val = (usart.packetIn[i]<<8)+usart.packetIn[i+1];
                            //canPGNRVC.heatExchangerTemperaturex10C = val;
                            //hcu.temperatureExchanger = val/10.0;
                            i+=2;
                            
                            //val = (usart.packetIn[i]<<8)+usart.packetIn[i+1];
                            //canPGNRVC.airTemperaturex10C = val;
                            //air.temperatureActual = val/10.0;
                            i+=2;
                            
                            //val = (usart.packetIn[i]<<8)+usart.packetIn[i+1];
                            //canPGNRVC.heaterTemperaturex10C = val;
                            //hcu.temperatureHeater = val/10.0;
                            i+=2;
                                
                            //hcu.stateHeater = usart.packetIn[i] ;
                            i++;
                            
                            //hcu.stateAch = usart.packetIn[i];
                            i++;
                            
                            //hcu.statePump = usart.packetIn[i];
                            i++;
                            
                            //hcu.stateZone0 = usart.packetIn[i];
                            i++;
                            
                            //hcu.stateFuelPump = usart.packetIn[i];
                            i++;
                            
                            //hcu.fanManual = usart.packetIn[i];
                            //hcu.fanAuto = !hcu.fanManual;
                            tempM = usart.packetIn[i];
                            i++;
                            
                            //hcu.fanPower = usart.packetIn[i];
                            i++;
                            
                            //hcu.pumpOn = usart.packetIn[i];
                            tempP = usart.packetIn[i];
                            i++;
                            
                                
                            
                            uint8_t receivedState = (tempF!=0) | 
                                                    ((tempE!=0)<<1) | 
                                                    ((tempW!=0)<<2) | 
                                                    ((tempA!=0)<<3) | 
                                                    ((tempM)<<4) | 
                                                    ((tempP!=0)<<5);
                

                        }
												*/
						
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
