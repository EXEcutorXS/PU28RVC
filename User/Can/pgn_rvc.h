/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PGN_RVC_H
#define __PGN_RVC_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Classes ------------------------------------------------------------------*/
class CAN_PGN_RVC
{
    public:
        CAN_PGN_RVC(void);
        uint32_t generateAID(uint8_t priority, uint32_t DGN, uint8_t SA);
        void msgWaterHeater(void);
		void msgWaterHeater2(void);
        void msgCirculationPumpStatus(void);
        void msgDiagnosticMessage(void);
        void msgFurnace();
        void msgThermostat1();
        void msgThermostat2();
        void msgThermostatSchedule1(uint8_t scheduleInstance);
        void msgAmbientTemp();
		void msgExtMessage();
		void msgTimers();
		void msgHeaterInfo();

		int16_t tankTemperaturex10C;
		int16_t airTemperaturex10C;
		int16_t heatExchangerTemperaturex10C;
		int16_t heaterTemperaturex10C;
		int16_t panelSensorx10C;
        private:
            
};
extern CAN_PGN_RVC canPGNRVC;

#endif /* __PGN_RVC_H */
