/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSTEM_H
#define __SYSTEM_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Defines ------------------------------------------------------------------*/

/* Classes ------------------------------------------------------------------*/
class System
{
    public:
        System(uint8_t offVal, uint8_t minVal, uint8_t maxVal, uint8_t startVal, uint8_t offImage, uint8_t onImage, uint8_t smallStep, uint8_t bigStep);
        void viewScreen(bool isFirst);
        uint8_t viewHandler(void);
        void viewMessage(char *txt);
        void viewMode(void);
        void viewDuration(void);
        void viewDuration(bool isVisible, uint16_t durationVal);
        void viewTemperature(bool isReset);
        uint8_t sensorCheck(void);
        void buttonPress(void);
        void buttonCheck(uint8_t mode);
        void checkConnect(void);
        void checkDayNight(void);
        void changeScale(uint8_t offVal, uint8_t minVal, uint8_t maxVal, uint8_t startVal, uint8_t offImage, uint8_t onImage, uint8_t smallStep, uint8_t bigStep);
        
        bool isWaterOn;
        bool isFHeaterOn;
        bool isEHeaterOn;
        bool isAirOn;
        char isPanelSensor;
    
        bool isDay, isSelectDay, isSelectNight;
        bool isResetSelectDayNight;
        uint32_t timerResetSelectDayNight;
        
        uint8_t dayTimeH, dayTimeM;
        uint8_t nightTimeH, nightTimeM;
        
        uint8_t value;
        int16_t temperatureActual;
        bool isChange;
    
        uint8_t OFF_VALUE;
        
        uint8_t isSetDuration;
        uint32_t timerSetDuration;
        uint16_t durationVal;
        
    private:
		void BleStatusUpdater(void);
		void BleStatusView(void);
		void ScheduleModeView(void);
        bool isWaterOnOld;
        bool isFHeaterOnOld;
        bool isEHeaterOnOld;
        uint8_t OFF_IMAGE;
        uint8_t ON_IMAGE;
        uint8_t SMALL_STEP;
        uint8_t BIG_STEP;
        uint8_t MIN_VALUE;
        uint8_t MAX_VALUE;
        uint8_t START_VALUE;
        int16_t temperatureActualOld;
        
};
extern System air;

/* Info ------------------------------------------------------------------*/

#endif /* __SYSTEM_H */
