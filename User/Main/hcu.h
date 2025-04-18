
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HCU_H
#define __HCU_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"
//#include "abstract_usart_3.h"

/* Classes ------------------------------------------------------------------*/
class Hcu
{
    public:
        Hcu(void);
		void initialise(void);
        void handler(void);
        void parsing(void);
        void checkPump(void);
		void UpdateSetpoints(uint8_t airState, float daySetpoint, float nightSetpoint);
//        void UpdateSetups(void);
        char fanAuto;
        char fanManual;
        char pumpOn;
        int16_t fanPower;
        uint32_t timerPumpOn;
        uint8_t isError;
        uint8_t faultCode;
        //uint8_t errorOld;
        uint8_t stage;
        uint8_t stageOld;
        float airHeaterTSetPoint[2];
    
        uint8_t faultCodeHcu, faultCodeHeater, faultCodePanel;
        
        uint16_t durationDomesticWater; //in minutes
        uint16_t durationSystem;  // in minutes
        
        uint32_t timerOffDomesticWater;
        uint32_t timerOffSystem;
    
        float temperatureTank;
        float temperatureExchanger;
        float temperatureHeater;
        char stateHeater;
        char stateAch;
        char statePump;
        char stateZone0;
        char stateFuelPump;
        
        float pressure, voltage;
        uint32_t counterLink, secondsLink;
        
        int numRequest;
        uint32_t lockTimer;
        
        uint32_t tickTimer;
        uint8_t isConnect;

        static const uint8_t MODE_WAIT = 0x01;
        static const uint8_t MODE_START = 0x02;
        static const uint8_t MODE_WORK = 0x03;
        static const uint8_t MODE_STOP = 0x04;
    
        static const uint8_t CONNECT_STATUS_SEARCH = 0x00;
        static const uint8_t CONNECT_STATUS_OK = 0x01;
        static const uint8_t CONNECT_STATUS_ERROR = 0x02;

        bool isVersion;
        bool isHeaterVersion;

        uint8_t version[4];
        uint8_t heaterVersion[4];
        uint32_t heaterTotalOperatingTime; //����� ��������� �������������, �
		bool needUpdate_f;
		bool clearErrorRequest;
		
		uint32_t reinitialisationCounter;   			//������� ����������������� �����
		uint32_t uptime;
		uint32_t ReceivedByHCUPacketCounter;
		uint32_t ReceivedByPanelPacketCounter;
		//uint32_t faultedCommandCounter;         //������� ���������� �������. ���� �������� ��������� ������ �� ������������� ������� �����������.
		uint32_t restartCounter;
		uint8_t code14Counter;
		uint16_t Code14CounterTotal;
		bool Ignore_ReceivedByHCUPacketCounter_f;
    private:
		uint32_t LastRecPackCheckTick;
		uint32_t LastReceivedPacketTick;
		void RestoreConnection(void);
        
};
extern Hcu hcu;

#endif /* __HCU_H */
