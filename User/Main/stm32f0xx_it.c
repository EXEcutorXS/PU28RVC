/**
  ******************************************************************************
  * @file    USB_Example/stm32f0xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    17-January-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_it.h"
#include "usart.h"
#include "bluetooth.h"
#include "sensor.h"
#include "can.h"

/** @addtogroup STM32F072B_DISCOVERY
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/
bool usartIrqHandler(IRQn_Type usartIrq)
{
    if (usart.usartIrq == usartIrq) usart.usartIrqHandler();
    else return false;
  
    return true;
}
void USART1_IRQHandler(void)
{
    if (!usartIrqHandler(USART1_IRQn)){
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        USART_ClearITPendingBit(USART1, USART_IT_TC);
        USART_ClearITPendingBit(USART1, USART_IT_TXE);
        USART_ClearITPendingBit(USART1, USART_IT_ORE);
        USART_ClearITPendingBit(USART1, USART_IT_NE);
        USART_ClearITPendingBit(USART1, USART_IT_FE);
        USART_ClearITPendingBit(USART1, USART_IT_PE);
        USART_ClearITPendingBit(USART1, USART_IT_ORE);
        USART_ClearITPendingBit(USART1, USART_IT_NE);
        USART_ClearITPendingBit(USART1, USART_IT_FE);
        USART_ClearITPendingBit(USART1, USART_IT_PE);
    }
}
void USART2_IRQHandler(void)
{
    if (!usartIrqHandler(USART2_IRQn)){
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
        USART_ClearITPendingBit(USART2, USART_IT_TC);
        USART_ClearITPendingBit(USART2, USART_IT_TXE);
        USART_ClearITPendingBit(USART2, USART_IT_ORE);
        USART_ClearITPendingBit(USART2, USART_IT_NE);
        USART_ClearITPendingBit(USART2, USART_IT_FE);
        USART_ClearITPendingBit(USART2, USART_IT_PE);
        USART_ClearITPendingBit(USART2, USART_IT_ORE);
        USART_ClearITPendingBit(USART2, USART_IT_NE);
        USART_ClearITPendingBit(USART2, USART_IT_FE);
        USART_ClearITPendingBit(USART2, USART_IT_PE);
    }
}
void USART3_4_IRQHandler(void)
{
    if (!usartIrqHandler(USART3_4_IRQn)){
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
        USART_ClearITPendingBit(USART3, USART_IT_TC);
        USART_ClearITPendingBit(USART3, USART_IT_TXE);
        USART_ClearITPendingBit(USART3, USART_IT_ORE);
        USART_ClearITPendingBit(USART3, USART_IT_NE);
        USART_ClearITPendingBit(USART3, USART_IT_FE);
        USART_ClearITPendingBit(USART3, USART_IT_PE);
        USART_ClearITPendingBit(USART3, USART_IT_ORE);
        USART_ClearITPendingBit(USART3, USART_IT_NE);
        USART_ClearITPendingBit(USART3, USART_IT_FE);
        USART_ClearITPendingBit(USART3, USART_IT_PE);
      
        USART_ClearITPendingBit(USART4, USART_IT_RXNE);
        USART_ClearITPendingBit(USART4, USART_IT_TC);
        USART_ClearITPendingBit(USART4, USART_IT_TXE);
        USART_ClearITPendingBit(USART4, USART_IT_ORE);
        USART_ClearITPendingBit(USART4, USART_IT_NE);
        USART_ClearITPendingBit(USART4, USART_IT_FE);
        USART_ClearITPendingBit(USART4, USART_IT_PE);
        USART_ClearITPendingBit(USART4, USART_IT_ORE);
        USART_ClearITPendingBit(USART4, USART_IT_NE);
        USART_ClearITPendingBit(USART4, USART_IT_FE);
        USART_ClearITPendingBit(USART4, USART_IT_PE);
    }
}

void extiIrqHandler(IRQn_Type extiIrq)
{
    if (bluetooth.extiIrq == extiIrq) bluetooth.extiIrqHandler();
    if (sensor.extiIrq == extiIrq) sensor.extiIrqHandler();
    if (sensor.extiBtn == extiIrq) sensor.extiBtnHandler();
}
void EXTI0_1_IRQHandler(void)
{
    extiIrqHandler(EXTI0_1_IRQn);
}
void EXTI2_3_IRQHandler(void)
{
    extiIrqHandler(EXTI2_3_IRQn);
}
void EXTI4_15_IRQHandler(void)
{
    extiIrqHandler(EXTI4_15_IRQn);
}
/*
void canIrqHandler(uint8_t canIrq)
{
    if (can.canIrq == canIrq) can.canIrqHandler();
}
void CEC_CAN_IRQHandler(void)
{
    canIrqHandler(CEC_CAN_IRQn);
}
*/
void timIrqHandler(uint8_t timIrq)
{
    //if (led.timIrq == timIrq) led.timIrqHandler();
}
void TIM1_CC_IRQHandler(void)
{
    timIrqHandler(TIM1_CC_IRQn);
}
void TIM2_IRQHandler(void)
{
    timIrqHandler(TIM2_IRQn);
}
void TIM3_IRQHandler(void)
{
    timIrqHandler(TIM3_IRQn);
}
void TIM14_IRQHandler(void)
{
    timIrqHandler(TIM14_IRQn);
}
void TIM15_IRQHandler(void)
{
    timIrqHandler(TIM15_IRQn);
}
void TIM16_IRQHandler(void)
{
    timIrqHandler(TIM16_IRQn);
}
void TIM17_IRQHandler(void)
{
    timIrqHandler(TIM17_IRQn);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
