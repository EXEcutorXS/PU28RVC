/******************************************************************************
* ООО Адверс
* Самара
* 
* Программисты: Клюев А.А.
* 
* 22.06.2022
* Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "sensor.h"
#include "core.h"
#include "sleep.h"

#include <math.h>

Sensor sensor;
/* Defines ------------------------------------------------------------------*/
#define sensor_RST_OFF	            gpio_bit_set(GPIOB, GPIO_PIN_8);
#define sensor_RST_ON	            gpio_bit_reset(GPIOB, GPIO_PIN_8);
#define sensor_OWN_ADDRESS (0x38<<1)

//-----------------------------------------------------
void Sensor::initialize(void)
{
    rcu_periph_clock_enable(RCU_AF);
    // Тактирование GPIO
    rcu_periph_clock_enable(RCU_GPIOB);
    // Включаю тактирование I2C
    rcu_periph_clock_enable(RCU_I2C0);
	// Настройка ног PB6, PB7
    gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);
	// Отключаю I2C
	i2c_deinit(I2C0);
	// Частота шины I2C = 100 kHz
	// Настраиваю тайминги
    i2c_clock_config(I2C0, 100000, I2C_DTCY_2);
    i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, sensor_OWN_ADDRESS>>1);
	// Включаю I2C
	i2c_enable(I2C0);
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);
    
    // Configure the Reset pin
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
    //sensor_RST_OFF;
    
    // Configure the Int pin
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
    
    sensor_RST_ON;
    core.delayMs(2);
    sensor_RST_OFF;
    this->initButton();
    
    // Connect Button EXTI Line to GPIO Pin
    // Configure User EXTI line to generate an interrupt on rising & falling edges
    nvic_irq_enable(EXTI5_9_IRQn, 3, 0);
    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_5);
    exti_init(EXTI_5, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
    exti_interrupt_flag_clear(EXTI_5);
}
//-----------------------------------------------------
uint8_t Sensor::handler(void)
{
    uint8_t result = 0;
    static uint32_t timer = 0;
    uint32_t pause = 50;
    
    if (sleep.isLowPower == true) pause = 5;
    if ((core.getTick()-timer) > pause){
        timer = core.getTick();
        if (isPress == true){
            isPress = false;
            testProg();                          // опрос сенсорного экрана
            result = 1;
        }
    }
	if (status==255) //DEBUG Костыль для выкючения зоны по RV-C
		status=0;
    return result;
}
//-----------------------------------------------------
void Sensor::initButton(void)
{
    rcu_periph_clock_enable(RCU_AF);
    // Тактирование GPIO
    rcu_periph_clock_enable(RCU_GPIOA);
    // Configure the pin
    gpio_init(GPIOA, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
    // Connect Button EXTI Line to GPIO Pin
    // Configure User EXTI line to generate an interrupt on rising & falling edges
    nvic_irq_enable(EXTI0_IRQn, 3, 0);
    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_0);
    exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
    exti_interrupt_flag_clear(EXTI_0);
}
//-----------------------------------------------------
uint8_t Sensor::read(uint8_t* reg, uint8_t address, uint8_t len)
{
    uint32_t timer = core.getTick();
    uint8_t res = 0;
    
    while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)){
        if ((core.getTick() - timer) > 1){
            res = 1;
            i2c_stop_on_bus(I2C0);
            return res;
        }
    }
    
    i2c_start_on_bus(I2C0);
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)){
        if ((core.getTick() - timer) > 1){
            res = 2;
            i2c_stop_on_bus(I2C0);
            return res;
        }
    }
    i2c_master_addressing(I2C0, sensor_OWN_ADDRESS, I2C_TRANSMITTER);
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)){
        if ((core.getTick() - timer) > 1) {
            res = 3;
            i2c_stop_on_bus(I2C0);
            return res;
        }
    }
    
    //i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    
    //i2c_flag_clear(I2C0, I2C_FLAG_PECERR);
    //i2c_flag_clear(I2C0, I2C_FLAG_OUERR);
    //i2c_flag_clear(I2C0, I2C_FLAG_AERR);
    //i2c_flag_clear(I2C0, I2C_FLAG_LOSTARB);
    //i2c_flag_clear(I2C0, I2C_FLAG_BERR);
    //i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    volatile uint32_t stat0 = I2C_STAT0(I2C0);
    volatile uint32_t stat1 = I2C_STAT1(I2C0);
    //I2C_STAT0(I2C0) &= ~(I2C_FLAG_ADDSEND);
    //volatile uint32_t stat2 = I2C_STAT0(I2C0);
    //volatile uint32_t stat3 = I2C_STAT1(I2C0);
    
    
    while(SET != i2c_flag_get(I2C0, I2C_FLAG_TBE)){
        if ((core.getTick() - timer) > 1) {
            res = 4;
            i2c_stop_on_bus(I2C0);
            return res;
        }
    }
    
    i2c_enable(I2C0);
    
    i2c_data_transmit(I2C0, address); 
    while(!i2c_flag_get(I2C0, I2C_FLAG_BTC)){
        if ((core.getTick() - timer) > 1) {
            res = 5;
            i2c_stop_on_bus(I2C0);
            return res;
        }
    }
    i2c_start_on_bus(I2C0);
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)){
        if ((core.getTick() - timer) > 1) {
            res = 6;
            i2c_stop_on_bus(I2C0);
            return res;
        }
    }
    i2c_master_addressing(I2C0, sensor_OWN_ADDRESS, I2C_RECEIVER);
    
    if(len < 3){
        /* disable acknowledge */
        i2c_ack_config(I2C0,I2C_ACK_DISABLE);
    }
    
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)){
        if ((core.getTick() - timer) > 1) {
            res = 7;
            i2c_stop_on_bus(I2C0);
            return res;
        }
    }
    stat0 = I2C_STAT0(I2C0);
    stat1 = I2C_STAT1(I2C0);
    
    if(1 == len){
        /* send a stop condition to I2C bus */
        i2c_stop_on_bus(I2C0);
    }
    
    while(len){
        if(3 == len){
            /* wait until BTC bit is set */
            while(!i2c_flag_get(I2C0, I2C_FLAG_BTC));

            /* disable acknowledge */
            i2c_ack_config(I2C0,I2C_ACK_DISABLE);
        }
        if(2 == len){
            /* wait until BTC bit is set */
            while(!i2c_flag_get(I2C0, I2C_FLAG_BTC));
            
            /* send a stop condition to I2C bus */
            i2c_stop_on_bus(I2C0);
        }
        
        if(i2c_flag_get(I2C0, I2C_FLAG_RBNE)){
            *reg++ = i2c_data_receive(I2C0);
            len--;
        }
        
        if ((core.getTick() - timer) > 1) {
            res = 8;
            i2c_stop_on_bus(I2C0);
            return res;
        }
        //i2c_stop_on_bus(I2C0);
    }
    while(I2C_CTL0(I2C0) & 0x0200);
    
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);
    
    i2c_ackpos_config(I2C0,I2C_ACKPOS_CURRENT);
    return res;
}
//-----------------------------------------------------
uint8_t Sensor::write(uint8_t* reg, uint8_t address, uint8_t len)
{
    //return I2C_Write_Transaction(sensor_OWN_ADDRESS, address, reg, len);
    
    i2c_start_on_bus(I2C0);
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
    i2c_master_addressing(I2C0, sensor_OWN_ADDRESS, I2C_TRANSMITTER);
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
    i2c_data_transmit(I2C0, address); 
    while(!i2c_flag_get(I2C0, I2C_FLAG_BTC));
    while(len--){
        i2c_data_transmit(I2C0, *reg); 
        while(!i2c_flag_get(I2C0, I2C_FLAG_BTC));
    }
    i2c_stop_on_bus(I2C0);
    while(!i2c_flag_get(I2C0, I2C_FLAG_RBNE));
    
    return true;
}
//-----------------------------------------------------
void Sensor::testProg(void)
{
    volatile bool isFault = false;
    static uint8_t counter = 0;
    
    isFault |= this->read(&this->gesture, 0x01, 1);
    isFault |= this->read(&this->status, 0x02, 1);
    isFault |= this->read(&this->periodMonitor, 0x89, 1);
    /*
    isFault |= this->read(&this->p1[0], 0x03, 1);
    isFault |= this->read(&this->p1[1], 0x04, 1);
    isFault |= this->read(&this->p1[2], 0x05, 1);
    isFault |= this->read(&this->p1[3], 0x06, 1);
    */
    isFault |= this->read(this->p1, 0x03, 4);
    //this->read(this->p2, 0x09, 4); //isOk &= this->read(this->p2, 0x09, 4); ///!!! иногда постоянно начинает возвращать false
    
    if (isFault == false){
        counter = 0;
        this->x1 = sensor.WIDTH - ((this->p1[2] & 0x0F)*256 + this->p1[3]);
        this->y1 = (this->p1[0] & 0x0F)*256 + this->p1[1];
        //this->x2 = sensor.WIDTH - ((this->p2[2] & 0x0F)*256 + this->p2[3]);
        //this->y2 = (this->p2[0] & 0x0F)*256 + this->p2[1];
        this->event1 = this->p1[2]>>6;
        //this->event2 = this->p2[2]>>6;
    }
    else{
        //I2C_ClearFlag(I2C1, I2C_ICR_STOPCF);
        if (counter < 10) counter++;
        else{
            counter = 0;
            this->initialize();
        }
    }
}
//-----------------------------------------------------
uint32_t Sensor::getTickTouch(void)
{
    return (core.getTick() - this->tickWhenTouching);
}
//-----------------------------------------------------
void Sensor::resetTickTouch(void)
{
    this->tickWhenTouching = core.getTick();
}
//-----------------------------------------------------
extern "C" void EXTI5_9_IRQHandler(void)
{ 
    if (exti_interrupt_flag_get(EXTI_5) != RESET)
    {
        exti_interrupt_flag_clear(EXTI_5);
        sensor.isPress = true;
    }
}
//-----------------------------------------------------
extern "C" void EXTI0_IRQHandler(void)
{ 
    if (exti_interrupt_flag_get(EXTI_0) != RESET)
    {
        exti_interrupt_flag_clear(EXTI_0);
        if (gpio_input_bit_get(GPIOA, GPIO_PIN_0) != RESET){
            // нажатие
            sensor.button = 1;
            sensor.tickWhenPress = core.getTick();
        }
        else{
            // отпускание
            sensor.button = 0;
            sensor.timerPress = core.getTick()- sensor.tickWhenPress;
        }
    }
}
//-----------------------------------------------------
