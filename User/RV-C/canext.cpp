/********************************************************************
* ��� ���������
* ������
* 
* ������������: �������� �.�.
* 
* __.09.2020
* ��������:
********************************************************************/
/* Includes -------------------------------------------------------*/
#include "canext.h"
#include "core.h"
#include "stm32f2xx.h"
#include "stm32f2xx_gpio.h"
#include "stm32f2xx_rcc.h"
#include "stm32f2xx_can.h"
#include "misc.h"


CanExt canExt;

//-------------------------------------------------------------------
CanExt::CanExt(void)
{
    for (uint8_t i=0; i<MSG_BUF_SIZE; i++) {
        this->RxMsgBuf[i].Received_f=false;
        this->TxMsgBuf[i].Transmitted_f=true;
    }
    this->FaultLink_f=false;
    this->FaultCnt=0;
    this->RxMailBoxReady_f=true;
    this->Recovery=0;
}
//-------------------------------------------------------------------
void CanExt::initialize(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    CAN_InitTypeDef CAN_InitStructure;

    // CAN GPIOs configuration
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);            // �������� ������������ �����
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);            // �������� ������������ �����
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);           // �������� ������������ �����

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);             // �������� ������������ CAN-����

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_CAN2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_CAN2);

    // ����������� CAN RX pin
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // ����������� CAN TX pin
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // ����������� CAN SHDN pin
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOC, GPIO_Pin_13); //SHDN OFF

    CAN_DeInit(CAN2);

    // CAN cell init
    CAN_StructInit(&CAN_InitStructure);

    CAN_InitStructure.CAN_ABOM = ENABLE;
    CAN_InitStructure.CAN_NART = ENABLE;
    CAN_InitStructure.CAN_TXFP = ENABLE;
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;                    // ������� ����� ������ ����������
    //CAN_InitStructure.CAN_Mode = CAN_Mode_LoopBack;                // ��� ������������ ��� ������������ ��������� ����

    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
    CAN_InitStructure.CAN_BS1 = CAN_BS1_6tq;
    CAN_InitStructure.CAN_BS2 = CAN_BS2_1tq;
    CAN_InitStructure.CAN_Prescaler = 12;                            // �������� ������ ��������
    CAN_Init(CAN2, &CAN_InitStructure);

    // CAN filter init
    CAN_FilterInitTypeDef CAN_FilterInitStructure;
    CAN_FilterInitStructure.CAN_FilterNumber = 14;
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_16bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO1;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);

    CAN_ITConfig(CAN2, CAN_IT_FMP1, ENABLE);                         // CAN FIFO0 message pending interrupt enable
    CAN2 -> IER |= CAN_IER_FMPIE1;                                   // FIFO Message Pending Interrupt Enable
    CAN_ITConfig(CAN2, CAN_IT_TME, ENABLE);
                                                                     // CAN Error Interrupts �������������� � ���������� CAN2_SCE_IRQHandler
    CAN_ITConfig(CAN2, CAN_IT_EWG, ENABLE);                          // Error warning Interrupt (error counter >= 96)
    CAN_ITConfig(CAN2, CAN_IT_EPV, ENABLE);                          // Error passive Interrupt (error counter > 127)
    CAN_ITConfig(CAN2, CAN_IT_BOF, ENABLE);                          // Bus-off Interrupt (error counter > 255)
    CAN_ITConfig(CAN2, CAN_IT_LEC, ENABLE);                          // Last error code - ��� ������������� ������ ������-��������
    CAN_ITConfig(CAN2, CAN_IT_ERR, ENABLE);                          // ���������� ��� ������������� ������ bxCan

    // NVIC Configuration
    // Enable CAN2 RX0 interrupt IRQ channel
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_EnableIRQ(CAN2_RX1_IRQn);

    NVIC_InitStructure.NVIC_IRQChannel = CAN2_TX_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_EnableIRQ(CAN2_TX_IRQn);

    // Enable CAN2 SCE (Status Change Error) interrupt IRQ channel
    NVIC_InitStructure.NVIC_IRQChannel = CAN2_SCE_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_EnableIRQ(CAN2_SCE_IRQn);
}
//-------------------------------------------------------------------
void CanExt::handler(void)
{
    static uint32_t timer = core.getTick();

    uint32_t BusOff=CAN2->ESR & CAN_ESR_BOFF;

    if (BusOff==0 && this->RxMailBoxReady_f && core.getTick()-lastTransTick > 10) {                       //�������� ��������� �� ������
        for (uint8_t i=0; i<MSG_BUF_SIZE; i++) {
            if (!this->TxMsgBuf[i].Transmitted_f) {
                this->RxMailBoxReady_f=false;
                CAN_Transmit(CAN2,&this->TxMsgBuf[i].TxMsg);
                this->TxMsgBuf[i].Transmitted_f=true;
							lastTransTick=core.getTick();
            }
        }
    }

    if ((core.getTick()-timer)>=1000) {
        timer = core.getTick();

        if (BusOff) {                                                //CAN BUS-OFF. ���� ���������������
            if (this->Recovery!=2) {                                 //������� � ����� �������������
                this->Recovery=2;
                CAN_OperatingModeRequest(CAN2,CAN_OperatingMode_Initialization);
                return;
            }
            else {                                                   //������� � ���������� �����
                this->Recovery=0;
                CAN_OperatingModeRequest(CAN2,CAN_OperatingMode_Normal);
                canExt.RxMailBoxReady_f=true;
                return;
            }
        }
        
        //Uncomment this->FaultCnt++;
        //Uncomment if (this->FaultCnt>30) this->FaultLink_f=true;
        //this->SendMessage(1111, 1, 2,3,4,5,6,7,8);//ToDelete
    }
}
//-------------------------------------------------------------------
void CanExt::ClearFaults(void)
{
    this->FaultCnt=0;
    this->FaultLink_f=false;
}
//-------------------------------------------------------------------
void CanExt::SendMessage(uint32_t AID, uint8_t AD0, uint8_t AD1, uint8_t AD2, uint8_t AD3, uint8_t AD4, uint8_t AD5, uint8_t AD6, uint8_t AD7)
{
    for (uint8_t i=0; i<MSG_BUF_SIZE; i++) {
        if (this->TxMsgBuf[i].Transmitted_f) {
            this->TxMsgBuf[i].Transmitted_f=false;

            this->TxMsgBuf[i].TxMsg.ExtId  =AID;
            this->TxMsgBuf[i].TxMsg.RTR    =CAN_RTR_DATA;
            this->TxMsgBuf[i].TxMsg.IDE    =CAN_ID_EXT;
            this->TxMsgBuf[i].TxMsg.DLC    =8;
            this->TxMsgBuf[i].TxMsg.Data[0]=AD0;
            this->TxMsgBuf[i].TxMsg.Data[1]=AD1;
            this->TxMsgBuf[i].TxMsg.Data[2]=AD2;
            this->TxMsgBuf[i].TxMsg.Data[3]=AD3;
            this->TxMsgBuf[i].TxMsg.Data[4]=AD4;
            this->TxMsgBuf[i].TxMsg.Data[5]=AD5;
            this->TxMsgBuf[i].TxMsg.Data[6]=AD6;
            this->TxMsgBuf[i].TxMsg.Data[7]=AD7;
            break;
        }
    }
}
//-------------------------------------------------------------------
//��������� ��������� RxMessage � ������ MsgBuf
void CanExt::SaveMessage(void)
{
    this->FaultCnt=0;
    for(uint8_t i=0; i<MSG_BUF_SIZE; i++) {
        if (!this->RxMsgBuf[i].Received_f) {
            this->RxMsgBuf[i].RxMsg=this->RxMessage;
            this->RxMsgBuf[i].Received_f=true;
            return;
        }
    }
}
//-------------------------------------------------------------------
//���������� �� ������ CAN
extern "C" void CAN2_RX1_IRQHandler(void)
{
    if (CAN_GetITStatus(CAN2, CAN_IT_FMP0) != RESET) {               // �������� �������� ����
        CAN_Receive(CAN2, CAN_FIFO0, &canExt.RxMessage);
        canExt.SaveMessage();
        CAN_ClearITPendingBit(CAN2,CAN_IT_FMP0);
    }
    if (CAN_GetITStatus(CAN2, CAN_IT_FMP1) != RESET) {               // �������� �������� ����
        CAN_Receive(CAN2, CAN_FIFO1, &canExt.RxMessage);
        canExt.SaveMessage();
        CAN_ClearITPendingBit(CAN2,CAN_IT_FMP1);
    }
}
//-------------------------------------------------------------------
//���������� ��� ������������ ���������� ��������� �����
extern "C" void CAN2_TX_IRQHandler(void)
{
    if (CAN_GetITStatus(CAN2, CAN_IT_TME)==SET) {                    // ���������� ��� ������������ ���������� ��������� �����
        CAN_ClearITPendingBit(CAN2, CAN_IT_TME);

        canExt.RxMailBoxReady_f=true;
    }
}
//-------------------------------------------------------------------
extern "C" void CAN2_SCE_IRQHandler(void)
{
    if (CAN_GetITStatus(CAN2, CAN_IT_ERR)==SET) {                    // ���������� ��� ������������� ������
        CAN_ClearITPendingBit(CAN2, CAN_IT_ERR);

        if (CAN_GetITStatus(CAN2, CAN_IT_EWG)==SET) {                // Error warning Interrupt (������� ������ >= 96)
            CAN_ClearITPendingBit(CAN2, CAN_IT_EWG);

            // ��������� ���� ��� �� ��������� ����������
            canExt.Error.Warning++;
        }
        if (CAN_GetITStatus(CAN2, CAN_IT_EPV)==SET) {                // Error passive Interrupt  (������� ������ > 127)
            CAN_ClearITPendingBit(CAN2, CAN_IT_EPV);

            // ��������� ���� ��� �� ��������� ����������
            canExt.Error.Passive++;
        }
        if (CAN_GetITStatus(CAN2, CAN_IT_BOF)==SET) {                // Bus-off. ���������� ��� ������������ �������� ������ (>255)
            CAN_ClearITPendingBit(CAN2, CAN_IT_BOF);                 // bxCan ������ � ����� Bus-OFF

            // ��������� ���� ��� �� ��������� ����������
            canExt.Error.Busoff++;
        }
        if (CAN_GetITStatus(CAN2, CAN_IT_LEC)==SET) {                // ���������� ��� ������ ������ �������� ���������
            CAN_ClearITPendingBit(CAN2, CAN_IT_LEC);

            // ��������� ���� ��� �� ��������� ����������
            canExt.Error.LastCode = CAN_GetLastErrorCode(CAN1);      // ������� ��� ������
            CAN_ClearFlag(CAN2, CAN_FLAG_LEC);                       // ����� ����� ������
        }
    } 
    else {
        if (CAN_GetITStatus(CAN2, CAN_IT_WKU)==SET) {                // ���������� ��� "�����������" - ����� �� "�������" ������
            CAN_ClearITPendingBit(CAN2, CAN_IT_WKU);

            // ��������� ���� ��� �� ��������� ����������
            CAN_ClearFlag(CAN2, CAN_FLAG_WKU);                       // ����� ����� ������
        }
        if (CAN_GetITStatus(CAN2, CAN_IT_SLK)==SET) {                // ���������� ��� �������� � "������" �����
            CAN_ClearITPendingBit(CAN2, CAN_IT_SLK);

            // ��������� ���� ��� �� ��������� ����������
            CAN_ClearFlag(CAN2, CAN_FLAG_SLAK);                      // ����� ����� ������
        }
    }
}
