/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Defines ------------------------------------------------------------------*/
#define THERMISTORNOMINAL   10000.0f
#define TEMPERATURENOMINAL  25.0f
#define BCOEFFICIENT        3988.0f
#define SERIESRESISTOR      10000.0f

/* Classes ------------------------------------------------------------------*/
class Temperature
{
    public:
        Temperature(void);
        void initialize(void);
        void request(void);
        int16_t getValue(void);
    
        static const int ARRAY_SIZE = 30;
        static const int VALS_SIZE = 100;
    
        int16_t panel;
        int16_t oldValue;
        uint8_t sourceOld;
        uint16_t array[ARRAY_SIZE];
    
        float Tcpu;                          // температура процессора
        float Uref;                          // опорное напряжение
        int32_t workTime;

    
    private:
        float steinhartHartEquation(uint16_t average);  // упрощенное B-parameter преобразование Стейнхарта — Харта
        float steinhartHartEquation(float average);     // упрощенное B-parameter преобразование Стейнхарта — Харта
    
        float tbuf[VALS_SIZE];
        uint8_t point, pmax;
        float steinhart;                                // переменная для преобразования сопротивления в температуру
        float correction;
    
};
extern Temperature temperature;

#endif /* __TEMPERATURE_H */
