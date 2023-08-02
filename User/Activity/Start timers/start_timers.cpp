/******************************************************************************
* ��� ���������
* ������
* 
* ������������: ����� �.�.
* 
* 08.08.2018
* ��������:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "start_timers.h"
#include "start_timer.h"
#include "clock.h"
#include "hcu.h"
#include "core.h"

StartTimers startTimers;
//-----------------------------------------------------
StartTimers::StartTimers(void)
{
    core.ClassInit(this,sizeof(this));
}
//-----------------------------------------------------
void StartTimers::viewScreen(void)
{
    startTimer.viewScreen(2);
}
//-----------------------------------------------------
uint8_t StartTimers::handler(void)
{
    uint8_t result;
    
    result = startTimer.viewScreen(0);
    
    return result;
}
//-----------------------------------------------------
