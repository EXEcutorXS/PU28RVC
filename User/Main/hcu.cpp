//
//    ћодуль св€зи блока электронного с пультом (компьютером, стендом...)
//    по последовательному каналу св€зи USART
//    ¬ерси€ протокола обмена 4

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
//-----------------------------------------------------
void Hcu::handler(void)
{
    static uint32_t timerTimeOut = 0;
    static uint32_t timerRequest = 0;
    static bool isLinkError = false;
    static uint32_t timerLinkError = 0;
    uint8_t i;
    uint32_t val;

    if ((core.getTick() - timerTimeOut) >= 200) {
        timerTimeOut = core.getTick();
        usart.processTimeOut();
    }

	 if (usart.isTransmission == false && clearErrorRequest) {
		 
		 clearErrorRequest = false;
            usart.packetOut[1] = 3;      //это команда, нужен ответ
			usart.packetOut[2] = 0;      //ƒлина...данных нет, только команда
            usart.packetOut[3] = 0;
            usart.packetOut[4] = 5;
            usart.startTransmission();
        }
	 
    if ((core.getTick() - timerRequest) >= 500 && core.getTick()>3000) { //запрос на передачу данных по изменению в случае потери св€зи первые 3 секунды молчим чтобы получить данные из HCU
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
				if (rvcTemp==255) rvcTemp=254;
			}
			usart.packetOut[i++] = rvcTemp;
            
            usart.packetOut[2] = i-5;      //длина
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
}
//-----------------------------------------------------
void Hcu::parsing(void)
{
    uint16_t  Command;
    uint8_t i;
    int16_t val;
    bool unlocked = (core.getTick()-lockTimer) > 1000;
    if (usart.isProcessPacket){
        Command = usart.packetIn[2]*256+usart.packetIn[3];
        
            switch(Command) {
                case 15:
                    if (usart.packetIn[1] >= 36 && unlocked){
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
                    }
					   usart.linkCnt=0;
                    break;
                case 22:  //запуск загрузчика
                    if (usart.packetIn[1] == 2 
                     && usart.packetIn[4] == 0xAA
                     && usart.packetIn[5] == 0x55) {
                        *(__IO uint32_t*) (NVIC_VectTab_RAM+BOOT_FLAG_SHIFT) = 0x0016AA55;
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
