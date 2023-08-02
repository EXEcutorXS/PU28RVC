/******************************************************************************
* ООО Теплостар
* Самара
* 
* Программисты: Клюев А.А.
* 
* 08.08.2018
* Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "temperature.h"
#include "core.h"
#include "display.h"
#include "sleep.h"
#include "math.h"
#include "main.h"
#include "pgn_rvc.h"

Temperature temperature;
/* Defines ------------------------------------------------------------------*/
/* Internal voltage reference calibration value address */
#define VREFINT_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7BA))
#define TS_CAL1_ADDR        ((uint16_t*) ((uint32_t) 0x1FFFF7B8))       // калибровочный коэф. 1 для датчика температуры
#define TS_CAL2_ADDR        ((uint16_t*) ((uint32_t) 0x1FFFF7C2))       // калибровочный коэф. 2 для датчика температуры

enum
{
    chTC1,                       // 4 Температура датчика
    chTcpu,                      // 16 Температура микроконтроллера
    chURef,                      // 17 Опорное напряжение для АЦП
};

//-----------------------------------------------------
Temperature::Temperature(void)
{
    	core.ClassInit(this,sizeof(this));
}
//-----------------------------------------------------
void Temperature::initialize(void)
{
    // Enable the GPIO Clock
    rcu_periph_clock_enable(RCU_GPIOA);
    
    // Configure the Temperature pin
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_4);
    
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV4);
    
    rcu_periph_clock_enable(RCU_ADC0);
    rcu_periph_clock_enable(RCU_DMA0);
    
    adc_disable(ADC0);
    adc_deinit(ADC0);
    dma_deinit(DMA0, DMA_CH0);
    
    adc_resolution_config(ADC0 , ADC_RESOLUTION_12B);
    adc_mode_config(ADC_MODE_FREE);
    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, ENABLE);        // !
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 3);
    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_4, ADC_SAMPLETIME_55POINT5);
    adc_regular_channel_config(ADC0, 1, ADC_CHANNEL_16, ADC_SAMPLETIME_55POINT5);
    adc_regular_channel_config(ADC0, 2, ADC_CHANNEL_17, ADC_SAMPLETIME_55POINT5);
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);        // !
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);        // !
    adc_flag_clear(ADC0,ADC_FLAG_EOIC);
    adc_tempsensor_vrefint_enable();
    adc_interrupt_disable(ADC0 , ADC_INT_EOC);
    adc_interrupt_disable(ADC0 , ADC_INT_EOIC);
    adc_enable(ADC0);
    adc_calibration_enable(ADC0);
    adc_dma_mode_enable(ADC0);
    
    dma_periph_address_config(DMA0, DMA_CH0, (uint32_t)(&ADC_RDATA(ADC0)));
    dma_memory_address_config(DMA0, DMA_CH0, (uint32_t)array);
    dma_circulation_enable(DMA0, DMA_CH0);
    dma_transfer_number_config(DMA0, DMA_CH0, ARRAY_SIZE);
    dma_priority_config(DMA0, DMA_CH0, DMA_PRIORITY_HIGH);
    dma_memory_width_config(DMA0, DMA_CH0, DMA_MEMORY_WIDTH_16BIT); // DMA_MEMORY_WIDTH_16BIT
    dma_periph_width_config (DMA0, DMA_CH0, DMA_PERIPHERAL_WIDTH_16BIT);    // DMA_PERIPHERAL_WIDTH_16BIT
    dma_memory_increase_enable(DMA0, DMA_CH0);
    dma_periph_increase_disable(DMA0, DMA_CH0);
    dma_transfer_direction_config(DMA0, DMA_CH0, DMA_PERIPHERAL_TO_MEMORY);
    
    dma_channel_enable(DMA0, DMA_CH0);
    adc_enable(ADC0);
    
    adc_calibration_enable(ADC0);
    adc_dma_mode_enable(ADC0);
}
//-----------------------------------------------------
void Temperature::request(void)
{
    static float val, ion;
    int32_t temp;

    temp = 3300 * array[chTcpu] / 4095;
    temp = 1450 - temp;
    Tcpu = temp/4.1 + 25.0;
    
    ion = 3300;
    Uref = ion / 1000.0f;
    
    val = (int16_t)(ion * this->array[chTC1]/4095);
    
    ////tbuf[point] = (val-2637)/(-13.6)-1.0;
    tbuf[point] = this->array[chTC1];
    
    if (point < (this->VALS_SIZE-1)) point++;
    else point = 0;
    if (point > pmax) pmax = point;
    float tmp = 0;
    for (uint8_t x = 0; x < pmax; x++){
        tmp += tbuf[x];
    }
    
    val = steinhartHartEquation(tmp/pmax);
    
    if (workTime < 100){
        correction = 0;
    }
    else if (workTime < 150){
        correction = -0.25;
    }
    else if (workTime < 200){
        correction = -0.5;
    }
    else if (workTime < 250){
        correction = -1.0;
    }
    else if (workTime < 350){
        correction = -1.0;
    }
    else if (workTime < 500){
        correction = -1.5;
    }
    else {
        correction = -1.5;
    }
    val += correction;
    
    #ifdef TEST_FROST_SENSOR
    val = -val;
    #endif
	
    canPGNRVC.panelSensorx10C = val*10;
    this->panel = (display.setup.celsius & 0x01)?(val):(core.celToFar(val));
	
}
//-----------------------------------------------------
int16_t Temperature::getValue(void)
{
    return this->panel;
}//-----------------------------------------------------
float Temperature::steinhartHartEquation (float average)
{
    ////   1     1     1        R     ////
    ////  --- = --- + --- x ln(---)   ////
    ////   T     To    B        Ro    ////
    steinhart = average;
    steinhart = 4095.0f / steinhart - 1.0f;                   // конвертируем значение в сопротивление
    steinhart = SERIESRESISTOR / steinhart;                 // конвертируем значение в сопротивление
    steinhart = steinhart / THERMISTORNOMINAL;              // (R/Ro)
    steinhart = log(steinhart);                             // ln(R/Ro)
    steinhart /= BCOEFFICIENT;                              // 1/B * ln(R/Ro)
    steinhart += 1.0f / (TEMPERATURENOMINAL + 273.15f);       // + (1/To)
    steinhart = 1.0f / steinhart;                            // инвертируем
    steinhart -= 273.15f;                                   // конвертируем в градусы по Цельсию
    return steinhart;
}
//-----------------------------------------------------
