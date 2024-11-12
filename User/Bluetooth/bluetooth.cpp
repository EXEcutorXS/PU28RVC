/******************************************************************************
* ООО Теплостар
* Самара
* 
* Программисты: Клюев А.А.
* 
* 18.01.2023
* Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "bluetooth.h"
#include "gd32f30x_spi.h"
#include "core.h"

Bluetooth bluetooth;
//-----------------------------------------------------
Bluetooth::Bluetooth(void)
{
    core.ClassInit(this,sizeof(this));
}
//-----------------------------------------------------
void Bluetooth::initSpi(void)
{
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_SPI1);
    
    // Configure the GPIO_SCLK pin
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);          //GPIO_MODE_AF_PP
    // Configure the GPIO_MOSI pin
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_15);    // GPIO_MODE_AF_PP
    // SPI1 GPIOB config: MISO/PB14
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14);         // GPIO_MODE_IN_FLOATING
    
    // Configure the GPIO_RESET pin
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);

    /* SPI1 GPIOA config: CS/PA8 */
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8 );

    /* bluenrg132 reset */
    gpio_bit_set(GPIOB,  GPIO_PIN_12);
    
}
//-----------------------------------------------------
void Bluetooth::handler(void)
{
    static uint32_t timer = 0;
    
    if ((core.getTick() - timer) >= 1 || byte_out[0] > 0){
        timer = core.getTick();
        ///__disable_irq();
        CS_DOWN;
        MOSI_UP;
        delay();
        for (uint8_t p=0; p<20; p++){
            for (uint8_t i=0; i<8; i++){ 
                CLK_UP;
                if ((byte_out[p] << i) & 0x80) MOSI_UP;
                else MOSI_DOWN;
                delay();
                CLK_DOWN;
                byte_in[p] = byte_in[p] << 1;
                byte_in[p] |= MISO_STATE;
                delay();
            }
        }
        CS_UP;
        MOSI_DOWN;
        ///__enable_irq();
        
        for (uint8_t p=0; p<20; p++){
            byte_out[p] = 0;
        }
        byte_out[1] = 0xAA;
        byte_out[2] = 0x55;
        readBleData(byte_in);
    }
}
//-----------------------------------------------------
void Bluetooth::initialize(char * dev_name)
{
    this->initSpi();
    gpio_bit_reset(GPIOB,  GPIO_PIN_12);
    uint32_t timer = core.getTick();
    while((core.getTick() - timer) < 5);
    gpio_bit_set(GPIOB,  GPIO_PIN_12);
}
//-----------------------------------------------------
void Bluetooth::sendArray(uint8_t* buf)
{
    for (uint8_t i=0; i<20; i++){
        byte_out[i] = buf[i];
    }
}
//-----------------------------------------------------
void Bluetooth::readBleData(uint8_t *buf)
{
    if (buf[0] == 0xFF){
        isConnected = false;
    }
    else{
        isConnected = true;
        if (buf[0] > 0){
            for (int i=0; i<20; i++){
                this->bufRx[i] = buf[i];
            }
            bluetooth.isCommand = TRUE;
        }
    }
}
//-----------------------------------------------------
void Bluetooth::delay(void)
{
    volatile uint32_t i = 20;    // 5 20
    while(i--);
}
//-----------------------------------------------------
void Bluetooth::setTimer(uint32_t value)
{
    this->tickTimer = core.getTick() + value;
}
//-----------------------------------------------------
uint32_t Bluetooth::getTimer(void)
{
    int32_t i;
    
    i = this->tickTimer - core.getTick();
    if (i < 0) i = 0;
    return i;
}
//-----------------------------------------------------
void Bluetooth::resetTimer(void)
{
    this->tickTimer = core.getTick();
}
//-----------------------------------------------------
extern "C" void EXTI10_15_IRQHandler(void)
{ 
    if (exti_interrupt_flag_get(EXTI_10) != RESET)
    {
        exti_interrupt_flag_clear(EXTI_10);
        //HCI_Isr();
    }
}
//-----------------------------------------------------
