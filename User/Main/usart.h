
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
        void processReceivedData(void);                 // прием данных по последовательному каналу от пульта, вызов ставится в main.
        void receiveIntHandler(uint8_t AByte);          // обработчик прерывания приема байтов
        bool startTransmission(void);                   // инициализация отправки пакета
        void transmitNextByte(void);                    // отправка следующего байта пакета
        void processTimeOut(void);                      // счетчик таймаута. вызов ставится в обработчик прерывание
        void send(uint8_t* buf, uint32_t len);
        void usartIrqHandler(void);

        #define DATA_FIELD_MAX_LENGTH 245   //максимальный размер поля данных исходящего пакета (90)
        #define PACKET_IN_MAX_LENGTH 254    //максимальный размер входящего пакета без заголовка (60)
        #define PACKET_OUT_MAX_LENGTH 254   //максимальный размер исходящего пакета с заголовком и без контрольной суммы (99)
        uint8_t packetIn[PACKET_IN_MAX_LENGTH];         // принятый пакет
        uint8_t packetOut[PACKET_OUT_MAX_LENGTH];       // массив для формирования пакета отправки
        uint8_t sendedCmd1, sendedCmd2;                 // переданная команда
        bool isProcessPacket;                           // пакет принят. можно обрабатывать
        uint8_t  linkCnt;
        uint8_t verProtocol;
        uint32_t baudrate;
        
        uint8_t commandProtocol1;
        uint8_t answerTimeOut;                          // для отсчета "нет связи" при приеме ответа на команду
        uint16_t badCrcCounter;
        
        bool isTransmission;                            // в процессе отправки
		
		uint32_t lastReceivedTick;						//Тик последнего принятого сообщения
		
		
		uint32_t faultedCommandCounter;         //Счётчик непринятых комманд. Если принятое состояние кнопок не соответствует недавно переданному.
		uint32_t lastCommandSendTick;
		uint8_t lastSendedState; //0 бит - Подогреватель, 1 бит - ТЭН, 2 - разбор воды. 3 - ручной вент. 5 - помпа

    private:
        static const int PACKET_HEADER = 170;           //заголовок пакета
        static const int BUFFER_SIZE = 1024;            //размер буферного массива

        static const int SD_NO_SENDING = 0;             //не передавать информацию
        static const int SD_EXTENDED = 1;               //передавать расширенный набор параметров (команда 0 из подогревателя).
        static const int SD_REDUCE = 2;                 //передавать сокращенный набор параметров (команда 2 из подогревателя)
        uint8_t buffer[BUFFER_SIZE],                   // буферный массив для приема байтов
                bufferCursorW,                          // курсор для записи байтов в буфер
                bufferCursorR,                          // курсор для чтения байтов из буфер
                packetLength,                           // размер поля данных принимаемого пакета
                packetCounter,                          // счетчик принятых байт в пакете
                packetTimeOut,                          // для отсчета "нет связи" внутри принимаемого пакета
                transmitCounter,                        // счетчик байтов для передачи пакета
                bytesToTransmit,                        // сколько байт будет передано в текущем пакете
                sendDataState;                          // 0 - не передавать информацию
                                                        // 1 - передавать расширенный набор параметров (команда 0 из подогревателя).
                                                        // 2 - передавать сокращенный набор параметров (команда 2 из подогревателя)

        uint16_t crc;                                   // для подсчета контрольной суммы

        bool isWaitHeaderByte,                          // ожидание заголовка пакета
                isLinkError;                            // нет связи с внешним устройством
};
extern Usart usart;

#endif /* __USART_H */
