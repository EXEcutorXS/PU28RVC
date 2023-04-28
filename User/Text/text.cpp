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
#include "text.h"
#include "display.h"
#include "canvas.h"
#include "font.h"
#include "memory.h"

Text text;
//-----------------------------------------------------
Text::Text(void)
{
    
}
//-----------------------------------------------------
void Text::writeChar(uint16_t x, uint16_t y, char ch, FontDef font, uint32_t color, uint32_t bgcolor)
{
    uint32_t b;
    int32_t i, j;

    display.setAddrWindow(x, y, font.width, font.height);
    DISP_cs_on;
    for(i = (font.width-1); i >= 0; i--) {
        for(j = 0; j < font.height; j++) {
            b = font.data[(ch - 32) * font.height + j];
            if((b << i) & 0x8000)  {
                display.writeColorFast(color);
            } else {
                display.writeColorFast(bgcolor);
            }
        }
    }
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
    DISP_cs_off;
}
//-----------------------------------------------------
void Text::writeCharOver(uint16_t x, uint16_t y, char ch, FontDef font, uint32_t color, uint32_t bgcolor, uint8_t mask)
{
    uint32_t b;
    int32_t i, j;

    ///display.setAddrWindow(x, y, font.width, font.height);
    ///DISP_cs_on;
    for(i = (font.width-1); i >= 0; i--) {
        for(j = 0; j < font.height; j++) {
            b = font.data[(ch - 32) * font.height + j];
            if((b << i) & 0x8000)  {
                if (mask == 1){
                    if ((x+i)&0x01 || (y+j)&0x01) continue;
                }
                if (mask == 2){
                    if (!((x+i)&0x01 || (y+j)&0x01)) continue;
                }
                display.setAddrWindow(x+i,y+j,1,1);
                display.writeColor(color);
            }
        }
    }
    ///while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
    ///DISP_cs_off;
}
//-----------------------------------------------------
void Text::writeString(uint16_t x, uint16_t y, const char* str, FontDef font, uint32_t color, uint32_t bgcolor)
{
    while(*str) {
        if(x + font.width >= display.WIDTH) {
            x = 0;
            y += font.height;
            if(y + font.height >= display.HEIGHT) {
                break;
            }

            if(*str == ' ') {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }

        this->writeChar(x, y, *str, font, color, bgcolor);
        x += font.width-1;
        str++;
    }
}
//-----------------------------------------------------
void Text::writeStringCenter(uint16_t x, uint16_t y, const char* str, FontDef font, uint32_t color, uint32_t bgcolor)
{
    x = x - getLenString(str)*font.width/2;
    while(*str) {
        if(x + font.width >= display.WIDTH) {
            x = 0;
            y += font.height;
            if(y + font.height >= display.HEIGHT) {
                break;
            }

            if(*str == ' ') {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }

        writeChar(x, y, *str, font, color, bgcolor);
        x += font.width;
        //x += font.width;
        str++;
    }
}
//-----------------------------------------------------
uint8_t Text::getLenString(const char* str)
{
    uint8_t a = 0;
    
    while (*str != 0){
        str++;
        a++;
    }
    return a;
}
//-----------------------------------------------------
void Text::writeDigit(uint16_t x, uint16_t y, const char* str, uint32_t color, uint8_t isOver)
{
    while(*str) {
        writeOneDigit(x, y, *str, color, isOver);
        if (*str == 'A' || *str == 'P'){
            x += 26;
        }
        else{
            x += 16;
        }
        str++;
    }
}
//-----------------------------------------------------
void Text::writeOneDigit(uint16_t x, uint16_t y, char str, uint32_t color, uint8_t isOver)
{
    if(x + 16 >= display.WIDTH) {
        x = 0;
        y += 26;
        if(y + 26 >= display.HEIGHT) {
            return;
        }
    }
    
    switch(str){
        case ' ':
            canvas.writeFillRect(x,y,16,26,0);
            break;
        case '0':
            if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_0,color);
            else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_0,STEP_ZERO,1,color);
            else if (isOver == 2){
                canvas.writeFillRect(x,y,16,26,0);
                canvas.loadImageColor(x,y,FONT_16x26_0,color);
            }
            break;
        case '1':
            if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_1,color);
            else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_1,STEP_ZERO,1,color);
            else if (isOver == 2){
                canvas.writeFillRect(x,y,16,26,0);
                canvas.loadImageColor(x,y,FONT_16x26_1,color);
            }
            break;
        case '2':
            if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_2,color);
            else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_2,STEP_ZERO,1,color);
            else if (isOver == 2){
                canvas.writeFillRect(x,y,16,26,0);
                canvas.loadImageColor(x,y,FONT_16x26_2,color);
            }
            break;
        case '3':
            if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_3,color);
            else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_3,STEP_ZERO,1,color);
            else if (isOver == 2){
                canvas.writeFillRect(x,y,16,26,0);
                canvas.loadImageColor(x,y,FONT_16x26_3,color);
            }
            break;
        case '4':
            if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_4,color);
            else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_4,STEP_ZERO,1,color);
            else if (isOver == 2){
                canvas.writeFillRect(x,y,16,26,0);
                canvas.loadImageColor(x,y,FONT_16x26_4,color);
            }
            break;
        case '5':
            if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_5,color);
            else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_5,STEP_ZERO,1,color);
            else if (isOver == 2){
                canvas.writeFillRect(x,y,16,26,0);
                canvas.loadImageColor(x,y,FONT_16x26_5,color);
            }
            break;
        case '6':
            if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_6,color);
            else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_6,STEP_ZERO,1,color);
            else if (isOver == 2){
                canvas.writeFillRect(x,y,16,26,0);
                canvas.loadImageColor(x,y,FONT_16x26_6,color);
            }
            break;
        case '7':
            if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_7,color);
            else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_7,STEP_ZERO,1,color);
            else if (isOver == 2){
                canvas.writeFillRect(x,y,16,26,0);
                canvas.loadImageColor(x,y,FONT_16x26_7,color);
            }
            break;
        case '8':
            if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_8,color);
            else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_8,STEP_ZERO,1,color);
            else if (isOver == 2){
                canvas.writeFillRect(x,y,16,26,0);
                canvas.loadImageColor(x,y,FONT_16x26_8,color);
            }
            break;
        case '9':
            if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_9,color);
            else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_9,STEP_ZERO,1,color);
            else if (isOver == 2){
                canvas.writeFillRect(x,y,16,26,0);
                canvas.loadImageColor(x,y,FONT_16x26_9,color);
            }
            break;
        case 'A':
            canvas.writeFillRect(x,y,26,26,0);
            text.writeString(x+2,y+6,"am",Font_11x18,color,display.COLOR_BACK);
            break;
        case 'P':
            canvas.writeFillRect(x,y,26,26,0);
            text.writeString(x+2,y+6,"pm",Font_11x18,color,display.COLOR_BACK);
            break;
        case 'C':
            if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_C,color);
            else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_C,STEP_ZERO,1,color);
            else if (isOver == 2){
                canvas.writeFillRect(x,y,16,26,0);
                canvas.loadImageColor(x,y,FONT_16x26_C,color);
            }
            break;
        case 'F':
            if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_F,color);
            else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_F,STEP_ZERO,1,color);
            else if (isOver == 2){
                canvas.writeFillRect(x,y,16,26,0);
                canvas.loadImageColor(x,y,FONT_16x26_F,color);
            }
            break;
        case 'v':
            if (false){
                if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_V,color);
                else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_V,STEP_ZERO,1,color);
                else if (isOver == 2){
                    canvas.writeFillRect(x,y,16,26,0);
                    canvas.loadImageColor(x,y,FONT_16x26_V,color);
                }
            }
            else{
                if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_V,color);
                else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_V,STEP_ZERO,1,color);
                else if (isOver == 2){
                    canvas.writeFillRect(x,y,16,26,0);
                    canvas.loadImageColor(x,y,FONT_16x26_V,color);
                }
            }
            break;
        case '-':
            if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_DASH,color);
            else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_DASH,STEP_ZERO,1,color);
            else if (isOver == 2){
                canvas.writeFillRect(x,y,16,26,0);
                canvas.loadImageColor(x,y,FONT_16x26_DASH,color);
            }
            break;
        case '.':
            if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_DOT,color);
            else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_DOT,STEP_ZERO,1,color);
            else if (isOver == 2){
                canvas.writeFillRect(x,y,16,26,0);
                canvas.loadImageColor(x,y,FONT_16x26_DOT,color);
            }
            break;
        case ':':
            if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_COL,color);
            else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_COL,STEP_ZERO,1,color);
            else if (isOver == 2){
                canvas.writeFillRect(x,y,16,26,0);
                canvas.loadImageColor(x,y,FONT_16x26_COL,color);
            }
            break;
        case '~':
            if (isOver == 0) canvas.loadImageColor(x,y,FONT_16x26_DEG,color);
            else if (isOver == 1) canvas.loadImageOverColor(x,y,FONT_16x26_DEG,STEP_ZERO,1,color);
            else if (isOver == 2){
                canvas.writeFillRect(x,y,16,26,0);
                canvas.loadImageColor(x,y,FONT_16x26_DEG,color);
            }
            break;
    }
}
//-----------------------------------------------------
void Text::writeStringOver(int16_t x, int16_t y, char* str, FontDef font, uint32_t color, uint32_t bgcolor, uint8_t mask)
{
    if (y < 0) y = 0;
    if (x < 0) x = 0;
    while(*str) {
        if(x + font.width >= display.WIDTH) {
            x = 0;
            y += font.height;
            if(y + font.height >= display.HEIGHT) {
                break;
            }

            if(*str == ' ') {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }

        this->writeCharOver(x, y, *str, font, color, bgcolor, mask);
        x += font.width;
        str++;
    }
}
//-----------------------------------------------------
void Text::writeStringEnd(uint16_t x, uint16_t y, char* str, FontDef font, uint32_t color, uint32_t bgcolor)
{
    int i;
    
    i=1;
    while(x > 0){
        if (str[i++] == 0) break;
        x -= font.width;
    }
    this->writeString(x, y, str, font, color, bgcolor);
}
//-----------------------------------------------------
uint8_t Text::decToString(char* str, int32_t D)
{
    uint32_t a1, a2, a3, a4, a5, a6, a7, a8, a9;
	uint32_t c1, c2, c3, c4, c5, c6, c7, c8;
    uint8_t n;
	
    n=0;
    if (D < 0){
        *str++='-';
        n++;
        D = -D;
    }
	a1 = (uint8_t)(D/100000000);
	c1 = a1*100000000;
	a2 = (uint8_t)((D-c1)/10000000);
	c2 = a2*10000000;
	a3 = (uint8_t)((D-(c1+c2))/1000000);
	c3 = a3*1000000;
	a4 = (uint8_t)((D-(c1+c2+c3))/100000);
	c4 = a4*100000;
	a5 = (uint8_t)((D-(c1+c2+c3+c4))/10000);
	c5 = a5*10000;
	a6 = (uint8_t)((D-(c1+c2+c3+c4+c5))/1000);
	c6 = a6*1000;
	a7 = (uint8_t)((D-(c1+c2+c3+c4+c5+c6))/100);
	c7 = a7*100;
	a8 = (uint8_t)((D-(c1+c2+c3+c4+c5+c6+c7))/10);
	c8 = a8*10;
	a9 = D-(c1+c2+c3+c4+c5+c6+c7+c8);
	if (a1>0)
	{
		*str++=(a1+0x30);
        n++;
	}
	if (a2>0 || a1>0)
	{
		*str++=(a2+0x30);
        n++;
	}
	if (a3>0 || a2>0 || a1>0)
	{
		*str++=(a3+0x30);
        n++;
	}
	if (a4>0 || a3>0 || a2>0 || a1>0)
	{
		*str++=(a4+0x30);
        n++;
	}
	if (a5>0 || a4>0 || a3>0 || a2>0 || a1>0)
	{
		*str++=(a5+0x30);
        n++;
	}
	if (a6>0 || a5>0 || a4>0 || a3>0 || a2>0 || a1>0)
	{
		*str++=(a6+0x30);
        n++;
	}
	if (a7>0 || a6>0 || a5>0 || a4>0 || a3>0 || a2>0 || a1>0)
	{
		*str++=(a7+0x30);
        n++;
	}
	if (a8>0 || a7>0 || a6>0 || a5>0 || a4>0 || a3>0 || a2>0 || a1>0)
	{
		*str++=(a8+0x30);
        n++;
	}
	//if (a5>0 || a4>0 || a3>0 || a2>0 || a1>0)
	*str++=(a9+0x30);
    n++;
    
    return n;
}
//-----------------------------------------------------
uint8_t Text::decToStringFix(char* str, int32_t D, uint8_t i)
{
    uint32_t a1, a2, a3, a4, a5, a6, a7, a8, a9;
	uint32_t c1, c2, c3, c4, c5, c6, c7, c8;
    uint8_t n;
	
    n=0;
    if (D < 0){
        *str++='-';
        n++;
        D = -D;
    }
	a1 = (uint8_t)(D/100000000);
	c1 = a1*100000000;
	a2 = (uint8_t)((D-c1)/10000000);
	c2 = a2*10000000;
	a3 = (uint8_t)((D-(c1+c2))/1000000);
	c3 = a3*1000000;
	a4 = (uint8_t)((D-(c1+c2+c3))/100000);
	c4 = a4*100000;
	a5 = (uint8_t)((D-(c1+c2+c3+c4))/10000);
	c5 = a5*10000;
	a6 = (uint8_t)((D-(c1+c2+c3+c4+c5))/1000);
	c6 = a6*1000;
	a7 = (uint8_t)((D-(c1+c2+c3+c4+c5+c6))/100);
	c7 = a7*100;
	a8 = (uint8_t)((D-(c1+c2+c3+c4+c5+c6+c7))/10);
	c8 = a8*10;
	a9 = D-(c1+c2+c3+c4+c5+c6+c7+c8);
	if (a1>0 || i>8)
	{
		*str++=(a1+0x30);
        n++;
	}
	if (a2>0 || a1>0 || i>7)
	{
		*str++=(a2+0x30);
        n++;
	}
	if (a3>0 || a2>0 || a1>0 || i>6)
	{
		*str++=(a3+0x30);
        n++;
	}
	if (a4>0 || a3>0 || a2>0 || a1>0 || i>5)
	{
		*str++=(a4+0x30);
        n++;
	}
	if (a5>0 || a4>0 || a3>0 || a2>0 || a1>0 || i>4)
	{
		*str++=(a5+0x30);
        n++;
	}
	if (a6>0 || a5>0 || a4>0 || a3>0 || a2>0 || a1>0 || i>3)
	{
		*str++=(a6+0x30);
        n++;
	}
	if (a7>0 || a6>0 || a5>0 || a4>0 || a3>0 || a2>0 || a1>0 || i>2)
	{
		*str++=(a7+0x30);
        n++;
	}
	if (a8>0 || a7>0 || a6>0 || a5>0 || a4>0 || a3>0 || a2>0 || a1>0 || i>1)
	{
		*str++=(a8+0x30);
        n++;
	}
	//if (a5>0 || a4>0 || a3>0 || a2>0 || a1>0)
	*str++=(a9+0x30);
    n++;
    
    return n;
}
//-----------------------------------------------------
uint8_t Text::floatToString (char* str, float X, uint8_t N)
{
	uint32_t A;
    uint8_t n, m;
	
    m = 1;
	if (X < 0)
	{
		*str++=('-');
        m++;
		X = -X;
	}
	A = X;
	n = this->decToString(str, A);
	str += n;
	*str++=('.');
	
	X = X-A;
	if (N == 1) A = (X+0.05f)*10.0f;
	if (N == 2) A = (X+0.005f)*100.0f;
	if (N == 3) A = (X+0.0005f)*1000.0f;
	if (N == 4) A = (X+0.00005f)*10000.0f;
	if (N == 5) A = (X+0.000005f)*100000.0f;
	if (N == 6) A = (X+0.0000005f)*1000000.0f;
	if (N == 7) A = (X+0.00000005f)*10000000.0f;
	if (N == 8) A = (X+0.000000005f)*100000000.0f;
	this->decToString(str, A);
    
    return n + m + N;
}
//-----------------------------------------------------
uint8_t Text::charToString (char* str, const char* chr)
{
    uint8_t n = 0;
    
    while(1){
        *str = *chr;
        if (*chr == 0) break;
        str++;
        chr++;
        n++;
    }
    return n; 
}
//-----------------------------------------------------
