//
//    ћодуль св€зи блока электронного с пультом (компьютером, стендом...)
//    по последовательному каналу св€зи USART
//    ¬ерси€ протокола обмена 3.1

/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "core.h"
#include "string.h"
#include "hcu.h"

Usart usart;
/* Defines ------------------------------------------------------------------*/
#define FALSE  0
#define TRUE   1

//-----------------------------------------------------
Usart::Usart(void)
{
    linkCnt = 1;
    isWaitHeaderByte=1;
    this->verProtocol = 3;     // 3
    this->baudrate = 9600;     // 4800-9600-19200-38400-57600-76800
    commandProtocol1 = 15;
}
//-----------------------------------------------------
void Usart::initialize(void)
{
	memset(this,0,sizeof(Usart));
	//Initialising basic values
	linkCnt = 1;
    isWaitHeaderByte=1;
    this->verProtocol = 3;     // 3
    this->baudrate = 9600;     // 4800-9600-19200-38400-57600-76800
    commandProtocol1 = 15;
	hcu.reinitialisationCounter++;
	
	//Initialising periph
	
    // Enable the GPIO Clock
    rcu_periph_clock_enable(RCU_GPIOA);
    // Enable the USART Clock
    rcu_periph_clock_enable(RCU_USART0);
    rcu_periph_clock_enable(RCU_AF);
    // Configure the USART pin
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);    
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    
    // Configure the USART
    usart_deinit(USART0);
    usart_disable(USART0);
    usart_baudrate_set(USART0, baudrate);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_data_first_config(USART0, USART_MSBF_LSB);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_dma_transmit_config(USART0, USART_DENT_ENABLE);
    usart_dma_receive_config(USART0, USART_DENR_ENABLE);
    usart_interrupt_enable(USART0, USART_INT_RBNE); // USART_INT_TBE // USART_INT_RBNE // USART_INT_TC
    nvic_irq_enable(USART0_IRQn, 1, 1);
    usart_enable(USART0);
}
//-----------------------------------------------------
void Usart::handler(void)
{
    static uint32_t timer = core.getTick();
    
    if ((core.getTick() - timer) >= USART_TIMEOUT_PERIOD) {
        timer = core.getTick();
        
        this->processTimeOut();
    }
	
	//Anti glitch
	if (core.getTick()-lastReceivedTick>10000)
	{
		lastReceivedTick = core.getTick();
		initialize();
	}
}
//-----------------------------------------------------
void Usart::changeBaudrate(uint32_t baudrate)
{
    usart_disable(USART0);       
    this->baudrate = baudrate;
    usart_baudrate_set(USART0, this->baudrate);
    usart_enable(USART0);
}
//-----------------------------------------------------
//прием данных по последовательному каналу от пульта
//вызов ставитс€ в main.
void Usart::processReceivedData(void)
{
    uint8_t j, bt, AByte;
    uint16_t CRCinPacket;

    if (this->isProcessPacket) return;                               //ѕредыдущий пакет не обработан

    while(this->bufferCursorR!=this->bufferCursorW) {
        AByte=this->buffer[this->bufferCursorR];
        this->bufferCursorR++;
        if (this->bufferCursorR>=this->BUFFER_SIZE) this->bufferCursorR=0;

        if (verProtocol == 1){
            this->packetIn[this->packetCounter]=AByte;
            this->packetCounter++;
            if (this->packetCounter == 1){
                this->crc = 0;
            }
            if (this->packetCounter < 4){
                this->crc += AByte;
            }
            else{
                if ((this->crc & 0xFF) == AByte){                                //контрольна€ сумма совпадает.
                    this->isProcessPacket=1;                            //обрабатываем пакет
                    //usart.linkCnt=0;
                }
                else badCrcCounter++;
                this->packetCounter = 0;
                this->crc = 0;
            }
            
        }
        else if (verProtocol == 2){
            
        }
        else if (verProtocol == 3){
            if (!this->isWaitHeaderByte) {                               //заголовочный байт прин€т. принимаем пакет
                this->packetIn[this->packetCounter]=AByte;
                if (this->packetCounter==1)                              //получили размер пол€ данных
                    this->packetLength=5+AByte;                          //посчитали размер принимаемого пакета.
                if (this->packetCounter<=this->packetLength-2) {         //считаем контрольную сумму на лету
                    for(j=1;j<=8;j++) {
                        bt=this->crc & 1;
                        this->crc=this->crc >> 1;
                        if ((AByte & 1)!=bt) this->crc=this->crc^0xA001;
                        AByte=AByte >> 1;
                    }
                }
                if (this->packetCounter>=this->packetLength) {
                    CRCinPacket=(this->packetIn[this->packetCounter-1]<<8)+this->packetIn[this->packetCounter];
                    if (this->crc==CRCinPacket) {                        //контрольна€ сумма совпадает.
                        this->isProcessPacket=1;                         //обрабатываем пакет
                        usart.linkCnt=0;
                    }
                    else badCrcCounter++;
                    this->isWaitHeaderByte=1;
                    break;
                }
                this->packetCounter++;
            }
            else {                                                       //заголовка не было
                if (AByte==this->PACKET_HEADER) {                        //прин€т заголовок пакета
                    this->isWaitHeaderByte=0;
                    this->packetCounter=0;
                    this->packetLength=PACKET_IN_MAX_LENGTH;       //пока не знаем размер пакета. принимаем его максимальным.
                    this->crc=0xFFFF;
                    for(j=1;j<=8;j++) {
                        bt=this->crc & 1;
                        this->crc=this->crc >> 1;
                        if ((AByte & 1)!=bt) this->crc=this->crc^0xA001;
                        AByte=AByte >> 1;
                    }
                }
            }
        }
    }
}

//-----------------------------------------------------
//обработчик прерывани€ приема байтов
void Usart::receiveIntHandler(uint8_t AByte)
{
    this->buffer[this->bufferCursorW]=AByte;
    this->bufferCursorW++;
    if (this->bufferCursorW>=this->BUFFER_SIZE) {
        this->bufferCursorW=0;
    }
    this->packetTimeOut=0;
}

//-----------------------------------------------------
//инициализаци€ отправки пакета
bool Usart::startTransmission(void)
{
    uint8_t i,j;
    uint16_t CRCinPacket;
    uint8_t nextByte,bt;

    if (isTransmission) return false;
    if (verProtocol == 1){
        isLinkError = 0;
        answerTimeOut = 0;                                       //инициализируем счетчик дл€ определени€ отсутстви€ св€зи
        isTransmission = 1;                                          //отправл€ем пакет
        bytesToTransmit = 1;                                         //сколько байт будем передавать
        transmitCounter = 0;
        usart_interrupt_enable(USART0, USART_INT_TC);            //разрешаем прерывание перед отправкой пакета
        usart_data_transmit(USART0, commandProtocol1);         //передаем первый байт пакета
        
    }
    else if (verProtocol == 2){
        
    }
    else if(verProtocol == 3){
        if (packetOut[1]&1) {                                      //на команду нужен ответ
            sendedCmd1 = packetOut[3];                         //запоминаем команду1
            sendedCmd2 = packetOut[4];                         //запоминаем команду2
            isLinkError = 0;
            answerTimeOut = 0;                                       //инициализируем счетчик дл€ определени€ отсутстви€ св€зи
        }
        isTransmission = 1;                                          //отправл€ем пакет

        if (packetOut[2]>DATA_FIELD_MAX_LENGTH)              //проверка длины пакета
            packetOut[2]=DATA_FIELD_MAX_LENGTH;

        packetOut[0]=PACKET_HEADER;                          //заголовок пакета
                                                                         //подсчет контрольной суммы
        CRCinPacket=0xFFFF;
        for(i=0;i<5+packetOut[2];i++) {
            nextByte=packetOut[i];
            for(j=1;j<=8;j++) {
                bt=CRCinPacket&1;
                CRCinPacket=CRCinPacket>>1;
                if ((nextByte&1)!=bt) CRCinPacket=CRCinPacket^0xA001;
                nextByte=nextByte>>1;
            }
        }
        packetOut[5+packetOut[2]]=CRCinPacket>>8;
        packetOut[6+packetOut[2]]=CRCinPacket;
        bytesToTransmit=6+packetOut[2];                      //сколько байт будем передавать
        transmitCounter=0;
        usart_interrupt_enable(USART0, USART_INT_TC);            //разрешаем прерывание перед отправкой пакета
        usart_data_transmit(USART0, packetOut[0]);             //передаем первый байт пакета
    }
    return true;
}
//-----------------------------------------------------
//отправка следующего байта пакета
void Usart::transmitNextByte(void)
{
    if (this->transmitCounter<this->bytesToTransmit) {
        this->transmitCounter++;
        if (verProtocol == 1){
            usart_data_transmit(USART0,this->commandProtocol1);
        }
        else if (verProtocol == 2){
            
        }
        else if (verProtocol == 3){
            usart_data_transmit(USART0,this->packetOut[this->transmitCounter]);
        }
    }
    else {
        usart_interrupt_disable(USART0, USART_INT_TC);       //запрещаем прерывание после отправки пакета
        this->isTransmission=0;                                      //отправка пакета завершена
    }
}

//-----------------------------------------------------
//счетчик таймаута. вызов ставитс€ в обработчик прерывани€ “аймера0
void Usart::processTimeOut(void)
{
    if (this->isWaitHeaderByte==0) {
        if (this->packetTimeOut<250) this->packetTimeOut++;
        if (this->packetTimeOut==25) {
            this->isWaitHeaderByte=1;
        }
    }
    if (this->answerTimeOut<250) this->answerTimeOut++;
    if (this->answerTimeOut==250) {                                  //250 - 250мс ждем ответа. если прошло, то нет св€зи
        if ((this->sendedCmd1>0)||(this->sendedCmd2>0)){
            this->isLinkError=1;//нет св€зи
        }
    }

}
//-----------------------------------------------------
extern "C" void USART0_IRQHandler(void)
{
    uint8_t rxByte;
    bool isError = true;
    
    if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE) == SET) {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE); // USART_INT_FLAG_TC
        rxByte = usart_data_receive(USART0);
        usart.receiveIntHandler(rxByte);
        isError = false;
    }
    if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_TC) == SET){
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_TC); 
        usart.transmitNextByte();
        isError = false;
    }
    if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_TBE) == SET){ 
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_TBE); 
        isError = false;
    }
    if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_ERR_ORERR) == SET){
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_ORERR); 
    }
    else if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_ERR_NERR) == SET){
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_NERR); 
    }
    else if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_ERR_FERR) == SET){
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_FERR); 
    }
    else if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE_ORERR) == SET){
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE_ORERR); 
    }
    else if (isError) {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_ORERR);
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_NERR); 
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_FERR); 
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE_ORERR); 
    }
}
//-----------------------------------------------------
