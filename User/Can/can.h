/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H
#define __CAN_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"
#include "gd32f30x_can.h"

/* Classes ------------------------------------------------------------------*/
class Can
{
    can_parameter_struct can_parameter;
    
    #define MSG_BUF_SIZE 50                                          //������ ��������� ������. �� ����� 255!!! ����� ���� ����� ������ ���
    typedef struct {
        bool Received_f;
        can_receive_message_struct RxMsg;
    } TReceivedMsg;
    typedef struct {
        bool Transmitted_f;
        can_trasnmit_message_struct TxMsg;
    } TTransmittedMsg;

    public:
        Can(void);
        void initialize(void);
        void handler(void);
        void ClearFaults(void);
        void SendMessage(uint32_t AID, uint8_t AD0, uint8_t AD1, uint8_t AD2, uint8_t AD3, uint8_t AD4, uint8_t AD5, uint8_t AD6, uint8_t AD7);
        void SaveMessage(void);                                      //��������� ��������� RxMessage � ������ MsgBuf
        
        int ID;                                                      //������������� ���������
        bool FaultLink_f;                                            //��� ����� � ��������������
        
        bool RxMailBoxReady_f;                                       //��������� �������� ���� ����� � �������� ���������� ���������
        can_trasnmit_message_struct TxMessage;
        can_receive_message_struct RxMessage;
        TReceivedMsg RxMsgBuf[MSG_BUF_SIZE];                         //����� ����������� ���������
        TTransmittedMsg TxMsgBuf[MSG_BUF_SIZE];                      //����� ������������ ���������
        struct {                                                     //������ �� CAN-����
            uint16_t Warning,Passive,Busoff,LastCode;
        } Error;
    private:
        uint8_t FaultCnt;
        uint8_t Recovery;                                            //��� �������������� CAN-���� � ������ �������� � ��������� BUS-OFF
        uint32_t lastTransTick;
};
extern Can can;

#endif /* __CAN_H */
