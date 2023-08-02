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
#include "main.h"
// Objects
#include "slider.h"
// Drivers
#include "temperature.h"
#include "core.h"
#include "usart.h"
#include "can.h"
#include "memory.h"
#include "display.h"
#include "sensor.h"
#include "canvas.h"
#include "text.h"
// Library
#include <math.h>

Slider slider;
//-----------------------------------------------------

void Slider::initialize(void)
{
    uint32_t i;
    float d, e, f, g;
    
    f = 5.28;    
    d = (float)(1-f)/(float)slider.VALUE_LEN;
    g = this->offValue; // minValue
    e = (float)(this->maxValue-this->offValue)/(float)slider.VALUE_LEN; // minValue
    for (i=0; i<VALUE_LEN; i++){
        this->x[i] = sin(f)*SLIDER_RADIUS_ALL+160;                // расчет возможных положений ползунка
        this->y[i] = cos(f)*SLIDER_RADIUS_ALL+130;
        this->valueX[i] = sin(f)*VALUE_RADIUS_ALL+160;                // расчет возможных положений шкалы
        this->valueY[i] = cos(f)*VALUE_RADIUS_ALL+130;
        this->values[i] = g + 0.5f;          // расчет возможных значений шкалы
        g += e;
        if (g > this->maxValue) g = this->maxValue;
        f += d;
    }
    for (i=VALUE_LEN-5; i<VALUE_LEN; i++){
        this->values[i] = this->maxValue;
    }
}
//-----------------------------------------------------
void Slider::viewScreen(void)
{
    display.turnOff();                                              // отключение дисплея
    canvas.loadImage(IMAGE_BCK_X,IMAGE_BCK_Y,BACKGROUND_IMAGE);     // загрузка начального изображения
    display.switchOn();                                             // включение дисплея
    ///slider.drawDigGrid();   
    slider.positionTemp = VALUE_LEN-1;
    //slider.position = slider.positionTemp;
    slider.old = slider.position;
    slider.setVal(slider.positionTemp);
    // отображение шкалы
    this->isFirst = true;
    // кнопки управления шкалой
    text.writeStringOver(BUTTON_DEC_X,BUTTON_DEC_Y,"-",Font_11x18,display.LIGHTGREY,display.COLOR_BACK,0);
    text.writeStringOver(BUTTON_INC_X,BUTTON_INC_Y,"+",Font_11x18,display.LIGHTGREY,display.COLOR_BACK,0);
}
//-----------------------------------------------------
bool Slider::setPosition(int16_t val)
{
    uint32_t i;
    
    for (i=0; i<VALUE_LEN; i++){
        if (this->values[i] == val){
            this->position = i;
            return true;
        }
    }
    return false;
}
//-----------------------------------------------------
uint8_t Slider::viewPosition(void)
{
    int16_t s;
    uint16_t i;
    int32_t x, a, b;
    char str[2][7];
    
    if (timerSliderMin<core.getTick()){
        this->touchOld = 1;
        timerSliderMin = 0xFFFFFFFF;
    }
    if (this->touchOld==1){                                                 // при отпущенном слайдере
        this->touchOld = 0;
        if (this->values[this->position] < this->minValue){                 // и ниже минимума
            if (this->position > 0){                                        // (но не off) 
                for (i=0; i<VALUE_LEN; i++){                                // ставим на позицию минимума
                    if (this->values[i] == this->minValue){
                        this->position = i;
                    }
                }
            }
        }
    }
    
    if (this->positionTemp < this->position){               // анимация движения ползунка вправо
            this->old = this->positionTemp;
            if ((this->maxValue - this->minValue) < 30){
                s = this->values[this->positionTemp];
                while(1){
                    if (this->positionTemp >= (this->VALUE_LEN-1)) break;
                    if (this->positionTemp == this->position) break;
                    this->positionTemp++;
                    if (this->values[this->positionTemp] != s) break;
                }
            }
            else {
                if (this->isFirst){
                    this->isFirst = false;
                    this->positionTemp++;
                }
                else this->positionTemp += SLIDER_STEP;
                if (this->positionTemp > this->position) this->positionTemp = this->position;
            }
            
            this->viewGrid(this->positionTemp,BACKGROUND_IMAGE);     // красная шкала
            
            for (x=0; x<6; x++) str[0][x] = 0;
            a = text.decToString(str[0], this->values[this->old]);
            if (display.setup.celsius){
                if (this->old < (slider.VALUE_LEN/2)) a=SLIDER_FONT.width*(a-1);
                else a = 0;
            }
            else{
                if (this->old < (slider.VALUE_LEN/2)) a=VALUE_FONT.width*(a-1);
                else a = 0;
            }
            for (x=0; x<6; x++) str[1][x] = 0;
            b = text.decToString(str[1], this->values[this->positionTemp]);
            if (display.setup.celsius){
                if (this->positionTemp < (slider.VALUE_LEN/2)) b=SLIDER_FONT.width*(b-1);
                else b = 0;
            }
            else{
                if (this->positionTemp < (slider.VALUE_LEN/2)) b=VALUE_FONT.width*(b-1);
                else b = 0;
            }
            
            this->clearDigRight(this->old);
            if (display.setup.celsius){
                viewString();
            }
            else{
                viewString();
            }
            this->viewDigLeft(this->positionTemp);
    }
    if (this->positionTemp > this->position){               // анимация движения ползунка влево
            this->old = this->positionTemp;
            if ((this->maxValue - this->minValue) < 30){
                s = this->values[this->positionTemp];
                while(1){
                    if (this->positionTemp <= 0) break;
                    if (this->positionTemp == this->position) break;
                    this->positionTemp--;
                    if (this->values[this->positionTemp] != s) break;
                }
            }
            else {
                if (this->isFirst){
                    this->isFirst = false;
                    this->positionTemp--;
                }
                else this->positionTemp -= SLIDER_STEP;
                if (this->positionTemp < this->position) this->positionTemp = this->position;
            }
            
            this->viewGridInvert(this->positionTemp,BACKGROUND_IMAGE);     // синяя шкала
            
            for (x=0; x<6; x++) str[0][x] = 0;
            a = text.decToString(str[0], this->values[this->old]);
            if (display.setup.celsius){
                if (this->old < (slider.VALUE_LEN/2)) a=SLIDER_FONT.width*(a-1);
                else a = 0;
            }
            else{
                if (this->old < (slider.VALUE_LEN/2)) a=VALUE_FONT.width*(a-1);
                else a = 0;
            }
            for (x=0; x<6; x++) str[1][x] = 0;
            b = text.decToString(str[1], this->values[this->positionTemp]);
            if (display.setup.celsius){
                if (this->positionTemp < (slider.VALUE_LEN/2)) b=SLIDER_FONT.width*(b-1);
                else b = 0;
            }
            else{
                if (this->positionTemp < (slider.VALUE_LEN/2)) b=VALUE_FONT.width*(b-1);
                else b = 0;
            }
            
            this->clearDigLeft(this->old);
            if (display.setup.celsius){
                viewString();
            }
            else{
                viewString();
            }
            this->viewDigRight(this->positionTemp);
    }
    return (this->positionTemp != this->position);
}
//-----------------------------------------------------
void Slider::viewString(void)
{
    int32_t x, a, b;
    char str[2][7];
    bool isOff[2];
    
    for (x=0; x<6; x++) str[0][x] = 0;
    if (this->offValue>=0 && this->old==0){
        a=0;
        str[0][a++] = 'o';
        str[0][a++] = 'f';
        str[0][a++] = 'f';
        isOff[0] = true;
    }
    else{
       if (this->values[this->old] >= this->minValue) a = text.decToString(str[0], this->values[this->old]); 
       else a = text.decToString(str[0], this->minValue); 
       isOff[0] = false;
    }
    if (this->old < (slider.VALUE_LEN/2)){
        if (isOff[0] == false) a=SLIDER_FONT.width*(a-1);
        else a=VALUE_FONT.width*(a-1);
    }
    else a = 0;
    for (x=0; x<6; x++) str[1][x] = 0;
    if (this->offValue>=0 && this->positionTemp==0){
        b=0;
        str[1][b++] = 'o';
        str[1][b++] = 'f';
        str[1][b++] = 'f';
        isOff[1] = true;
    }
    else{
        if (this->values[this->positionTemp] >= this->minValue) b = text.decToString(str[1], this->values[this->positionTemp]);
        else b = text.decToString(str[1], this->minValue);
        isOff[1] = false;
    }
    if (this->positionTemp < (slider.VALUE_LEN/2)){
        if (isOff[1] == false) b=SLIDER_FONT.width*(b-1);
        else b=VALUE_FONT.width*(b-1);
    }
    else b = 0;
    if (isOff[0] == false){
        if (old < VALUE_LEN) text.writeStringOver(valueX[old]-SLIDER_FONT.width/2-a,valueY[old]-SLIDER_FONT.height/2,str[0],SLIDER_FONT,display.COLOR_BACK,display.COLOR_BACK,1);
    }else{
        if (old < VALUE_LEN) text.writeStringOver(valueX[old]-SLIDER_FONT.width/2-a,valueY[old]-SLIDER_FONT.height/2,str[0],VALUE_FONT,display.COLOR_BACK,display.COLOR_BACK,1);
    }
    if (isOff[1] == false){
        if (positionTemp < VALUE_LEN) text.writeStringOver(valueX[positionTemp]-SLIDER_FONT.width/2-b,valueY[positionTemp]-SLIDER_FONT.height/2,str[1],SLIDER_FONT,display.COLOR_TEMP,display.COLOR_BACK,1);
    }else{
        if (positionTemp < VALUE_LEN) text.writeStringOver(valueX[positionTemp]-SLIDER_FONT.width/2-b,valueY[positionTemp]-SLIDER_FONT.height/2,str[1],VALUE_FONT,display.COLOR_TEMP,display.COLOR_BACK,1);
    }
    if (isOff[0] == false){
        if (old < VALUE_LEN) text.writeStringOver(valueX[old]-SLIDER_FONT.width/2-a,valueY[old]-SLIDER_FONT.height/2,str[0],SLIDER_FONT,display.COLOR_BACK,display.COLOR_BACK,2);
    }else{
        if (old < VALUE_LEN) text.writeStringOver(valueX[old]-SLIDER_FONT.width/2-a,valueY[old]-SLIDER_FONT.height/2,str[0],VALUE_FONT,display.COLOR_BACK,display.COLOR_BACK,2);
    }
    if (isOff[1] == false){
        if (positionTemp < VALUE_LEN) text.writeStringOver(valueX[positionTemp]-SLIDER_FONT.width/2-b,valueY[positionTemp]-SLIDER_FONT.height/2,str[1],SLIDER_FONT,display.COLOR_TEMP,display.COLOR_BACK,2);
    }else{
        if (positionTemp < VALUE_LEN) text.writeStringOver(valueX[positionTemp]-SLIDER_FONT.width/2-b,valueY[positionTemp]-SLIDER_FONT.height/2,str[1],VALUE_FONT,display.COLOR_TEMP,display.COLOR_BACK,2);
    }
}
//-----------------------------------------------------
bool Slider::checkDigGrid(uint16_t pos)
{
    if ((this->values[pos]%(this->stepValueBig)) == 0) return true;
    if ((this->values[pos]%(this->stepValueSmall)) == 0) return true;
    if (pos == (slider.VALUE_LEN-1)) return true;
    if (pos == 0) return true;
    if (this->values[pos] == this->maxValue) return true;
    return false;
}
//-----------------------------------------------------
uint16_t Slider::getPosition(uint16_t* posX, uint16_t* posY, uint16_t pos)
{
    int16_t temp;
    
    temp = this->values[pos];
    while (true){
        if (pos == 0) break;
        if (this->values[pos] != temp) break;
        pos--;
    }
    pos++;
    *posX = this->valueX[pos];
    *posY = this->valueY[pos];
    return pos;
}
//-----------------------------------------------------
int8_t Slider::getCorrectXDigGrid(uint16_t pos, FontDef font, uint8_t len)
{
    int8_t result;
    
    if (pos < (slider.VALUE_LEN/2-20)) result = font.width*(len-1);
    else if (pos >= (slider.VALUE_LEN/2-20) && pos <= (slider.VALUE_LEN/2+20)) result = font.width*(len-1)/2;
    else result = 0;
    
    return result;
}
//-----------------------------------------------------
void Slider::eraseDigGrid(void)
{
    uint32_t color = display.BLACK;
    int32_t x, y, a, b, i;
    uint16_t posX, posY, pos;
    char str[7];
    
    y=0;
    for (i=0; i<this->VALUE_LEN; i++){
        for (x=0; x<6; x++) str[x] = 0;
        b = this->values[i];
        pos = this->getPosition(&posX, &posY, i);
        
        if (this->offValue>=0 && pos==0 && y!=b){
            y = b;
            a=0;
            str[a++] = 'o';
            str[a++] = 'f';
            str[a++] = 'f';
            pos = this->getPosition(&posX, &posY, pos);
            a = getCorrectXDigGrid(pos, VALUE_FONT, a);
            text.writeStringOver(posX-VALUE_FONT.width/2-a,posY-VALUE_FONT.height/2,str,VALUE_FONT,color,display.COLOR_BACK,0);
        }
        if ((b%(this->stepValueBig))==0 && y!=b){
            y = b;
            a = text.decToString(str, b);
            a = getCorrectXDigGrid(pos, VALUE_FONT, a);
            text.writeStringOver(posX-VALUE_FONT.width/2-a,posY-VALUE_FONT.height/2,str,VALUE_FONT,color,display.COLOR_BACK,0);
        }
        if (((b%(this->stepValueSmall))==0 || i==0 || i==(slider.VALUE_LEN-1)) && y!=b){
            y = b;
            a = text.decToString(str, b);
            a = getCorrectXDigGrid(pos, VALUE_SMALL_FONT, a);
            text.writeStringOver(posX-VALUE_SMALL_FONT.width/2-a,posY-VALUE_SMALL_FONT.height/2,str,VALUE_SMALL_FONT,color,display.COLOR_BACK,0);
        }
    }
}
//-----------------------------------------------------
void Slider::drawDigGrid(void)
{
    uint32_t color = COLOR_DIG_GRID;
    int32_t x, y, a, val, i;
    uint16_t posX, posY, pos;
    char str[7];
    
    y=0;
    for (i=0; i<this->VALUE_LEN; i++){
        for (x=0; x<6; x++) str[x] = 0;
        val = this->values[i];
        pos = this->getPosition(&posX, &posY, i);
        
        if (this->offValue>=0 && pos < 2 && y!=val){
            y = val;
            a=0;
            str[a++] = 'o';
            str[a++] = 'f';
            str[a++] = 'f';
            pos = this->getPosition(&posX, &posY, pos);
            a = getCorrectXDigGrid(pos, VALUE_FONT, a);
            text.writeStringOver(posX-VALUE_FONT.width/2-a,posY-VALUE_FONT.height/2,str,VALUE_FONT,color,display.COLOR_BACK,0);
        }
        else if ((val%(this->stepValueBig))==0 && y!=val){
            y = val;
            a = text.decToString(str, val);
            a = getCorrectXDigGrid(pos, VALUE_FONT, a);
            text.writeStringOver(posX-VALUE_FONT.width/2-a,posY-VALUE_FONT.height/2,str,VALUE_FONT,color,display.COLOR_BACK,0);
        }
        if (((val%(this->stepValueSmall))==0 || i==0 || i==(slider.VALUE_LEN-1)) && y!=val){
            y = val;
            a = text.decToString(str, val);
            a = getCorrectXDigGrid(pos, VALUE_SMALL_FONT, a);
            text.writeStringOver(posX-VALUE_SMALL_FONT.width/2-a,posY-VALUE_SMALL_FONT.height/2,str,VALUE_SMALL_FONT,color,display.COLOR_BACK,0);
        }
    }
}
//-----------------------------------------------------
void Slider::viewDigLeft(uint16_t pos)
{
    uint16_t i, x;
    
    i = pos;
    for (x=0; x<distanceL; x++){
        // идем влево от ползунка
        if (i > 0) i--;
        else break;
        if (checkDigGrid(i) == true && this->values[i] != this->values[pos] && x>offset){
            // достигли символа, рисуем
            textDigGrid(i, COLOR_DIG_GRID);
            ///break;
        }
    }
}
//-----------------------------------------------------
void Slider::clearDigLeft(uint16_t pos)
{
    uint16_t i, x;
    
    i = pos;
    for (x=0; x<distanceL; x++){
        // идем влево от ползунка
        if (i > 0) i--;
        else break;
        if (checkDigGrid(i) == true/* && this->values[i] != this->values[pos]*/){
            // достигли символа, стираем
            textDigGrid(i, display.BLACK);
            break;
        }
    }
}
//-----------------------------------------------------
void Slider::viewDigRight(uint16_t pos)
{
    uint16_t i, x, y;
    
    i = pos;
    for (y=0; y<distanceM; y++){
        if (i < (slider.VALUE_LEN-1)) i++;
        if ((this->values[i] != this->values[pos]) || i==(slider.VALUE_LEN-1)){
            break;
        }
    }
    pos = i;
    for (x=0; x<distanceR; x++){
        // идем вправо от ползунка
        if (i < (slider.VALUE_LEN-1)) i++;
        else break;
        if (checkDigGrid(i) == true && this->values[i] != this->values[pos] && x>offset){
            // достигли символа, рисуем
            textDigGrid(i, COLOR_DIG_GRID);
            ///break;
        }
    }
}
//-----------------------------------------------------
void Slider::clearDigRight(uint16_t pos)
{
    uint16_t i, x;
    
    i = pos;
    pos = i;
    for (x=0; x<distanceR; x++){
        // идем вправо от ползунка
        if (i < (slider.VALUE_LEN-1)) i++;
        else break;
        if (checkDigGrid(i) == true && this->values[i] != this->values[pos]){
            // достигли символа, стираем
            textDigGrid(i, display.BLACK);
            break;
        }
    }
}
//-----------------------------------------------------
void Slider::textDigGrid(uint16_t pos, uint32_t color)
{
    int32_t a;
    uint16_t posX, posY, val;
    char str[7] = {0,0,0,0,0,0,0};
    
    val = this->values[pos];
    pos = this->getPosition(&posX, &posY, pos);
    if (this->offValue>=0 && pos < 2){
        a=0;
        str[a++] = 'o';
        str[a++] = 'f';
        str[a++] = 'f';
        a = getCorrectXDigGrid(pos, VALUE_FONT, a);
        text.writeStringOver(posX-VALUE_FONT.width/2-a,posY-VALUE_FONT.height/2,str,VALUE_FONT,color,display.COLOR_BACK,0);
    }
    else if ((val%(this->stepValueBig))==0){
        a = text.decToString(str, val);
        a = getCorrectXDigGrid(pos, VALUE_FONT, a);
        text.writeStringOver(posX-VALUE_FONT.width/2-a,posY-VALUE_FONT.height/2,str,VALUE_FONT,color,display.COLOR_BACK,0);
    }
    else if ((val%(this->stepValueSmall))==0 || pos==0 || pos==(slider.VALUE_LEN-1) || values[pos] == maxValue){
        a = text.decToString(str, val);
        a = getCorrectXDigGrid(pos, VALUE_SMALL_FONT, a);
        text.writeStringOver(posX-VALUE_SMALL_FONT.width/2-a,posY-VALUE_SMALL_FONT.height/2,str,VALUE_SMALL_FONT,color,display.COLOR_BACK,0);
    }
}
//-----------------------------------------------------
uint8_t Slider::viewGrid(uint16_t pos_new, uint32_t N)
{
    uint32_t addr=0;
    uint16_t w, h;
    uint8_t data[11];
    uint32_t c;
    uint16_t x, y;
    int16_t i;
    float dx, dy;
    int16_t pos;
    int16_t a, b;
    uint32_t colors[3];
    
    if (screen_visible == SCREEN_VISIBLE_SLEEP) return 0x01;
    
    while(N--){
        memory.readPage(data, addr, 11);
        if (data[0]==0xCD && data[1]==0xBA && data[2]==0x00){
            w = data[4]*256+data[3];
            h = data[6]*256+data[5];
            addr += w*h*3+11;
        }
        else return 0x01;
    }
    
    memory.readPage(data, addr, 11);
    if (data[0]==0xCD && data[1]==0xBA && data[2]==0x00){
        w = data[4]*256+data[3];
        h = data[6]*256+data[5];
        addr += 11;
    }
    else return 0x01;
    
    a = pos_new - this->positionTempOld;
    b = 1;
    if (a < 0){
        a = -a;
        b = -1;
    }
    pos = this->positionTempOld;
    if (this->change <= 0){
        this->change = 1;
        a += SLIDER_CHANGE_CLEAR;
        if (b>0){
            pos -= SLIDER_CHANGE_CLEAR;
        }
        else{
            //pos += SLIDER_CHANGE_CLEAR;
        }
    }
    while (a--)
    {
        pos += b;
        if (pos>=slider.VALUE_LEN || pos<0) continue;
        dx = (float)(this->valueX[pos] - this->x[pos]) / DELTA_RADIUS_ALL;
        dy = (float)(this->valueY[pos] - this->y[pos]) / DELTA_RADIUS_ALL;
        for (i=SLIDER_CLEAR_MIN; i<SLIDER_CLEAR_MAX; i++){
            
            x = this->x[pos] - (dx * (SLIDER_CLEAR_MAX/2)) + (dx * i);
            y = this->y[pos] - (dy * (SLIDER_CLEAR_MAX/2)) + (dy * i);
            
            c = addr+((x-IMAGE_BCK_X)*w*3+(y-IMAGE_BCK_Y)*3);
            MEM_CS_ON;
            memory.quadOutInit();
            memory.quadSend(W25_FAST_READ_QUAD);
            memory.quadSend((c >> 16) & 0xFF);
            memory.quadSend((c >> 8) & 0xFF);
            memory.quadSend(c & 0xFF);
            memory.quadSend(0x00);
            memory.quadInInit();
            data[0] = memory.quadReceiv();
            data[1] = memory.quadReceiv();
            data[2] = memory.quadReceiv();
            colors[0] = (data[0]<<16)+(data[1]<<8)+data[2];
            
            data[0] = memory.quadReceiv();
            data[1] = memory.quadReceiv();
            data[2] = memory.quadReceiv();
            colors[1] = (data[0]<<16)+(data[1]<<8)+data[2];
            
            //data[0] = memory.quadReceiv();
            //data[1] = memory.quadReceiv();
            //data[2] = memory.quadReceiv();
            //colors[2] = (data[0]<<16)+(data[1]<<8)+data[2];
            
            canvas.writePixelCount(x, y, colors, 2);
            
            MEM_CS_OFF;
        }
    }
    this->positionTempOld = pos_new-1;
    
    return 0x00;
}
//-----------------------------------------------------
uint8_t Slider::viewGridInvert(uint16_t pos_new, uint32_t N)
{
    uint32_t addr=0;
    uint16_t w, h;
    uint8_t data[11];
    uint32_t c;
    uint16_t x, y;
    int16_t i;
    float dx, dy;
    int16_t pos;
    int16_t a, b;
    uint32_t colors[3];
    
    if (screen_visible == SCREEN_VISIBLE_SLEEP) return 0x01;
    
    while(N--){
        memory.readPage(data, addr, 11);
        if (data[0]==0xCD && data[1]==0xBA && data[2]==0x00){
            w = data[4]*256+data[3];
            h = data[6]*256+data[5];
            addr += w*h*3+11;
        }
        else return 0x01;
    }
    
    memory.readPage(data, addr, 11);
    if (data[0]==0xCD && data[1]==0xBA && data[2]==0x00){
        w = data[4]*256+data[3];
        h = data[6]*256+data[5];
        addr += 11;
    }
    else return 0x01;
    
    a = pos_new - this->positionTempOld;
    b = 1;
    if (a < 0){
        a = -a;
        b = -1;
    }
    pos = this->positionTempOld;
    if (this->change >= 0){
        this->change = -1;
        a += SLIDER_CHANGE_CLEAR;
        if (b>0){
        //    pos -= SLIDER_CHANGE_CLEAR;
        }
        else{
            pos += SLIDER_CHANGE_CLEAR;
        }
    }
    while (a--)
    {
        pos += b;
        if (pos>=slider.VALUE_LEN || pos<0) continue;
        dx = (float)(this->valueX[pos] - this->x[pos]) / 30.0f;
        dy = (float)(this->valueY[pos] - this->y[pos]) / 30.0f;
        for (i=SLIDER_CLEAR_MIN; i<SLIDER_CLEAR_MAX; i++){
            
            x = this->x[pos] - (dx * (SLIDER_CLEAR_MAX/2)) + (dx * i);
            y = this->y[pos] - (dy * (SLIDER_CLEAR_MAX/2)) + (dy * i);
            
            c = addr+((x-IMAGE_BCK_X)*w*3+(y-IMAGE_BCK_Y)*3);
            MEM_CS_ON;
            memory.quadOutInit();
            memory.quadSend(W25_FAST_READ_QUAD);
            memory.quadSend((c >> 16) & 0xFF);
            memory.quadSend((c >> 8) & 0xFF);
            memory.quadSend(c & 0xFF);
            memory.quadSend(0x00);
            memory.quadInInit();
            data[0] = memory.quadReceiv();
            data[1] = memory.quadReceiv();
            data[2] = memory.quadReceiv();
            colors[0] = SLIDER_COLOR_COLD;
            
            data[0] = memory.quadReceiv();
            data[1] = memory.quadReceiv();
            data[2] = memory.quadReceiv();
            colors[1] = SLIDER_COLOR_COLD;
            
            //data[0] = memory.quadReceiv();
            //data[1] = memory.quadReceiv();
            //data[2] = memory.quadReceiv();
            //colors[2] = SLIDER_COLOR_COLD;
            
            canvas.writePixelCount(x, y, colors, 2);
            
            MEM_CS_OFF;
        }
    }
    this->positionTempOld = pos_new+1;
    
    return 0x00;
}
//-----------------------------------------------------
bool Slider::viewGridSens(uint16_t pos_new, uint32_t N, bool isClear)
{
    static bool isFirst = true;
    static uint32_t addr;
    static uint16_t w, h;
    static uint8_t data[11];
    static uint32_t c, d;
    static uint16_t x, y;
    static int16_t i;
    static float dx, dy;
    static int16_t pos;
    static int16_t a, b;
    
    if (isClear == true) isFirst = true;
    if (isFirst == true){
        addr=0;
        isFirst = false;
        while(N--){
            memory.readPage(data, addr, 11);
            if (data[0]==0xCD && data[1]==0xBA && data[2]==0x00){
                w = data[4]*256+data[3];
                h = data[6]*256+data[5];
                addr += w*h*3+11;
            }
            else return 0x01;
        }
        
        memory.readPage(data, addr, 11);
        if (data[0]==0xCD && data[1]==0xBA && data[2]==0x00){
            w = data[4]*256+data[3];
            h = data[6]*256+data[5];
            addr += 11;
        }
        else return 0x01;
        
        a = pos_new - this->positionTempOld;
        b = 1;
        if (a < 0){
            a = -a;
            b = -1;
        }
        pos = this->positionTempOld;
        if (this->change <= 0){
            this->change = 1;
            a += SLIDER_CHANGE_CLEAR;
            if (b>0){
                pos -= SLIDER_CHANGE_CLEAR;
            }
            else{
                //pos += SLIDER_CHANGE_CLEAR;
            }
        }
    }
    
    while (a--)
    {
        pos += b;
        if (pos>=slider.VALUE_LEN || pos<0) continue;
        dx = (float)(this->valueX[pos] - this->x[pos]) / DELTA_RADIUS_ALL;
        dy = (float)(this->valueY[pos] - this->y[pos]) / DELTA_RADIUS_ALL;
        for (i=SLIDER_CLEAR_MIN; i<SLIDER_CLEAR_MAX; i++){
            
            x = this->x[pos] - (dx * (SLIDER_CLEAR_MAX/2)) + (dx * i);
            y = this->y[pos] - (dy * (SLIDER_CLEAR_MAX/2)) + (dy * i);
            
            c = addr+((x-IMAGE_BCK_X)*w*3+(y-IMAGE_BCK_Y)*3);
            MEM_CS_ON;
            memory.quadOutInit();
            memory.quadSend(W25_FAST_READ_QUAD);
            memory.quadSend((c >> 16) & 0xFF);
            memory.quadSend((c >> 8) & 0xFF);
            memory.quadSend(c & 0xFF);
            memory.quadSend(0x00);
            memory.quadInInit();
            data[0] = memory.quadReceiv();
            data[1] = memory.quadReceiv();
            data[2] = memory.quadReceiv();
            c = (data[0]<<16)+(data[1]<<8)+data[2];
            
            data[0] = memory.quadReceiv();
            data[1] = memory.quadReceiv();
            data[2] = memory.quadReceiv();
            d = (data[0]<<16)+(data[1]<<8)+data[2];
            canvas.writePixelDouble(x, y, c, d);
            
            MEM_CS_OFF;
        }
        return false;
    }
    this->positionTempOld = pos_new-1;
    
    isFirst = true;
    return true;
}
//-----------------------------------------------------
void Slider::setVal(uint16_t pos)
{
    this->positionTempOld = pos;
}
//-----------------------------------------------------
void Slider::draw(uint16_t x, uint16_t y, uint8_t z, char* txt, uint16_t value, char unit, bool isDrawText, bool* isValue, bool isVisible)
{
    uint8_t n;
    char str[8];
    
    if (isDrawText == TRUE) text.writeString(x,y,txt,Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
    if (*isValue == TRUE){
        *isValue = FALSE;
        canvas.drawSlider(x,y,z,isVisible);
        for (n=0; n<8; n++) str[n] = 0;
        n = 0;
        n += text.decToString(&str[n], value);
        if (unit != NULL) str[n++] = unit; 
        str[n++] = ' '; str[n++] = ' ';
        text.writeString(255,y+21,str,Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
    }
}
//-----------------------------------------------------
int16_t Slider::drawSlim(uint16_t x, uint16_t y, float z, const char* txt, uint16_t value, char unit, bool isDrawText, bool* isValue, bool isVisible, uint32_t color, uint8_t id)
{
    uint8_t n;
    uint32_t x_pos;
    char str[8];
    int16_t result = -3;
    static uint8_t modeBtn=0;
    
    const int SLIDER_H = 50;
    const int SLIDER_W = 200;
    
    if (isDrawText == TRUE) text.writeString(x,y,txt,Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
    x = 35;
    if (*isValue == TRUE){
        *isValue = FALSE;
        canvas.drawSliderSlim(x,y,z,isVisible, color);
        
        canvas.writeMinus(x-10, y+29);
        canvas.writePlus(x+235, y+29);
        
        for (n=0; n<8; n++) str[n] = 0;
        n = 0;
        str[n++] = ' ';
        n += text.decToString(&str[n], value);
        if (unit != NULL) str[n++] = unit; 
        x_pos = 320 - (Font_11x18.width * n + 2);
        text.writeString(x_pos,y+21,str,Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
    }
    
    if (sensor.status == 1){   // касание сенсорного экрана
        if ((sensor.x1>=x && sensor.x1<=(x+SLIDER_W) && sensor.y1>=y && sensor.y1<=(y+SLIDER_H) && sensor.touch==0) || sensor.touch==id){ // касание ползунка
            sensor.touch = id;
            result = sensor.x1-x;
        }
        if ((sensor.x1<x && sensor.y1>=y && sensor.y1<=(y+SLIDER_H) && sensor.touch==0) || sensor.touch==(id|0x80)){ // касание декремента
            sensor.touch = (id|0x80);
            if (core.getTimer() == 0){
                if (modeBtn == 0) core.setTimer(500);
                modeBtn = 1;
                result = -1;
            }
        }  
        if ((sensor.x1>(x+SLIDER_W) && sensor.y1>=y && sensor.y1<=(y+SLIDER_H) && sensor.touch==0) || sensor.touch==(id|0x40)){ // касание инкремента
            sensor.touch = (id|0x40);
            if (core.getTimer() == 0){
                if (modeBtn == 0) core.setTimer(500);
                modeBtn = 1;
                result = -2;
            }
        }  
    }
    else{
        modeBtn = 0;
        sensor.touch = 0;
    }
    
    return result;
}
//-----------------------------------------------------
int16_t Slider::drawSlim(uint16_t x, uint16_t y, float z, const char* txt, bool isDrawText, bool* isValue, bool isVisible, uint32_t color, uint8_t id)
{
    int16_t result = -3;
    static uint8_t modeBtn=0;
    
    const int SLIDER_H = 50;
    const int SLIDER_W = 200;
    
    if (isDrawText == TRUE) text.writeString(x,y,txt,Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
    x = 35;
    if (*isValue == TRUE){
        *isValue = FALSE;
        canvas.drawSliderSlim(x,y,z,isVisible, color);
        
        canvas.writeMinus(x-10, y+29);
        canvas.writePlus(x+235, y+29);
        
        canvas.writeFillRect(320-10*4,y+21,10*4,18,0);
    }
    
    if (sensor.status == 1){   // касание сенсорного экрана
        if ((sensor.x1>=x && sensor.x1<=(x+SLIDER_W) && sensor.y1>=y && sensor.y1<=(y+SLIDER_H) && sensor.touch==0) || sensor.touch==id){ // касание ползунка
            sensor.touch = id;
            result = sensor.x1-x;
        }
        if ((sensor.x1<x && sensor.y1>=y && sensor.y1<=(y+SLIDER_H) && sensor.touch==0) || sensor.touch==(id|0x80)){ // касание декремента
            sensor.touch = (id|0x80);
            if (core.getTimer() == 0){
                if (modeBtn == 0) core.setTimer(500);
                modeBtn = 1;
                result = -1;
            }
        }  
        if ((sensor.x1>(x+SLIDER_W) && sensor.y1>=y && sensor.y1<=(y+SLIDER_H) && sensor.touch==0) || sensor.touch==(id|0x40)){ // касание инкремента
            sensor.touch = (id|0x40);
            if (core.getTimer() == 0){
                if (modeBtn == 0) core.setTimer(500);
                modeBtn = 1;
                result = -2;
            }
        }  
    }
    else{
        modeBtn = 0;
        sensor.touch = 0;
    }
    
    return result;
}
//-----------------------------------------------------
void Slider::drawDouble(uint16_t x, uint16_t y, uint8_t z1, uint8_t z2, const char* txt, bool isDrawText, bool* isValue, uint8_t numSelectSlider)
{
    //uint8_t n;
    //char str[8];
    
    if (isDrawText == TRUE) text.writeString(x,y,txt,Font_11x18,display.WHITE,display.COLOR_BACK);
    if (*isValue == TRUE){
        *isValue = FALSE;
        canvas.drawSliderSlimDouble(x,y+40,z1,z2,numSelectSlider);
        /*for (n=0; n<8; n++) str[n] = 0;
        n = 0;
        n += text.decToString(&str[n], z1);
        str[n++] = ':'; 
        str[n++] = ' '; str[n++] = ' ';
        text.writeString(x,y+21,str,Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);*/
    }
}
//-----------------------------------------------------
void Slider::draw(uint16_t x, uint16_t y, uint8_t z, const char* txt, bool isDrawText, bool* isValue, bool isVisible)
{
    if (isDrawText == TRUE) text.writeString(x,y,txt,Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
    if (*isValue == TRUE){
        *isValue = FALSE;
        canvas.drawSlider(x,y,z,isVisible);
        text.writeString(255,y+21,"    ",Font_11x18,display.COLOR_TEXT,display.COLOR_BACK);
    }
}
//-----------------------------------------------------
bool Slider::setMinMax(int16_t offValue, uint8_t minValue, uint8_t maxValue, uint8_t smallStep, uint8_t bigStep)
{
    bool result;
    int32_t x, a;
    char str[7];
    
    result = (this->offValue != offValue);
    result |= (this->minValue != minValue);
    result |= (this->maxValue != maxValue);
    result |= (this->stepValueSmall != smallStep);
    result |= (this->stepValueBig != bigStep);
    if (screen_visible == SCREEN_VISIBLE_SLEEP) return result;
    
    if (result == TRUE){
        for (x=0; x<6; x++) str[x] = 0;
        a = text.decToString(str, this->values[this->positionTemp]);
        if (display.setup.celsius){
            if (this->positionTemp < (this->VALUE_LEN/2)) a=SLIDER_FONT.width*(a-1);
            else a = 0;
        }
        else{
            if (this->positionTemp < (this->VALUE_LEN/2)) a=VALUE_FONT.width*(a-1);
            else a = 0;
        }
        if (display.setup.celsius){
            text.writeStringOver(this->valueX[this->positionTemp]-SLIDER_FONT.width/2-a,this->valueY[this->positionTemp]-SLIDER_FONT.height/2,str,SLIDER_FONT,display.COLOR_BACK,display.COLOR_BACK,0);
        }
        else{
            text.writeStringOver(this->valueX[this->positionTemp]-VALUE_FONT.width/2-a,this->valueY[this->positionTemp]-VALUE_FONT.height/2,str,VALUE_FONT,display.COLOR_BACK,display.COLOR_BACK,0);
        }
        this->eraseDigGrid();
    }
    
    this->offValue = offValue;
    this->minValue = minValue;
    this->maxValue = maxValue;
    this->stepValueSmall = smallStep;
    this->stepValueBig = bigStep;
    
    if (result){
        this->initialize();
        this->drawDigGrid();   
        this->positionTemp = VALUE_LEN-1;
        this->old = this->position;
        this->setVal(this->positionTemp);
    }
    
    return result;
}
//-----------------------------------------------------
