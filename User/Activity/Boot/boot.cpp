/******************************************************************************
* ООО Теплостар
* Самара
* 
* Программисты: Клюев А.А.
* 
* 08.08.2018
* Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
// Activity
#include "main.h"
#include "boot.h"
// Objects
#include "heater.h"
#include "start_timer.h"
#include "slider.h"
#include "checkbox.h"
#include "clock.h"
// Drivers
#include "temperature.h"
#include "core.h"
#include "usart.h"
#include "can.h"
#include "bluetooth.h"
#include "memory.h"
#include "display.h"
#include "sensor.h"
#include "canvas.h"
#include "text.h"

Boot boot;
//-----------------------------------------------------
void Boot::receiveInterrupt(uint8_t byte)
{
    static uint8_t array[12];
    static uint32_t count=0;
    
     // старт приема изображения
        if (this->rxFlag == 0){
            if (byte == 0xAA){
                count = 0;
                array[count++] = byte;
                this->rxFlag = 1;
            }
        }
        // прием заголовка изображения
        else if (this->rxFlag == 1){
            array[count++] = byte;
            if (count >= 12) this->rxFlag = 2;
        }        
        // обработка заголовка изображения
        else if (this->rxFlag == 2){
            if (array[1]==0xCD && array[2]==0xBA){
                if (array[3]==0x00){    // прием изображения
                    this->image_w = array[5]*256+array[4];
                    this->image_h = array[7]*256+array[6];
                    this->image_count = this->image_w*this->image_h*3;
                    this->command = 0x12;
                }
                else if (array[3]==0xC0){   // прием команд для изображений
                    this->command = array[8];
                    this->rxFlag = 0;
                }
            }
            else{
                this->rxFlag = 0;
            }
        }
        // загрузка изображения
        else if (this->rxFlag == 3){
            if (this->image_count > 0){
                this->image_bytes[this->image_point1++] = byte;
                this->image_numb++;
            }
            else{
                this->command = 0x14;
                this->rxFlag = 0;
            }
        }
}
//-----------------------------------------------------
void Boot::viewScreen(void)
{
    uint8_t a;
    
    a = memory.initializeSpi();
    canvas.writeFillRect(0,0,320,240,display.COLOR_BACK);
    if (a == MEM_OK) text.writeString(160-11*5,40,"Boot mode",Font_11x18,display.WHITE,display.COLOR_BACK);
    else{
        text.writeString(160-11*6,40,"memory error",Font_11x18,display.WHITE,display.COLOR_BACK);
    }
    display.setLight(100);
    display.switchOn();
}
//-----------------------------------------------------
void Boot::handler(void)
{
    uint8_t array[12];
    uint32_t N;
    uint8_t x, n;
    char str[32];
    
    while(1)
    {
        if (this->command == 0xFF){      // очистка памяти
            text.writeString(160-11*7,60,"erase memory...",Font_11x18,display.WHITE,display.COLOR_BACK);
            memory.chipErase();
            array[0] = 0xAA;
            array[1] = 0xCD;
            array[2] = 0xBA;
            array[3] = 0xCF;
            array[8] = 0xFF;
            heater.send(array, 12);
            this->command = 0;
            canvas.writeFillRect(0,60,320,60,display.COLOR_BACK);
        }
        if (this->command == 0xF9){      // удаление последнего изображения
            memory.lastErase();
            this->command = 0;
        }
        if (this->command == 0x55){      // сброс
            NVIC_SystemReset();
            this->command = 0;
        }
        if (this->command == 0x12){      // прием изображения
            display.setAddrWindow(0,0,this->image_h,this->image_w);
            this->image_addr = memory.searchEnd(&N);
            array[1] = 0xCD;
            array[2] = 0xBA;
            array[3] = 0x00;
            array[4] = this->image_w & 0xFF;
            array[5] = this->image_w >> 8;
            array[6] = this->image_h & 0xFF;
            array[7] = this->image_h >> 8;
            array[8] = 0x00;
            array[9] = 0x00;
            array[10] = 0x00;
            array[11] = 0x00;
            memory.writePage(&array[1], this->image_addr, 11);
            this->image_addr += 11;
            this->rxFlag = 3;
            this->command = 0;
            n=0;
            for (x=0; x<32; x++) str[x] = 0;
            str[n++] = 'r'; str[n++] = 'e'; str[n++] = 'c'; str[n++] = 'e'; str[n++] = 'i'; str[n++] = 'v'; str[n++] = 'e'; str[n++] = ' '; 
            str[n++] = 'i'; str[n++] = 'm'; str[n++] = 'a'; str[n++] = 'g'; str[n++] = 'e'; str[n++] = ' ';
            n += text.decToString(&str[n], N);
            str[n++] = '.'; str[n++] = '.'; str[n++] = '.'; 
            text.writeString(160-11*9,60,str,Font_11x18,display.WHITE,display.COLOR_BACK);
        }
        if (this->image_numb > 0){                          // загрузка изображения
            this->image_numb--;
            this->image_count--;
            this->image_point = this->image_point1 - this->image_point2;
            memory.writePage(&this->image_bytes[this->image_point2++], this->image_addr++, 1);
            this->command = 0;
        }
        if (this->command == 0x14 && this->image_numb==0){                          // окончание загрузки изображения
            this->command = 0;
            canvas.writeFillRect(0,60,320,60,display.COLOR_BACK);
        }
    }
}
//-----------------------------------------------------
