/******************************************************************************
* ООО Теплостар
* Самара
* 
* Программисты: Клюев А.А.
* 
* 28.03.2022
* Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "memory.h"
#include "core.h"
#include "gd32f30x_spi.h"

Memory memory;
//-----------------------------------------------------
uint8_t Memory::initializeSpi(void)
{
    uint8_t a, i;
    
    this->initSpi();
    MEM_RST_ON;
    core.delayMs(1);
    MEM_RST_OFF;
    core.delayMs(1);
    MEM_WP_OFF;
    this->isQuadMode = 0;
    a = this->ident();
    ///*
    if (a != MEM_OK){
        for (i=0; i<10; i++){
            core.delayMs(100);
            MEM_CS_ON;
            this->quadSend(W25_EXIT_QPI);
            MEM_CS_OFF;
            MEM_CS_ON;
            this->quadSend(W25_WRITE_ENABLE);
            MEM_CS_OFF;
            MEM_CS_ON;
            this->quadSend(W25_WRITE_STATUS_2);
            this->quadSend(0x00);
            MEM_CS_OFF;
            this->isQuadMode = 0;
            a = this->ident();
            if (a != MEM_OK){
                a = this->initializeQuad();
                a = this->ident();
            }
            else break;
        }
    }//*/
    return a;
}
//-----------------------------------------------------
uint8_t Memory::initializeQuad(void)
{
    uint8_t a, reg[3];
    uint32_t www;
    Status_reg_1_t r;
    
    this->initSpi();
    MEM_RST_ON;
    core.delayMs(1);
    MEM_RST_OFF;
    core.delayMs(1);
    this->isQuadMode = 0;
    this->powerUp();
    a = this->ident();
    if (a == MEM_IDENT_ERROR){
        this->isQuadMode = 1;
        this->powerUp();
        a = this->ident();
        return a;
    }
    
    core.delayMs(1);
    MEM_CS_ON;
    this->spiSend(W25_WRITE_ENABLE);
    MEM_CS_OFF;
    
    www = 1000;
    do {
        r.all=this->readStatus();
        core.delayUs(10);
        if (www > 0) www--;
        else break;
    } while(r.bit.busy);
    
    MEM_CS_ON;
    this->spiSend(W25_WRITE_STATUS_2);
    this->spiSend(0x02);
    MEM_CS_OFF;
    
    www = 1000;
    do {
        r.all=this->readStatus();
        core.delayUs(10);
        if (www > 0) www--;
        else break;
    } while(r.bit.busy);
    
    MEM_CS_ON;
    this->spiSend(W25_READ_STATUS_1);
    this->read(&reg[0],1);
    MEM_CS_OFF;
    
    MEM_CS_ON;
    this->spiSend(W25_READ_STATUS_2);
    this->read(&reg[1],1);
    MEM_CS_OFF;
    
    MEM_CS_ON;
    this->spiSend(W25_READ_STATUS_3);
    this->read(&reg[2],1);
    MEM_CS_OFF;
    
    core.delayMs(1);
    MEM_CS_ON;
    this->spiSend(W25_WRITE_STATUS_2);
    this->spiSend(0x02);
    MEM_CS_OFF;
    
    www = 1000;
    do {
        r.all=this->readStatus();
        core.delayUs(10);
        if (www > 0) www--;
        else break;
    } while(r.bit.busy);
    
    MEM_CS_ON;
    this->spiSend(W25_ENTER_QPI);
    MEM_CS_OFF;
    
    www = 1000;
    do {
        r.all=this->readStatus();
        core.delayUs(10);
        if (www > 0) www--;
        else break;
    } while(r.bit.busy);
    
    this->isQuadMode = 1;
    core.delayMs(1);
    /*
    this->writeEnable();
    
    MEM_CS_ON;
    this->quadOutInit();
    this->quadSend(W25_READ_STATUS_1);
    this->quadInInit();
    reg[0] = this->quadReceiv();
    MEM_CS_OFF;
    
    MEM_CS_ON;
    this->quadOutInit();
    this->quadSend(W25_READ_STATUS_2);
    this->quadInInit();
    reg[1] = this->quadReceiv();
    MEM_CS_OFF;
    
    MEM_CS_ON;
    this->quadOutInit();
    this->quadSend(W25_READ_STATUS_3);
    this->quadInInit();
    reg[2] = this->quadReceiv();
    MEM_CS_OFF;
    
    a = this->ident();
    */
    return a;
}
//-----------------------------------------------------
void Memory::initSpi(void)
{
    // Enable the GPIO Clock
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_AF);
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
    // Configure the Reset pin
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    MEM_RST_OFF;  
    // Configure the WP pin
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
    MEM_WP_OFF;
    // Configure the CS pin
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    MEM_CS_OFF;
    // Configure the DO pin
    gpio_init(GPIOC, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
    // Configure the DI pin
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
    MEM_DI_OFF;
    // Configure the CLK pin
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
    MEM_CLK_OFF;
}
//-----------------------------------------------------
void Memory::quadOutInit(void)
{
    // Configure the IO_0(DI) pin
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
    // Configure the IO_1(DO) pin
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
    // Configure the IO_2(WP) pin
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
    // Configure the IO_3(Reset) pin
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
}
//-----------------------------------------------------
void Memory::quadInInit(void)
{
    // Configure the IO_0(DI) pin
    gpio_init(GPIOB, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
    // Configure the IO_1(DO) pin
    gpio_init(GPIOC, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
    // Configure the IO_2(WP) pin
    gpio_init(GPIOC, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
    // Configure the IO_3(Reset) pin
    gpio_init(GPIOB, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
}
//-----------------------------------------------------
uint8_t Memory::spiSend(uint8_t data)
{
    uint8_t temp, i;
    
    for (i=0; i<8; i++){
        MEM_CLK_ON;
        if (data&0x80){
            MEM_DI_OFF;
        }
        else{
            MEM_DI_ON;
        }
        data = data<<1;
        MEM_CLK_OFF;
        temp = temp<<1;
        if (MEM_DO_READ){
            temp |= 0x01;
        }
        else{
            temp &= ~0x01;
        }
    }
    
    return temp;
}
//-----------------------------------------------------
void Memory::quadSend(uint8_t data)
{
    uint8_t i;
    
    for (i=0; i<2; i++){
        MEM_CLK_ON;
        //core.delayMs(1);
        
        if (data&0x10) MEM_IO0_OFF;
        else MEM_IO0_ON;
        if (data&0x20) MEM_IO1_OFF;
        else MEM_IO1_ON;
        if (data&0x40) MEM_IO2_OFF;
        else MEM_IO2_ON;
        if (data&0x80) MEM_IO3_OFF;
        else MEM_IO3_ON;
        
        MEM_CLK_OFF;
        //core.delayMs(1);
        data = data << 4;
    }
}
//-----------------------------------------------------
uint8_t Memory::quadReceiv(void)
{
    uint8_t temp, i;
    
    for (i=0; i<2; i++){
        MEM_CLK_ON;
        //core.delayMs(1);
        MEM_CLK_OFF;
        //core.delayMs(1);
        
        temp = temp<<4;
        if (MEM_IO0_READ) temp |= 0x01;
        else temp &= ~0x01;
        if (MEM_IO1_READ) temp |= 0x02;
        else temp &= ~0x02;
        if (MEM_IO2_READ) temp |= 0x04;
        else temp &= ~0x04;
        if (MEM_IO3_READ) temp |= 0x08;
        else temp &= ~0x08;
    }
    return temp;
}
//-----------------------------------------------------
uint8_t Memory::write(uint8_t* data, uint8_t len)
{
    while(len--) {
        this->spiSend(*data++);
    }
    return 0;
}
//-----------------------------------------------------
uint8_t Memory::read(uint8_t* data, uint8_t len)
{
    while(len--) {
        *data++=this->spiSend(0x00);
    }
    return 0;
}
//-----------------------------------------------------
uint8_t Memory::writeEnable(void)
{
    MEM_CS_ON;
    if (!this->isQuadMode){
        this->spiSend(W25_WRITE_ENABLE);
    }
    else{
        this->quadOutInit();
        this->quadSend(W25_WRITE_ENABLE);
    }
    MEM_CS_OFF;
    
    return 0;
}
//-----------------------------------------------------
uint8_t Memory::powerDown(void)
{
    MEM_CS_ON;
    if (!this->isQuadMode){
        this->spiSend(W25_POWER_DOWN);
    }
    else{
        this->quadOutInit();
        this->quadSend(W25_POWER_DOWN);
    }
    MEM_CS_OFF;
    
    return 0;
}
//-----------------------------------------------------
uint8_t Memory::powerUp(void)
{
    MEM_CS_ON;
    if (!this->isQuadMode){
        this->spiSend(W25_POWER_UP);
    }
    else{
        this->quadOutInit();
        this->quadSend(W25_POWER_UP);
    }
    MEM_CS_OFF;
    
    return 0;
}
//-----------------------------------------------------
uint8_t Memory::readPage(uint8_t* data, uint32_t addr, uint32_t len)
{
    MEM_CS_ON;
    if (!this->isQuadMode){
        this->spiSend(W25_READ);
        this->spiSend((addr >> 16) & 0xFF);
        this->spiSend((addr >> 8) & 0xFF);
        this->spiSend(addr & 0xFF);
        while(len--) {
            *data++=this->spiSend(0x00);
        }
    }
    else{
        this->quadOutInit();
        this->quadSend(W25_FAST_READ_QUAD);
        this->quadSend((addr >> 16) & 0xFF);
        this->quadSend((addr >> 8) & 0xFF);
        this->quadSend(addr & 0xFF);
        this->quadSend(0x00);
        this->quadInInit();
        while(len--) {
            *data++=this->quadReceiv();
        }
    }
    MEM_CS_OFF;
    return 0;
}
//-----------------------------------------------------
uint8_t Memory::writePage(uint8_t* data, uint32_t addr, uint8_t len)
{
    Status_reg_1_t r;
    
    this->writeEnable();
    
    MEM_CS_ON;
    if (!this->isQuadMode){
        this->spiSend(W25_PAGE_PROGRAMM);
        this->spiSend((addr >> 16) & 0xFF);
        this->spiSend((addr >> 8) & 0xFF);
        this->spiSend(addr & 0xFF);
        while(len--) {
            this->spiSend(*data++);
        }
    }
    else{
        this->quadOutInit();
        this->quadSend(W25_PAGE_PROGRAMM);
        this->quadSend((addr >> 16) & 0xFF);
        this->quadSend((addr >> 8) & 0xFF);
        this->quadSend(addr & 0xFF);
        while(len--) {
            this->quadSend(*data++);
        }
    }
    MEM_CS_OFF;
    
    do {
        r.all=this->readStatus();
    } while(r.bit.busy);
    
    return 0;
}
uint8_t Memory::writeByte(uint8_t data, uint32_t addr, uint32_t len)
{
    Status_reg_1_t r;
    
    this->writeEnable();
    
    MEM_CS_ON;
    if (!this->isQuadMode){
        this->spiSend(W25_PAGE_PROGRAMM);
        this->spiSend((addr >> 16) & 0xFF);
        this->spiSend((addr >> 8) & 0xFF);
        this->spiSend(addr & 0xFF);
        while(len--) {
            this->spiSend(data);
        }
    }
    else{
        this->quadOutInit();
        this->quadSend(W25_PAGE_PROGRAMM);
        this->quadSend((addr >> 16) & 0xFF);
        this->quadSend((addr >> 8) & 0xFF);
        this->quadSend(addr & 0xFF);
        while(len--) {
            this->quadSend(data);
        }
    }
    MEM_CS_OFF;
    
    do {
        r.all=this->readStatus();
    } while(r.bit.busy);
    
    return 0;
}
//-----------------------------------------------------
void Memory::blockErase(uint32_t addr)
{
    Status_reg_1_t r;
    
    this->writeEnable();
    
    MEM_CS_ON;
    if (!this->isQuadMode){
        this->spiSend(W25_BLOCK_ERASE);
        this->spiSend((addr >> 16) & 0xFF);
        this->spiSend((addr >> 8) & 0xFF);
        this->spiSend(addr & 0xFF);
    }
    else{
        this->quadOutInit();
        this->quadSend(W25_BLOCK_ERASE);
        this->quadSend((addr >> 16) & 0xFF);
        this->quadSend((addr >> 8) & 0xFF);
        this->quadSend(addr & 0xFF);
    }
    MEM_CS_OFF;
    
    do {
        r.all=this->readStatus();
    } while(r.bit.busy);
}
//-----------------------------------------------------
uint8_t Memory::writeZero(uint32_t addr, uint8_t len)
{
    this->writeEnable();
     
    MEM_CS_ON;
    this->spiSend(W25_PAGE_PROGRAMM);
    this->spiSend((addr >> 16) & 0xFF);
    this->spiSend((addr >> 8) & 0xFF);
    this->spiSend(addr & 0xFF);
    while(len--) {
        this->spiSend(0xFF);
    }
    MEM_CS_OFF;
    return 0;
}
//-----------------------------------------------------
void Memory::reset(void)
{
    MEM_CS_ON;
    this->spiSend(W25_ENABLE_RESET);
    MEM_CS_OFF;
 
    MEM_CS_ON;
    this->spiSend(W25_RESET);
    MEM_CS_OFF;
}
//-----------------------------------------------------
uint8_t Memory::readStatus(void)
{
    uint8_t resp;
    MEM_CS_ON;
    if (!this->isQuadMode){
        this->spiSend(W25_READ_STATUS_1);
        this->read(&resp,1);
    }
    else{
        this->quadOutInit();
        this->quadSend(W25_READ_STATUS_1);
        this->quadInInit();
        resp = this->quadReceiv();
    }
    MEM_CS_OFF;
    
    return resp;
}
//-----------------------------------------------------
void Memory::chipErase(void)
{
    Status_reg_1_t r;
    
    this->writeEnable();
    
    MEM_CS_ON;
    if (!this->isQuadMode){
        this->spiSend(W25_CHIP_ERASE);
    }
    else{
        this->quadOutInit();
        this->quadSend(W25_CHIP_ERASE);
    }
    MEM_CS_OFF;
    
    do {
        r.all=this->readStatus();
    } while(r.bit.busy);
}
//-----------------------------------------------------
void Memory::fillPage(void)
{
    Status_reg_1_t r;
    uint8_t page[255];
    uint32_t i;
    
    for (i=0;i<255;i++) {
        page[i]=i;
    }
    r.all=this->readStatus();
    if (!r.bit.busy) {
        this->writePage(page, 0, 10);
        do {
            r.all=this->readStatus();
        } while(r.bit.busy);
    }
}
//-----------------------------------------------------
uint8_t Memory::ident(void)
{
    uint8_t get_id[]={W25_GET_DEV_ID, 0,0,0};
    uint8_t buff[2], ret;
    
    MEM_CS_ON;
    if (!this->isQuadMode){
        this->write(get_id, 4);
        this->read(buff,2);
    }
    else{
        this->quadOutInit();
        this->quadSend(get_id[0]);
        this->quadSend(get_id[1]);
        this->quadSend(get_id[2]);
        this->quadSend(get_id[3]);
        this->quadInInit();
        buff[0] = this->quadReceiv();
        buff[1] = this->quadReceiv();
        
    }
    MEM_CS_OFF;
    
    if (buff[0] != 0xef && buff[1] != 0x15) {
        ret=MEM_IDENT_ERROR;
    }
    else {
        ret=MEM_OK;
    }
    return ret;
}
//-----------------------------------------------------
uint32_t Memory::searchEnd(uint32_t* N)
{
    uint32_t x=0;
    uint8_t data[11];
    
    *N = 0;
    while(1){
        this->readPage(data, x, 11);
        if (data[0]==0xCD && data[1]==0xBA && data[2]==0x00){
            x += (data[4]*256+data[3])*(data[6]*256+data[5])*3+11;
        }
        else return x;
        *N = *N + 1;
    }
}
//-----------------------------------------------------
uint32_t Memory::searchImage(uint32_t N)
{
    uint32_t x=0;
    uint8_t data[11];
    
    while(N--){
        this->readPage(data, x, 11);
        if (data[0]==0xCD && data[1]==0xBA && data[2]==0x00){
            x += (data[4]*256+data[3])*(data[6]*256+data[5])*3+11;
        }
        else return 0xFFFFFFFF;
    }
    return x;
}
//-----------------------------------------------------
void Memory::lastErase(void)
{
    uint32_t a, x, addr=11;
    
    for (x=0; x<512; x++){
        a = this->searchImage(x);
        if (a == 0xFFFFFFFF){
            
            this->writeZero(addr-11, 11);
            break;
        }
        addr = a;
    }
}
//-----------------------------------------------------
int32_t Memory::calcCRC(void)
{
    static uint32_t x=0, max=0;
    uint8_t data[256];
    uint8_t j, bt, AByte;
    uint16_t i;
    static uint16_t crc=0xFFFF;
    
    if (max == 0){
        while(1){
            readPage(data, x, 11);
            if (data[0]==0xCD && data[1]==0xBA && data[2]==0x00){
                x += (data[4]*256+data[3])*(data[6]*256+data[5])*3+11;
            }
            else break;
        }
        max = x;
        x = 0;
    }
    if (x<max){
        readPage(data, x, 256);
        for (i=0; i<256; i++){
            AByte = data[i];
            for(j=1; j<=8; j++) {
                bt = crc & 1;
                crc = crc >> 1;
                if ((AByte & 1)!=bt) crc = crc^0xA001;
                AByte = AByte >> 1;
            }
        }
        x += 256;
        return -1;
    }
    else return crc;
}
//-----------------------------------------------------
