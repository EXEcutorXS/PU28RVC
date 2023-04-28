/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PGN_RVC_H
#define __PGN_RVC_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"

/* Classes ------------------------------------------------------------------*/
class CAN_PGN_RVC
{
    public:
        CAN_PGN_RVC(void);
        void msgWaterHeater(void);
        void msgCirculationPumpStatus(void);
        void msgDiagnosticMessage(void);
        void msgFurnace(uint8_t ZoneNum);
        void msgThermostat1(uint8_t ZoneNum);
		    void msgThermostat2(uint8_t ZoneNum);
				void msgThermostatSchedule1(uint8_t ZoneNum, uint8_t scheduleInstance);
				void msgThermostatSchedule2(uint8_t ZoneNum, uint8_t scheduleInstance);
		    void msgAmbientTemp(uint8_t ZoneNum);
		    uint32_t generateAID(uint8_t priority, uint32_t DGN, uint8_t SA);
private:
};
extern CAN_PGN_RVC canPGNRVC;

#endif /* __PGN_RVC_H */
