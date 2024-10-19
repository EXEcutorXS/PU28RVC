
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H
#define __USART_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Classes ------------------------------------------------------------------*/
class Usart
{
    public:
        Usart(void);
        void initialize(void);
        void handler(void);
        void changeBaudrate(uint32_t baudrate);
        void processReceivedData(void);                 // ����� ������ �� ����������������� ������ �� ������, ����� �������� � main.
        void receiveIntHandler(uint8_t AByte);          // ���������� ���������� ������ ������
        bool startTransmission(void);                   // ������������� �������� ������
        void transmitNextByte(void);                    // �������� ���������� ����� ������
        void processTimeOut(void);                      // ������� ��������. ����� �������� � ���������� ����������
        void send(uint8_t* buf, uint32_t len);
        void usartIrqHandler(void);

        #define DATA_FIELD_MAX_LENGTH 245   //������������ ������ ���� ������ ���������� ������ (90)
        #define PACKET_IN_MAX_LENGTH 254    //������������ ������ ��������� ������ ��� ��������� (60)
        #define PACKET_OUT_MAX_LENGTH 254   //������������ ������ ���������� ������ � ���������� � ��� ����������� ����� (99)
        uint8_t packetIn[PACKET_IN_MAX_LENGTH];         // �������� �����
        uint8_t packetOut[PACKET_OUT_MAX_LENGTH];       // ������ ��� ������������ ������ ��������
        uint8_t sendedCmd1, sendedCmd2;                 // ���������� �������
        bool isProcessPacket;                           // ����� ������. ����� ������������
        uint8_t  linkCnt;
        uint8_t verProtocol;
        uint32_t baudrate;
        
        uint8_t commandProtocol1;
        uint8_t answerTimeOut;                          // ��� ������� "��� �����" ��� ������ ������ �� �������
        uint16_t badCrcCounter;
        
        bool isTransmission;                            // � �������� ��������
		
		uint32_t lastReceivedTick;						//��� ���������� ��������� ���������
		
		
		uint32_t faultedCommandCounter;         //������� ���������� �������. ���� �������� ��������� ������ �� ������������� ������� �����������.
		uint32_t lastCommandSendTick;
		uint8_t lastSendedState; //0 ��� - �������������, 1 ��� - ���, 2 - ������ ����. 3 - ������ ����. 5 - �����

    private:
        static const int PACKET_HEADER = 170;           //��������� ������
        static const int BUFFER_SIZE = 1024;            //������ ��������� �������

        static const int SD_NO_SENDING = 0;             //�� ���������� ����������
        static const int SD_EXTENDED = 1;               //���������� ����������� ����� ���������� (������� 0 �� �������������).
        static const int SD_REDUCE = 2;                 //���������� ����������� ����� ���������� (������� 2 �� �������������)
        uint8_t buffer[BUFFER_SIZE],                   // �������� ������ ��� ������ ������
                bufferCursorW,                          // ������ ��� ������ ������ � �����
                bufferCursorR,                          // ������ ��� ������ ������ �� �����
                packetLength,                           // ������ ���� ������ ������������ ������
                packetCounter,                          // ������� �������� ���� � ������
                packetTimeOut,                          // ��� ������� "��� �����" ������ ������������ ������
                transmitCounter,                        // ������� ������ ��� �������� ������
                bytesToTransmit,                        // ������� ���� ����� �������� � ������� ������
                sendDataState;                          // 0 - �� ���������� ����������
                                                        // 1 - ���������� ����������� ����� ���������� (������� 0 �� �������������).
                                                        // 2 - ���������� ����������� ����� ���������� (������� 2 �� �������������)

        uint16_t crc;                                   // ��� �������� ����������� �����

        bool isWaitHeaderByte,                          // �������� ��������� ������
                isLinkError;                            // ��� ����� � ������� �����������
};
extern Usart usart;

#endif /* __USART_H */
