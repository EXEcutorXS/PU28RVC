/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BLT_H
#define __BLT_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Defines ------------------------------------------------------------------*/


/* Classes ------------------------------------------------------------------*/
class BltHandler_C
{
    public:
        BltHandler_C(void);
        void initialize(void);
        void handler(void);
		void addNewId(uint8_t *id);
		void setNewId(void);
		void setNewKey(void);
		uint8_t isBleAccept;
		uint8_t isBleCancel;
		uint8_t isBleSendKey;
		uint8_t isBleAcceptPage;
		uint32_t timerCancel;
		uint8_t newId[16];
		float DaySetpoint;
		float NightSetpoint;
     
    private:

		uint32_t counterBleTemp, counterBleSendTemp; // for debug !!!!
		uint32_t counterBle, counterBleSend;          // for debug !!!!
		uint32_t receivedMsgCounter, receivedMessages;// for debug !!!!
		uint32_t sentMsgCounter, sentMessages;       // for debug !!!!

		uint32_t BLE_SEND_LONG_PERIOD;
		uint32_t BLE_SEND_SHORT_PERIOD;
		uint32_t BLE_PAUSE_PERIOD;
		uint8_t buf[20];
		uint32_t connectTimout;
		uint8_t packNum;
		uint8_t updateGroup;
		uint8_t isAirOn;

		uint16_t Group1PacketCount;  // for debug !!!!
		uint16_t Group2PacketCount; // for debug !!!!
		uint16_t Group3PacketCount; // for debug !!!!
		uint16_t Group7PacketCount; // for debug !!!!
		int8_t DaySetFromPhone; // 
		int8_t NightSetFromPhone; // 
//		int8_t DaySetToPhone; // for debug !!!!
//		int8_t NightSetToPhone; // for debug !!!!
		
       
};
extern BltHandler_C blt;

#endif /* __BLT_H */
