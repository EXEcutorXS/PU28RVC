/******************************************************************************
* ООО Теплостар
* Самара
* 
* Программисты: Клюев А.А.
* 
* 08.10.2021
* Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "sleep.h"
#include "temperature.h"
#include "core.h"
#include "main.h"
#include "string.h"
Core core;
//-----------------------------------------------------
Core::Core(void)
{
		ClassInit(this,sizeof(this));
    this->timePhase=0xFFFFFFFF;
    this->command=0;
    
    //this->tickTimer = -30;
}
//-----------------------------------------------------
void Core::initialize(void)
{
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock/1000);
    NVIC_SetPriority(SysTick_IRQn, 0);
    
    rcu_periph_clock_enable(RCU_TIMER5);    
    timer_prescaler_config(TIMER5, SystemCoreClock/1000000-1, TIMER_PSC_RELOAD_NOW);
    timer_autoreload_value_config(TIMER5, (uint16_t)0xFFFF);
    timer_enable(TIMER5);
}
//-----------------------------------------------------
void Core::delayUs(uint32_t delay)
{
//    TIM6->ARR = delay;
//    TIM6->EGR |= TIM_EGR_UG;
//    TIM6->CR1 |= TIM_CR1_CEN|TIM_CR1_OPM;
//    while ((TIM6->CR1&TIM_CR1_CEN)!=0);
    timer_disable(TIMER5);
    timer_counter_value_config(TIMER5, (uint16_t)delay);   // TIM6->ARR = delay;
    TIMER_SWEVG(TIMER5) |= (uint32_t)TIMER_SWEVG_UPG;   // TIM6->EGR |= TIM_EGR_UG;
    // TIM6->CR1 |= TIM_CR1_CEN|TIM_CR1_OPM;
    timer_single_pulse_mode_config(TIMER5, TIMER_SP_MODE_SINGLE);
    timer_enable(TIMER5);
    while (TIMER_CNT(TIMER5) != 0);   // while ((TIM6->CR1&TIM_CR1_CEN)!=0);
}
//-----------------------------------------------------
void Core::delayMs(uint32_t delay)
{
	uint32_t timer = core.getTick();
    while((core.getTick() - timer) < delay);
}
//-----------------------------------------------------
void Core::protectedFlash(void)
{
    /*
    if (FLASH_GetReadOutProtectionStatus() == RESET){
        fmc_unlock();
        FLASH_ReadOutProtection(ENABLE);
        fmc_lock();
    }
    */
}
//-----------------------------------------------------
void Core::remapTable(void)
{
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, MAIN_PROGRAM_START_ADDRESS - FLASH_BASE);
}
//-----------------------------------------------------
void Core::incTick(void)
{
    static uint16_t temperatureCount = 0;
    
    this->msTick++;
    
    if (sleep.isLowPower){
        if (temperatureCount < 2000) temperatureCount++;
        else{
            temperatureCount = 0;
            if (temperature.workTime > 50) temperature.workTime--;
        }
    }
    else{
        if (temperatureCount < 1000) temperatureCount++;
        else{
            temperatureCount = 0;
            if (temperature.workTime < 500) temperature.workTime++;
        }
    }
}
//-----------------------------------------------------
uint32_t Core::getTick(void)
{
    return this->msTick;
}
//-----------------------------------------------------
void Core::setTimer(uint32_t value)
{
    this->tickTimer = this->msTick + value;
}
//-----------------------------------------------------
uint32_t Core::getTimer(void)
{
    int32_t i;
    
    i = this->tickTimer - this->msTick;
    if (i < 0) i = 0;
    return i;
}
//-----------------------------------------------------
void Core::resetTimer(void)
{
    this->tickTimer = this->msTick;
}
//-----------------------------------------------------
float Core::celToFar(float cel)
{
    return (9.0f / 5.0f * cel + 32.0f + 0.5f);
}
//-----------------------------------------------------
float Core::farToCel(float far)
{
    return (5.0f / 9.0f * (far - 32.0f + 0.5f));
}
//-----------------------------------------------------
void Core::packFloat(uint8_t *dst, float val)
{
    *dst++ = (uint16_t)(val)>>8;
    *dst++ = (uint8_t)(val);
}
//-----------------------------------------------------
uint16_t Core::getDelta(int16_t valA, int16_t valB)
{
    int16_t val;
    
    val = valA-valB;
    if (val < 0) val = -val;
    return val;
}
//-----------------------------------------------------
void Core::paramToProtocol(volatile uint8_t* array, volatile uint8_t group, volatile uint8_t* name, volatile uint8_t type, volatile uint32_t addr, volatile uint8_t* val)
{
    *array++ = group;
    *array++ = *name++;
    *array++ = *name++;
    *array++ = *name++;
    *array++ = type;
    *array++ = addr&0xFF;
    *array++ = (addr>>8)&0xFF;
    *array++ = (addr>>16)&0xFF;
    *array++ = *val++;
    *array++ = *val++;
    *array++ = *val++;
    *array++ = *val++;
    
    return;
}

void Core::ClassInit(void* address,uint32_t size)
{
	memset(address,0,size);
}

//-----------------------------------------------------
extern "C" void SysTick_Handler(void)
{
    core.incTick();         // системный таймер
}
//-----------------------------------------------------
extern "C" void NMI_Handler(void)
{
    
}
//-----------------------------------------------------
extern "C" void HardFault_Handler(void)
{
  NVIC_SystemReset();
}
//-----------------------------------------------------
