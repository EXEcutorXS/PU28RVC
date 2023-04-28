/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_EXT_H
#define __CAN_EXT_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"
#include "stm32f2xx_can.h"

/* Classes ------------------------------------------------------------------*/
class CanExt
{
    #define MSG_BUF_SIZE 50                                          //размер приемного буфера. Не более 255!!! иначе надо везде менять тип
    typedef struct {
        bool Received_f;
        CanRxMsg RxMsg;
    } TReceivedMsg;
    typedef struct {
        bool Transmitted_f;
        CanTxMsg TxMsg;
    } TTransmittedMsg;

    public:
        CanExt(void);
        void initialize(void);
        void handler(void);
        void ClearFaults(void);
        void SendMessage(uint32_t AID, uint8_t AD0, uint8_t AD1, uint8_t AD2, uint8_t AD3, uint8_t AD4, uint8_t AD5, uint8_t AD6, uint8_t AD7);
        void SaveMessage(void);                                      //Сохранить сообщение RxMessage в буфере MsgBuf

        int ID;                                                      //Идентификатор сообщения
        bool FaultLink_f;                                            //нет связи с подогревателем

        bool RxMailBoxReady_f;                                       //Исходящий почтовый ящик готов к отправке очередного сообщения
        CanTxMsg TxMessage;
        CanRxMsg RxMessage;
        TReceivedMsg RxMsgBuf[MSG_BUF_SIZE];                         //Буфер принимаемых сообщений
        TTransmittedMsg TxMsgBuf[MSG_BUF_SIZE];                      //Буфер передаваемых сообщений
        struct {                                                     //Ошибки на CAN-шине
            uint16_t Warning,Passive,Busoff,LastCode;
        } Error;
    private:
        uint8_t FaultCnt;
        uint8_t Recovery;                                            //для восстановления CAN-шины в случае перехода в состояние BUS-OFF
        uint32_t lastTransTick;
};
extern CanExt canExt;

#endif /* __CAN_EXT_H */
