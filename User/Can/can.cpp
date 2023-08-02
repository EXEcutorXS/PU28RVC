/******************************************************************************
* ООО Теплостар
* Самара
* 
* Программисты: Кубышкин К.А.
* 
* __.09.2020
* Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "can.h"
#include "core.h"
#include "rvc.h"

Can can;

//-------------------------------------------------------------------
Can::Can(void)
{
		core.ClassInit(this,sizeof(this));
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
void ConfigFilter(uint32_t DGN,uint8_t num)
{
	can_filter_parameter_struct canfilterSetup;
	uint32_t FilterID;
	uint16_t MaskMSB = 0xFFF;
	uint16_t MaskLSB = 0x1F<<11;
		
	FilterID = DGN << (8 + 3);//DGN shifted by 8 bit+Mask in register shifted by 3 bit
		
    can_struct_para_init(CAN_FILTER_STRUCT, &canfilterSetup);
	
    canfilterSetup.filter_fifo_number = CAN_FIFO1;
    canfilterSetup.filter_bits = CAN_FILTERBITS_32BIT;
    canfilterSetup.filter_mode = CAN_FILTERMODE_MASK;
	canfilterSetup.filter_mask_high = MaskMSB;
	canfilterSetup.filter_mask_low = MaskLSB;
	canfilterSetup.filter_list_high = (FilterID>>16)&0xFFFF;
	canfilterSetup.filter_list_low = FilterID&0xFFFF;
    canfilterSetup.filter_number = num;
    canfilterSetup.filter_enable = ENABLE;
    can_filter_init(&canfilterSetup);
}

void Can::initialize(void)
{
    #ifdef IS_MINSK
    /* baudrate 500Kbps */
    uint16_t bdrt = 500;
    #else
    /* baudrate 250Kbps */
    uint16_t bdrt = 250;
    #endif
    
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_PMU);
    
    can_deinit(CAN0);
    
    // настройка входа/выхода CAN
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
    ///gpio_pin_remap_config(GPIO_CAN_FULL_REMAP,ENABLE);
    // настройка выхода STB
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
    gpio_bit_reset(GPIOB, GPIO_PIN_2);
    ///*
    can_parameter_struct canInitStruct;
    can_struct_para_init(CAN_INIT_STRUCT, &canInitStruct);
    //Configure CAN
    canInitStruct.working_mode = CAN_NORMAL_MODE;                              
    canInitStruct.resync_jump_width = CAN_BT_SJW_1TQ;                          
    canInitStruct.time_segment_1 = CAN_BT_BS1_13TQ;                            
    canInitStruct.time_segment_2 = CAN_BT_BS2_2TQ;                             
    canInitStruct.time_triggered = DISABLE;                                    
    canInitStruct.auto_bus_off_recovery = DISABLE;                             
    canInitStruct.auto_wake_up = DISABLE;                                      
    canInitStruct.no_auto_retrans = ENABLE;                                    
    canInitStruct.rec_fifo_overwrite = DISABLE;                                
    canInitStruct.trans_fifo_order = DISABLE;                                  
    canInitStruct.prescaler = SystemCoreClock/32000/bdrt;                                   
    
    can_init(CAN0, &canInitStruct);
    
        // CAN filter init
	ConfigFilter(0x1FFF6,0); //Heater command
	ConfigFilter(0x1FFE3,1); //Furnace command
	ConfigFilter(0x1FEF9,2); //Thermostat control
	ConfigFilter(0x1FF9C,3); //Ambient temp
	ConfigFilter(0x1FEF5,4); //Thermostat schedule command
	ConfigFilter(0x1EF65,5); //Proprietary packet
	ConfigFilter(0x1FE96,6); //Pump command
	ConfigFilter(0x1FFFF,7); //Pump command

    can_interrupt_enable(CAN0, CAN_INT_RFNE1); // |CAN_INT_WERR|CAN_INT_PERR|CAN_INT_ERRN |CAN_INT_ERR  CAN_INT_RFF1 CAN_INT_RFNE1
    //can_interrupt_enable(CAN0, CAN_INT_WERR | CAN_INT_PERR | CAN_INT_ERRN | CAN_INT_ERR); // |CAN_INT_WERR|CAN_INT_PERR|CAN_INT_ERRN |CAN_INT_ERR  
            // CAN_INT_RFF0 | CAN_INT_RFF1 | CAN_INT_RFO0 | CAN_INT_RFO1 | 
    can_interrupt_enable(CAN0, CAN_INT_RFNE1); 
    nvic_irq_enable(CAN0_RX1_IRQn, 0, 1);
    //nvic_irq_enable(USBD_LP_CAN0_RX0_IRQn, 0, 1);
    //nvic_irq_enable(USBD_HP_CAN0_TX_IRQn, 0, 1);
}
//-------------------------------------------------------------------
void Can::handler(void)
{
    static uint32_t timer = core.getTick();
    uint8_t TransmitMailBox=0;

    if (core.getTick()-lastTransTick > 10) {                       //Отправка сообщений из буфера
        for (uint8_t i=0; i<MSG_BUF_SIZE; i++) {
            if (!this->TxMsgBuf[i].Transmitted_f) {
                this->RxMailBoxReady_f=false;
                TransmitMailBox = can_message_transmit(CAN0, &this->TxMsgBuf[i].TxMsg);
                this->TxMsgBuf[i].Transmitted_f=true;
							lastTransTick=core.getTick();
            }
        }
    }
    
    if (TransmitMailBox == 3) {                                                //CAN BUS-OFF. Надо восстанавливать
        initialize();
    }
}
//-------------------------------------------------------------------
void Can::ClearFaults(void)
{
    this->FaultCnt=0;
    this->FaultLink_f=false;
}
//-------------------------------------------------------------------
void Can::SendMessage(uint32_t AID, uint8_t AD0, uint8_t AD1, uint8_t AD2, uint8_t AD3, uint8_t AD4, uint8_t AD5, uint8_t AD6, uint8_t AD7)
{
    for (uint8_t i=0; i<MSG_BUF_SIZE; i++) {
        if (this->TxMsgBuf[i].Transmitted_f) {
            this->TxMsgBuf[i].Transmitted_f = false;
            this->TxMsgBuf[i].TxMsg.tx_sfid  = 0;
            this->TxMsgBuf[i].TxMsg.tx_efid  = AID;
            this->TxMsgBuf[i].TxMsg.tx_ft = CAN_FT_DATA;
            this->TxMsgBuf[i].TxMsg.tx_ff = CAN_FF_EXTENDED;
            this->TxMsgBuf[i].TxMsg.tx_dlen    = 8;
            this->TxMsgBuf[i].TxMsg.tx_data[0]=AD0;
            this->TxMsgBuf[i].TxMsg.tx_data[1]=AD1;
            this->TxMsgBuf[i].TxMsg.tx_data[2]=AD2;
            this->TxMsgBuf[i].TxMsg.tx_data[3]=AD3;
            this->TxMsgBuf[i].TxMsg.tx_data[4]=AD4;
            this->TxMsgBuf[i].TxMsg.tx_data[5]=AD5;
            this->TxMsgBuf[i].TxMsg.tx_data[6]=AD6;
            this->TxMsgBuf[i].TxMsg.tx_data[7]=AD7;
            break;
        }
    }
}
//-------------------------------------------------------------------
//Сохранить сообщение RxMessage в буфере MsgBuf
void Can::SaveMessage(void)
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
//Прерывание по приему CAN
extern "C" void CAN0_RX1_IRQHandler(void)
{
    if (can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFL0) == SET) {                   // Прерывание получения пакета в буфер FIFO 0
                                                                      // Флаг сбрасывается автоматически после прочтения последнего сообщения
        can_message_receive(CAN0, CAN_FIFO0, &can.RxMessage);                  // Получим сообщение
        //if (((can.RxMessage.ExtId >> 3)& 0x7F) < 126)
        //    address = ((can.RxMessage.ExtId & 0x3FF) << 10);
        //parsingMessage(can.RxMessage.ExtId >> 20);
        can.SaveMessage();
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFL0);
    }
    if (can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFL1) == SET) {                   // Прерывание получения пакета в буфер FIFO 1
                                                                      // Флаг сбрасывается автоматически после прочтения последнего сообщения
        can_message_receive(CAN0, CAN_FIFO1, &can.RxMessage);                  // Получим сообщение
        //if (((can.RxMessage.ExtId >> 3)& 0x7F) < 126)
        //    address = ((can.RxMessage.ExtId & 0x3FF) << 10);
        //parsingMessage(can.RxMessage.ExtId >> 20);
        can.SaveMessage();
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFL1);
    }
}
