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
#include "canvas.h"
#include "display.h"
#include "memory.h"
#include <math.h>

Canvas canvas;
//-----------------------------------------------------
Canvas::Canvas(void)
{
    
}
//-----------------------------------------------------
void Canvas::writePixel(int16_t x, int16_t y, uint32_t color)
{
    display.setAddrWindow(x,y,1,1);
    display.writeColor(color);
}
//-----------------------------------------------------
void Canvas::writePixelZoom(int16_t x, int16_t y, uint8_t zoom, uint32_t color)
{
    uint16_t z;
    
    display.setAddrWindow(x,y,zoom,zoom);
    DISP_cs_on;
    for (z=0; z<(zoom*zoom); z++){
        display.writeColorFast(color);
    }
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
    DISP_cs_off;
    //*/
    /*
    DISP_cs_on;
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData16(SPI1, 0x100 + (color>>16)&0xFF);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData16(SPI1, 0x100 + (color>>8)&0xFF);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData16(SPI1, 0x100 + color&0xFF);
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
    DISP_cs_off;
    */
}
//-----------------------------------------------------
void Canvas::writePixelDouble(int16_t x, int16_t y, uint32_t color1, uint32_t color2)
{
    display.setAddrWindow(x,y,1,2);
    DISP_cs_on;
    display.writeColorFast(color1);
    display.writeColorFast(color2);
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
    DISP_cs_off;
}
//-----------------------------------------------------
void Canvas::writePixelCount(int16_t x, int16_t y, uint32_t *color, uint8_t count)
{
    display.setAddrWindow(x,y,1,count);
    DISP_cs_on;
    while (count--){
        display.writeColorFast(*color++);
    }
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
    DISP_cs_off;
}
//-----------------------------------------------------
void Canvas::writeFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color)
{
    uint32_t i;
    uint32_t xa, ya;
    uint32_t len;
    
    x = display.WIDTH-x;
    xa = ((uint32_t)y << 16) | (y+h-1);
    ya = ((uint32_t)(x-w) << 16) | (x-1);
    display.writeCommand(ST7789H2_CASET); // Column addr set
    display.writeData((xa>>24)&0xFF);
    display.writeData((xa>>16)&0xFF);
    display.writeData((xa>>8)&0xFF);
    display.writeData(xa&0xFF);
    display.writeCommand(ST7789H2_PASET); // Row addr set
    display.writeData((ya>>24)&0xFF);
    display.writeData((ya>>16)&0xFF);
    display.writeData((ya>>8)&0xFF);
    display.writeData(ya&0xFF);
    display.writeCommand(ST7789H2_RAMWR); // write to RAM
    len = w*h;
    DISP_cs_on;
    for (i=0; i<len; i++){
        /*
        while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
        SPI_I2S_SendData16(SPI1, 0x100 + ((color>>16)&0xFF));
        while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
        SPI_I2S_SendData16(SPI1, 0x100 + ((color>>8)&0xFF));
        while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
        SPI_I2S_SendData16(SPI1, 0x100 + (color&0xFF));
        */
        display.writeColorFast(color);
    }
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
    DISP_cs_off;
}
//-----------------------------------------------------
void Canvas::writeFillRectEffectPoly(uint16_t X, uint16_t Y, uint16_t w, uint16_t h, uint32_t color, uint32_t S)
{
    uint32_t i;
    int32_t x, y;
    uint32_t rnd=0;
    bool right, left, up, down;
    
    i = h*w;
    while(1){
        for (y=0; y<h; y++){
            for (x=0; x<w; x++){
                rnd++;
                if (rnd > S){
                    rnd = 0;
                    // исключаем углы изображения
                    right = (x > (w-10));
                    left = (x < 10);
                    up = (y > (h-10));
                    down = (y < 10);
                 
                    if (!((right && up) || (right && down) || (left && up) || (left && down))){
                        canvas.writePixel(X+x, Y+y, color);
                    }
                    if (i > 0) i--;
                    else return;
                }
            }
        }
    }
}
//-----------------------------------------------------
void Canvas::writeFillRectEffect(uint16_t X, uint16_t Y, uint16_t w, uint16_t h, uint32_t color, uint32_t S)
{
    uint32_t i;
    int32_t x, y;
    uint32_t rnd=0;
    
    i = h*w;
    while(1){
        for (y=0; y<h; y++){
            for (x=0; x<w; x++){
                rnd++;
                if (rnd > S){
                    rnd = 0;
                    canvas.writePixel(X+x, Y+y, color);
                    if (i > 0) i--;
                    else return;
                }
            }
        }
    }
}
//-----------------------------------------------------
void Canvas::drawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color_back, uint32_t color_side, uint32_t color)
{
    uint32_t xa, ya, a, b, c;
    
    x = display.WIDTH-x;
    xa = ((uint32_t)y << 16) | (y+h-1);
    ya = ((uint32_t)(x-w) << 16) | (x-1);
    display.writeCommand(ST7789H2_CASET); // Column addr set
    display.writeData((xa>>24)&0xFF);
    display.writeData((xa>>16)&0xFF);
    display.writeData((xa>>8)&0xFF);
    display.writeData(xa&0xFF);
    display.writeCommand(ST7789H2_PASET); // Row addr set
    display.writeData((ya>>24)&0xFF);
    display.writeData((ya>>16)&0xFF);
    display.writeData((ya>>8)&0xFF);
    display.writeData(ya&0xFF);
    display.writeCommand(ST7789H2_RAMWR); // write to RAM
    DISP_cs_on;
    for (a=0; a<w; a++){
        for (b=0; b<h; b++){
            
            if (a==0 || b==0 || a==(w-1) || b==(h-1)) c = color_side;
            else c = color;
            
            if (a+b < 3) c = color_side;
            if (w-a-1+b < 3) c = color_side;
            if (h-b-1+a < 3) c = color_side;
            if ((h-b-1)+(w-a-1) < 3) c = color_side;
            
            if (a==0 && b==0) c = color_back;
            if (a==0 && b==(h-1)) c = color_back;
            if (a==(w-1) && b==0) c = color_back;
            if (a==(w-1) && b==(h-1)) c = color_back;
            
            /*
            while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
            SPI_I2S_SendData16(SPI1, 0x100 + ((c>>16)&0xFF));
            while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
            SPI_I2S_SendData16(SPI1, 0x100 + ((c>>8)&0xFF));
            while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
            SPI_I2S_SendData16(SPI1, 0x100 + (c&0xFF));
            */
            display.writeColorFast(c);
        }
    }
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
    DISP_cs_off;
}
//-----------------------------------------------------
void Canvas::drawSlider(int16_t x, int16_t y, uint8_t z, char v)
{
    const int LINE_WIDTH = 2;
    const int LINE_HEIGHT = 18;
    const int SLIDER_WIDTH = 14;
    const int SLIDER_HEIGHT = 20;
    const int SLIDER_COLOR = 0x00E0E0E0;
    
    if (v){
        this->drawRoundRect(x,y+20,LINE_WIDTH*z,LINE_HEIGHT,display.COLOR_BACK,display.COLOR_BACK,0x00C0FF);
        this->drawRoundRect(x+LINE_WIDTH*z+SLIDER_WIDTH,y+20,(LINE_WIDTH*100+SLIDER_WIDTH)-(LINE_WIDTH*z+SLIDER_WIDTH),LINE_HEIGHT,display.COLOR_BACK,display.COLOR_BACK,display.DARKGREY);
        this->drawRoundRect(x+LINE_WIDTH*z,y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2),SLIDER_WIDTH,SLIDER_HEIGHT,display.COLOR_BACK,display.COLOR_BACK,SLIDER_COLOR);
    }
    else{
        this->drawRoundRect(x,y+20,LINE_WIDTH*z,LINE_HEIGHT,display.COLOR_BACK,display.COLOR_BACK,display.DARKGREY);
        this->drawRoundRect(x+LINE_WIDTH*z+SLIDER_WIDTH,y+20,(LINE_WIDTH*100+SLIDER_WIDTH)-(LINE_WIDTH*z+SLIDER_WIDTH),LINE_HEIGHT,display.COLOR_BACK,display.COLOR_BACK,display.DARKGREY);
        this->drawRoundRect(x+LINE_WIDTH*z,y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2),SLIDER_WIDTH,SLIDER_HEIGHT,display.COLOR_BACK,display.COLOR_BACK,SLIDER_COLOR);
    }
}
//-----------------------------------------------------
void Canvas::drawSliderSlim(int16_t x, int16_t y, uint8_t z, bool isActive, uint32_t color)
{
    const float LINE_WIDTH = 2.2;
    const int LINE_HEIGHT = 5;
    const int SLIDER_WIDTH = 8;
    const int SLIDER_HEIGHT = 20;
    
    y += 7;
    if (isActive){
        this->drawRoundRect(x,y+20,LINE_WIDTH*z,LINE_HEIGHT,display.COLOR_BACK,display.COLOR_BACK,color);
        this->drawRoundRect(x+LINE_WIDTH*z+SLIDER_WIDTH,y+20,(LINE_WIDTH*100+SLIDER_WIDTH)-(LINE_WIDTH*z+SLIDER_WIDTH),LINE_HEIGHT,display.COLOR_BACK,display.COLOR_BACK,display.DARKGREY);
        writeFillRect(x-SLIDER_WIDTH/2, y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2), LINE_WIDTH*z+SLIDER_WIDTH/2, (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
        writeFillRect(x-SLIDER_WIDTH/2, y+20+LINE_HEIGHT, LINE_WIDTH*z+SLIDER_WIDTH/2, (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
        writeFillRect(x+LINE_WIDTH*z+SLIDER_WIDTH, y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2), (LINE_WIDTH*100+SLIDER_WIDTH)-(LINE_WIDTH*z+SLIDER_WIDTH/2), (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
        writeFillRect(x+LINE_WIDTH*z+SLIDER_WIDTH, y+20+LINE_HEIGHT, (LINE_WIDTH*100+SLIDER_WIDTH)-(LINE_WIDTH*z+SLIDER_WIDTH/2), (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
        this->drawRoundRect(x+LINE_WIDTH*z,y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2),SLIDER_WIDTH,SLIDER_HEIGHT,display.COLOR_BACK,display.COLOR_BACK,display.LIGHTGREY);
    }
    else{
        this->drawRoundRect(x,y+20,LINE_WIDTH*z,LINE_HEIGHT,display.COLOR_BACK,display.COLOR_BACK,display.DARKGREY);
        this->drawRoundRect(x+LINE_WIDTH*z+SLIDER_WIDTH,y+20,(LINE_WIDTH*100+SLIDER_WIDTH)-(LINE_WIDTH*z+SLIDER_WIDTH),LINE_HEIGHT,display.COLOR_BACK,display.COLOR_BACK,display.DARKGREY);
        writeFillRect(x-SLIDER_WIDTH/2, y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2), LINE_WIDTH*z+SLIDER_WIDTH/2, (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
        writeFillRect(x-SLIDER_WIDTH/2, y+20+LINE_HEIGHT, LINE_WIDTH*z+SLIDER_WIDTH/2, (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
        writeFillRect(x+LINE_WIDTH*z+SLIDER_WIDTH, y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2), (LINE_WIDTH*100+SLIDER_WIDTH)-(LINE_WIDTH*z+SLIDER_WIDTH/2), (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
        writeFillRect(x+LINE_WIDTH*z+SLIDER_WIDTH, y+20+LINE_HEIGHT, (LINE_WIDTH*100+SLIDER_WIDTH)-(LINE_WIDTH*z+SLIDER_WIDTH/2), (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
        this->drawRoundRect(x+LINE_WIDTH*z,y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2),SLIDER_WIDTH,SLIDER_HEIGHT,display.COLOR_BACK,display.COLOR_BACK,display.LIGHTGREY);
    }
}
//-----------------------------------------------------
void Canvas::drawSliderSlimDouble(int16_t x, int16_t y, uint8_t z1, uint8_t z2, uint8_t numSelectSlider)
{
    const float LINE_WIDTH = 2.4;
    const int LINE_HEIGHT = 5;
    const int SLIDER_WIDTH = 8;
    const int SLIDER_HEIGHT = 20;
    const int SLIDER_COLOR = 0x00E0E0E0;
    int colorSlider1 = SLIDER_COLOR, colorSlider2 = SLIDER_COLOR;
    
    if (numSelectSlider == 0) colorSlider1 = display.RED;
    else colorSlider2 = display.RED;
    
    this->drawRoundRect(x, y+20, LINE_WIDTH*z1, LINE_HEIGHT, display.COLOR_BACK, display.COLOR_BACK, display.DARKGREY);
    this->drawRoundRect(x+LINE_WIDTH*z1+SLIDER_WIDTH, y+20, LINE_WIDTH*z2-(LINE_WIDTH*z1+SLIDER_WIDTH), LINE_HEIGHT, display.COLOR_BACK, display.COLOR_BACK, 0x00C0FF);
    this->drawRoundRect(x+LINE_WIDTH*z2+SLIDER_WIDTH, y+20, (LINE_WIDTH*100+SLIDER_WIDTH)-(LINE_WIDTH*z2+SLIDER_WIDTH), LINE_HEIGHT, display.COLOR_BACK, display.COLOR_BACK, display.DARKGREY);
    
    writeFillRect(x-SLIDER_WIDTH/2, y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2), LINE_WIDTH*z1+SLIDER_WIDTH/2, (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
    writeFillRect(x-SLIDER_WIDTH/2, y+20+LINE_HEIGHT, LINE_WIDTH*z1+SLIDER_WIDTH/2, (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
    this->drawRoundRect(x+LINE_WIDTH*z1,y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2),SLIDER_WIDTH,SLIDER_HEIGHT,display.COLOR_BACK,display.COLOR_BACK,colorSlider1);
    
    writeFillRect(x+LINE_WIDTH*z1+SLIDER_WIDTH, y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2), LINE_WIDTH*z2-(LINE_WIDTH*z1+SLIDER_WIDTH), (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
    writeFillRect(x+LINE_WIDTH*z1+SLIDER_WIDTH, y+20+LINE_HEIGHT, LINE_WIDTH*z2-(LINE_WIDTH*z1+SLIDER_WIDTH), (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
    this->drawRoundRect(x+LINE_WIDTH*z2,y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2),SLIDER_WIDTH,SLIDER_HEIGHT,display.COLOR_BACK,display.COLOR_BACK,colorSlider2);
    
    writeFillRect(x+LINE_WIDTH*z2+SLIDER_WIDTH, y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2), (LINE_WIDTH*100+SLIDER_WIDTH)-(LINE_WIDTH*z2+SLIDER_WIDTH)+SLIDER_WIDTH/2, (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
    writeFillRect(x+LINE_WIDTH*z2+SLIDER_WIDTH, y+20+LINE_HEIGHT, (LINE_WIDTH*100+SLIDER_WIDTH)-(LINE_WIDTH*z2+SLIDER_WIDTH)+SLIDER_WIDTH/2, (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
}
//-----------------------------------------------------
void Canvas::drawSliderDouble(int16_t x, int16_t y, uint8_t z1, uint8_t z2, uint8_t numSelectSlider)
{
    const float LINE_WIDTH = 2.4;
    const int LINE_HEIGHT = 18;
    const int SLIDER_WIDTH = 14;
    const int SLIDER_HEIGHT = 20;
    const int SLIDER_COLOR = 0x00E0E0E0;
    int colorSlider1 = SLIDER_COLOR, colorSlider2 = SLIDER_COLOR;
    
    if (numSelectSlider == 0) colorSlider1 = display.RED;
    else colorSlider2 = display.RED;
    
    this->drawRoundRect(x, y+20, LINE_WIDTH*z1, LINE_HEIGHT, display.COLOR_BACK, display.COLOR_BACK, display.DARKGREY);
    this->drawRoundRect(x+LINE_WIDTH*z1+SLIDER_WIDTH, y+20, LINE_WIDTH*z2-(LINE_WIDTH*z1+SLIDER_WIDTH), LINE_HEIGHT, display.COLOR_BACK, display.COLOR_BACK, 0x00C0FF);
    this->drawRoundRect(x+LINE_WIDTH*z2+SLIDER_WIDTH, y+20, (LINE_WIDTH*100+SLIDER_WIDTH)-(LINE_WIDTH*z2+SLIDER_WIDTH), LINE_HEIGHT, display.COLOR_BACK, display.COLOR_BACK, display.DARKGREY);
    
    writeFillRect(x-SLIDER_WIDTH/2, y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2), LINE_WIDTH*z1+SLIDER_WIDTH/2, (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
    writeFillRect(x-SLIDER_WIDTH/2, y+20+LINE_HEIGHT, LINE_WIDTH*z1+SLIDER_WIDTH/2, (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
    this->drawRoundRect(x+LINE_WIDTH*z1,y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2),SLIDER_WIDTH,SLIDER_HEIGHT,display.COLOR_BACK,display.COLOR_BACK,colorSlider1);
    
    writeFillRect(x+LINE_WIDTH*z1+SLIDER_WIDTH, y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2), LINE_WIDTH*z2-(LINE_WIDTH*z1+SLIDER_WIDTH), (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
    writeFillRect(x+LINE_WIDTH*z1+SLIDER_WIDTH, y+20+LINE_HEIGHT, LINE_WIDTH*z2-(LINE_WIDTH*z1+SLIDER_WIDTH), (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
    this->drawRoundRect(x+LINE_WIDTH*z2,y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2),SLIDER_WIDTH,SLIDER_HEIGHT,display.COLOR_BACK,display.COLOR_BACK,colorSlider2);
    
    writeFillRect(x+LINE_WIDTH*z2+SLIDER_WIDTH, y+20-((SLIDER_HEIGHT-LINE_HEIGHT)/2), (LINE_WIDTH*100+SLIDER_WIDTH)-(LINE_WIDTH*z2+SLIDER_WIDTH)+SLIDER_WIDTH/2, (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
    writeFillRect(x+LINE_WIDTH*z2+SLIDER_WIDTH, y+20+LINE_HEIGHT, (LINE_WIDTH*100+SLIDER_WIDTH)-(LINE_WIDTH*z2+SLIDER_WIDTH)+SLIDER_WIDTH/2, (SLIDER_HEIGHT-LINE_HEIGHT)/2, display.COLOR_BACK);
}
//-----------------------------------------------------
uint8_t Canvas::loadImage(uint16_t x, uint16_t y, uint32_t n)
{
    uint32_t i, addr=0, c;
    uint16_t w, h;
    uint8_t data[11];
    
    while(n--){
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
    
    display.setAddrWindow(x,y,w,h);
    MEM_CS_ON;
    DISP_cs_on;
    memory.quadOutInit();
    memory.quadSend(W25_FAST_READ_QUAD);
    memory.quadSend((addr >> 16) & 0xFF);
    memory.quadSend((addr >> 8) & 0xFF);
    memory.quadSend(addr & 0xFF);
    memory.quadSend(0x00);
    memory.quadInInit();
    for (i=0; i<(w*h); i++){  // (i=0; i<(w*h*3); i++)
        /*
        byte = memory.quadReceiv();
        while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
        SPI_I2S_SendData16(SPI1, 0x100 + byte);
        */
        c = memory.quadReceiv()<<16;
        c += memory.quadReceiv()<<8;
        c += memory.quadReceiv();
        display.writeColorFast(c);
    }
    MEM_CS_OFF;
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
    DISP_cs_off;
    
    return 0x00;
}
//-----------------------------------------------------
uint8_t Canvas::loadImagePointer(uint16_t X, uint16_t Y, uint32_t S)
{
    uint32_t i, c, addr=0, rnd=0;
    uint8_t data[11], n;
    int32_t x, y;
    static uint16_t w, h;
    static uint8_t buf[2700], f=0;
    
    if (f == 0){
        n = 4;
        while(n--){
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
        
        display.setAddrWindow(x,y,h,w);
        MEM_CS_ON;
        memory.spiSend(W25_FAST_READ);
        memory.spiSend((addr >> 16) & 0xFF);
        memory.spiSend((addr >> 8) & 0xFF);
        memory.spiSend(addr & 0xFF);
        memory.spiSend(0x00);
        for (i=0; i<(w*h*3); i++){
            buf[i] = memory.spiSend(0x00);
        }
        MEM_CS_OFF;
        f = 1;
    }
    
    
    i = h*w;
    while(1){
        for (y=0; y<h; y++){
            for (x=0; x<w; x++){
                rnd++;
                if (rnd > S){
                    rnd = 0;
                    c = (buf[y*h*3+x*3+0]<<16)+(buf[y*h*3+x*3+1]<<8)+buf[y*h*3+x*3+2];
                    
                    if (c > 0) canvas.writePixel(X+y, Y+x, c);
                    if (i > 0) i--;
                    else return 0;
                }
            }
        }
    }
}
//-----------------------------------------------------
uint8_t Canvas::loadBackground(uint16_t X, uint16_t Y, uint16_t W, uint16_t H, uint32_t N)
{
    uint32_t addr=0;
    uint16_t w, h;
    uint8_t data[11];
    uint32_t c;
    int32_t x, y;
    
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
    
    display.setAddrWindow(X,Y,W,H);
    DISP_cs_on;
    
    for (x=(X+W-1); x>=X; x--){
        y = Y;
        c = addr+((x-IMAGE_BCK_X)*w*3+(y-IMAGE_BCK_Y)*3);
        MEM_CS_ON;
        memory.spiSend(W25_READ);
        memory.spiSend((c >> 16) & 0xFF);
        memory.spiSend((c >> 8) & 0xFF);
        memory.spiSend(c & 0xFF);
        for (y=0; y<(H*3); y++){
            data[0] = memory.spiSend(0x00);
            if ((x-IMAGE_BCK_X)>w || (Y-IMAGE_BCK_Y+y/3)>h || 
                (x-IMAGE_BCK_X)<0 || (Y-IMAGE_BCK_Y+y/3)<0) data[0] = 0;
            while(spi_i2s_flag_get(SPI0, SPI_FLAG_TBE) == RESET);
            spi_i2s_data_transmit(SPI0, 0x100 + data[0]);
        }
        MEM_CS_OFF;
    }
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
    DISP_cs_off;
    
    return 0x00;
}
//-----------------------------------------------------
uint8_t Canvas::loadBackgroundInvert(uint16_t X, uint16_t Y, uint16_t W, uint16_t H, uint32_t N)
{
    uint32_t addr=0;
    uint16_t w, h;
    uint8_t data[11];
    uint32_t c;
    int32_t x, y;
    
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
    
    display.setAddrWindow(X,Y,W,H);
    DISP_cs_on;
    
    for (x=(X+W-1); x>=X; x--){
        y = Y;
        c = addr+((x-IMAGE_BCK_X)*w*3+(y-IMAGE_BCK_Y)*3);
        MEM_CS_ON;
        memory.spiSend(W25_READ);
        memory.spiSend((c >> 16) & 0xFF);
        memory.spiSend((c >> 8) & 0xFF);
        memory.spiSend(c & 0xFF);
        for (y=0; y<H; y++){
            data[0]=memory.spiSend(0x00);
            data[1]=memory.spiSend(0x00);
            data[2]=memory.spiSend(0x00);
            
            while(spi_i2s_flag_get(SPI0, SPI_FLAG_TBE) == RESET);
            spi_i2s_data_transmit(SPI0, 0x100 + data[2]);
            while(spi_i2s_flag_get(SPI0, SPI_FLAG_TBE) == RESET);
            spi_i2s_data_transmit(SPI0, 0x100 + data[1]);
            while(spi_i2s_flag_get(SPI0, SPI_FLAG_TBE) == RESET);
            spi_i2s_data_transmit(SPI0, 0x100 + data[0]);
            
        }
        MEM_CS_OFF;
    }
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
    DISP_cs_off;
    
    return 0x00;
}
//-----------------------------------------------------
uint8_t Canvas::loadImageEffectPoly(uint16_t X, uint16_t Y, uint32_t n, uint32_t S, uint8_t F)
{
    uint32_t i, addr=0;
    uint16_t w, h;
    uint8_t data[11];
    uint32_t c;
    int32_t x, y;
    uint32_t rnd=0;
    bool right, left, up, down;
    
    while(n--){
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
    
    i = h*w;
    while(1){
        for (y=0; y<h; y++){
            for (x=0; x<w; x++){
                rnd++;
                if (rnd > S){
                    rnd = 0;
                    memory.readPage(data, addr+(y*w*3+x*3), 3);
                    c = (data[0]<<16)+(data[1]<<8)+data[2];
                    
                    // исключаем углы изображения
                    right = (x > (w-10));
                    left = (x < 10);
                    up = (y > (h-10));
                    down = (y < 10);
                 
                    if (!((right && up) || (right && down) || (left && up) || (left && down))){
                        if (c>0 || F==0) canvas.writePixel(X+(w-y-1), Y+x, c);
                    }
                    if (i > 0) i--;
                    else return 0;
                }
            }
        }
    }
}
//-----------------------------------------------------
uint8_t Canvas::loadImageEffect(uint16_t X, uint16_t Y, uint32_t n, uint32_t S, uint8_t F)
{
    uint32_t i, addr=0;
    uint16_t w, h;
    uint8_t data[11];
    uint32_t c;
    int32_t x, y;
    uint32_t rnd=0;
    
    while(n--){
        memory.readPage(data, addr, 11);
        if (data[0]==0xCD && data[1]==0xBA && data[2]==0x00){
            h = data[4]*256+data[3];
            w = data[6]*256+data[5];
            addr += w*h*3+11;
        }
        else return 0x01;
    }
    
    memory.readPage(data, addr, 11);
    if (data[0]==0xCD && data[1]==0xBA && data[2]==0x00){
        h = data[4]*256+data[3];
        w = data[6]*256+data[5];
        addr += 11;
    }
    else return 0x01;
    
    i = h*w;
    while(1){
        for (y=0; y<h; y++){
            for (x=0; x<w; x++){
                rnd++;
                if (rnd > S){
                    rnd = 0;
                    memory.readPage(data, addr+(y*w*3+x*3), 3);
                    c = (data[0]<<16)+(data[1]<<8)+data[2];
                    
                    if (c>0 || F==0) canvas.writePixel(X+(h-y-1), Y+x, c);
                    if (i > 0) i--;
                    else return 0;
                }
            }
        }
    }
}
//-----------------------------------------------------
uint8_t Canvas::loadImageColor(uint16_t x, uint16_t y, uint32_t n, uint32_t color)
{
    uint32_t i, addr=0, c;
    uint16_t w, h;
    uint8_t data[11];
    
    while(n--){
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
    
    display.setAddrWindow(x,y,w,h);
    MEM_CS_ON;
    DISP_cs_on;
    memory.quadOutInit();
    memory.quadSend(W25_FAST_READ_QUAD);
    memory.quadSend((addr >> 16) & 0xFF);
    memory.quadSend((addr >> 8) & 0xFF);
    memory.quadSend(addr & 0xFF);
    memory.quadSend(0x00);
    memory.quadInInit();
    for (i=0; i<(w*h); i++){  // (i=0; i<(w*h*3); i++)
        /*
        byte = memory.quadReceiv();
        while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
        SPI_I2S_SendData16(SPI1, 0x100 + byte);
        */
        c = memory.quadReceiv()<<16;
        c += memory.quadReceiv()<<8;
        c += memory.quadReceiv();
        display.writeColorFast(c & color);
    }
    MEM_CS_OFF;
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
    DISP_cs_off;
    
    return 0x00;
}
//-----------------------------------------------------
uint8_t Canvas::loadImageOverColor(uint16_t X, uint16_t Y, uint32_t n, uint32_t S, uint8_t F, uint32_t color)
{
    uint32_t i, addr=0;
    uint16_t w, h;
    uint8_t data[11];
    uint32_t c;
    int32_t x, y;
    uint32_t rnd=0;
    
    while(n--){
        memory.readPage(data, addr, 11);
        if (data[0]==0xCD && data[1]==0xBA && data[2]==0x00){
            h = data[4]*256+data[3];
            w = data[6]*256+data[5];
            addr += w*h*3+11;
        }
        else return 0x01;
    }
    
    memory.readPage(data, addr, 11);
    if (data[0]==0xCD && data[1]==0xBA && data[2]==0x00){
        h = data[4]*256+data[3];
        w = data[6]*256+data[5];
        addr += 11;
    }
    else return 0x01;
    
    i = h*w;
    while(1){
        for (y=0; y<h; y++){
            for (x=0; x<w; x++){
                rnd++;
                if (rnd > S){
                    rnd = 0;
                    memory.readPage(data, addr+(y*w*3+x*3), 3);
                    c = (data[0]<<16)+(data[1]<<8)+data[2];
                    
                    if (c>0 || F==0){
                        c &= color;
                        canvas.writePixel(X+(h-y-1), Y+x, c);
                    }
                    if (i > 0) i--;
                    else return 0;
                }
            }
        }
    }
}
//-----------------------------------------------------
uint8_t Canvas::loadImageEffectInvert(uint16_t X, uint16_t Y, uint32_t n, uint32_t S)
{
    uint32_t i, addr=0;
    uint16_t w, h;
    uint8_t data[11];
    uint32_t c;
    int32_t x, y;
    uint32_t rnd=0;
    
    while(n--){
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
    
    i = h*w;
    while(1){
        for (y=0; y<h; y++){
            for (x=0; x<w; x++){
                rnd++;
                if (rnd > S){
                    rnd = 0;
                    memory.readPage(data, addr+(y*h*3+x*3), 3);
                    c = (data[2]<<16)+(data[1]<<8)+data[0];
                    
                    if (c > 0) canvas.writePixel(X+(h-y-1), Y+x, c);
                    if (i > 0) i--;
                    else return 0;
                }
            }
        }
    }
}
//-----------------------------------------------------
uint8_t Canvas::loadImageEffectRed(uint16_t X, uint16_t Y, uint32_t n, uint32_t S)
{
    uint32_t i, addr=0;
    uint16_t w, h;
    uint8_t data[11];
    uint32_t c;
    int32_t x, y;
    uint32_t rnd=0;
    
    while(n--){
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
    
    i = h*w;
    while(1){
        for (y=0; y<h; y++){
            for (x=0; x<w; x++){
                rnd++;
                if (rnd > S){
                    rnd = 0;
                    memory.readPage(data, addr+(y*h*3+x*3), 3);
                    c = (data[2]<<16)+(data[1]/8<<8)+data[0]/4;
                    
                    if (c > 0) canvas.writePixel(X+(h-y-1), Y+x, c);
                    if (i > 0) i--;
                    else return 0;
                }
            }
        }
    }
}
//-----------------------------------------------------
uint8_t Canvas::loadImageEffectBlue(uint16_t X, uint16_t Y, uint32_t n, uint32_t S)
{
    uint32_t i, addr=0;
    uint16_t w, h;
    uint8_t data[11];
    uint32_t c;
    int32_t x, y;
    uint32_t rnd=0;
    
    while(n--){
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
    
    i = h*w;
    while(1){
        for (y=0; y<h; y++){
            for (x=0; x<w; x++){
                rnd++;
                if (rnd > S){
                    rnd = 0;
                    memory.readPage(data, addr+(y*h*3+x*3), 3);
                    
                    uint8_t red = 0;    // data[2]/test_r;
                    uint8_t green = data[1]/1.5;
                    uint8_t blue = data[0];
                    
                    c = (red<<16)+(green<<8)+blue;
                    
                    if (c > 0) canvas.writePixel(X+(h-y-1), Y+x, c);
                    if (i > 0) i--;
                    else return 0;
                }
            }
        }
    }
}
//-----------------------------------------------------
uint8_t Canvas::loadImageEffectGrey(uint16_t X, uint16_t Y, uint32_t n, uint32_t S)
{
    uint32_t i, addr=0;
    uint16_t w, h;
    uint8_t data[11];
    uint32_t c;
    int32_t x, y;
    uint32_t rnd=0;
    
    while(n--){
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
    
    i = h*w;
    while(1){
        for (y=0; y<h; y++){
            for (x=0; x<w; x++){
                rnd++;
                if (rnd > S){
                    rnd = 0;
                    memory.readPage(data, addr+(y*h*3+x*3), 3);
                    
                    uint8_t red = data[2]/2;
                    uint8_t green = data[1]/2;
                    uint8_t blue = data[0]/2;
                    
                    c = (red<<16)+(green<<8)+blue;
                    
                    if (c > 0) canvas.writePixel(X+(h-y-1), Y+x, c);
                    if (i > 0) i--;
                    else return 0;
                }
            }
        }
    }
}
//-----------------------------------------------------
uint8_t Canvas::loadImageEffectColor(uint16_t X, uint16_t Y, uint32_t n, uint32_t S, uint32_t color)
{
    uint32_t i, addr=0;
    uint16_t w, h;
    uint8_t data[11];
    uint32_t c;
    int32_t x, y;
    uint32_t rnd=0;
    
    while(n--){
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
    
    i = h*w;
    while(1){
        for (y=0; y<h; y++){
            for (x=0; x<w; x++){
                rnd++;
                if (rnd > S){
                    rnd = 0;
                    memory.readPage(data, addr+(y*h*3+x*3), 3);
                    c = (data[0]<<16)+(data[1]<<8)+data[2];
                    
                    if (c > 0) canvas.writePixel(X+(h-y-1), Y+x, color);
                    if (i > 0) i--;
                    else return 0;
                }
            }
        }
    }
}
//-----------------------------------------------------
uint8_t Canvas::drawImageEffectColor(uint16_t X, uint16_t Y, uint16_t W, uint16_t H, uint32_t n, uint32_t S, uint32_t color)
{
    uint32_t i, addr=0;
    uint16_t w, h;
    uint8_t data[11];
    uint32_t c;
    int32_t x, y;
    uint32_t rnd=0;
    
    while(n--){
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
    
    i = H*W;
    while(1){
        for (y=Y; y<(Y+H); y++){
            for (x=X; x<(X+W); x++){
                rnd++;
                if (rnd > S){
                    rnd = 0;
                    memory.readPage(data, addr+((w-x-1)*h*3+y*3), 3);
                    c = (data[0]<<16)+(data[1]<<8)+data[2];
                    
                    if (c > 0) canvas.writePixel(x, y, c & color);
                    ///canvas.writePixel(x, y, color);
                    if (i > 0) i--;
                    else return 0;
                }
            }
        }
    }
}
//-----------------------------------------------------
void Canvas::rotateImage(uint16_t posX, uint16_t posY, int angle, int offsetX, int offsetY)
{
    const double PI = 3.14159265359;
    uint16_t w=3, h=12, i=0;
    uint32_t arrow_buf[3*12] = {
    display.LIGHTGREY, display.LIGHTGREY, display.LIGHTGREY,
    display.LIGHTGREY, display.LIGHTGREY, display.LIGHTGREY,
    display.LIGHTGREY, display.LIGHTGREY, display.LIGHTGREY,
    display.LIGHTGREY, display.LIGHTGREY, display.LIGHTGREY,
    display.LIGHTGREY, display.LIGHTGREY, display.LIGHTGREY,
    display.LIGHTGREY, display.LIGHTGREY, display.LIGHTGREY,
    display.LIGHTGREY, display.LIGHTGREY, display.LIGHTGREY,
    display.LIGHTGREY, display.LIGHTGREY, display.LIGHTGREY,
    display.LIGHTGREY, display.LIGHTGREY, display.LIGHTGREY,
    display.LIGHTGREY, display.LIGHTGREY, display.LIGHTGREY,
    display.LIGHTGREY, display.LIGHTGREY, display.LIGHTGREY,
    display.LIGHTGREY, display.LIGHTGREY, display.LIGHTGREY,
};
    
    double aRad = angle/180.0*PI;
    double rMx[2][2] = 
    {
        {cos(aRad),-sin(aRad)},
        {sin(aRad),cos(aRad)}
    };
    for (int x=0; x<w; x++){
        for (int y=0; y<h; y++) {
            int tx = (x-offsetX)*rMx[0][0] + (y-offsetY)*rMx[0][1] + offsetX;
            int ty = (x-offsetX)*rMx[1][0] + (y-offsetY)*rMx[1][1] + offsetY;
            //нужно в точку (tx,ty) в newImg поместить точку (x,y) из img
            canvas.writePixel(tx+posX, ty+posY, arrow_buf[i++]);
        }
    }
}
//-----------------------------------------------------
void Canvas::writeDot(uint16_t x, uint16_t y, char flag, char size)
{
    if (size == 0){
        display.setAddrWindow(x-1,y-1,3,3);
        DISP_cs_on;
        if (flag == 0){
            display.writeColorFast(this->color1); display.writeColorFast(this->color2); display.writeColorFast(this->color1);
            display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2);
            display.writeColorFast(this->color1); display.writeColorFast(this->color2); display.writeColorFast(this->color1);
        }
        else if (flag == 1){
            display.writeColorFast(this->color2); display.writeColorFast(this->color3); display.writeColorFast(this->color2);
            display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3);
            display.writeColorFast(this->color2); display.writeColorFast(this->color3); display.writeColorFast(this->color2);
        }
        else if (flag == 2){
            display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2);
            display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2);
            display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2);
        }
        else if (flag == 3){
            display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3);
            display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3);
            display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3);
        }
        else if (flag == 4){
            display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0);
            display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0);
            display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0);
        }
        while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
        DISP_cs_off;
    }
    else if (size == 1){
        display.setAddrWindow(x-2,y-2,5,5);
        DISP_cs_on;
        if (flag == 0){
            display.writeColorFast(this->color1); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color1);
            display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2);
            display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2);
            display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2);
            display.writeColorFast(this->color1); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color1);
        }
        else if (flag == 1){
            display.writeColorFast(this->color2); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color2);
            display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3);
            display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3);
            display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3);
            display.writeColorFast(this->color2); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color2);
        }
        else if (flag == 2){
            display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2);
            display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2);
            display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2);
            display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2);
            display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2); display.writeColorFast(this->color2);
        }
        else if (flag == 3){
            display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3);
            display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3);
            display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3);
            display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3);
            display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3); display.writeColorFast(this->color3);
        }
        else if (flag == 4){
            display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0);
            display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0);
            display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0);
            display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0);
            display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0);
        }
        while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
        DISP_cs_off;
    }
}
//-----------------------------------------------------
void Canvas::clearDot(uint16_t x, uint16_t y, char flag, char size)
{
    if (size == 0){
        display.setAddrWindow(x-1,y-1,3,3);
        DISP_cs_on;
        display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0);
        display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0);
        display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0);
        while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
        DISP_cs_off;
    }
    else if (size == 1){
        display.setAddrWindow(x-2,y-2,5,5);
        DISP_cs_on;
        display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); 
        display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); 
        display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); 
        display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); 
        display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); display.writeColorFast(this->color0); 
        while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
        DISP_cs_off;
    }
}
//-----------------------------------------------------
void Canvas::drawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color)
{
    writeFillRect(x+1, y, w-2, 1, color);
    writeFillRect(x+1, y+h-1, w-2, 1, color);
    writeFillRect(x, y+1, 1, h-2, color);
    writeFillRect(x+w-1, y+1, 1, h-2, color);
}
//-----------------------------------------------------
void Canvas::writeMinus(uint16_t x, uint16_t y)
{
    uint16_t i;
    const char matrix[9*9] = 
    {
        0,0,0,0,0,0,0,0,0,
        0,0,0,1,2,1,0,0,0,
        0,0,0,2,2,2,0,0,0,
        0,0,0,2,2,2,0,0,0,
        0,0,0,2,2,2,0,0,0,
        0,0,0,2,2,2,0,0,0,
        0,0,0,2,2,2,0,0,0,
        0,0,0,1,2,1,0,0,0,
        0,0,0,0,0,0,0,0,0
    };
    
    display.setAddrWindow(x-4,y-4,9,9);
    DISP_cs_on;
    for (i=0; i<(9*9); i++){
        if (matrix[i] == 2) display.writeColorFast(0x808080);
        else if (matrix[i] == 1) display.writeColorFast(0x101010);
        else display.writeColorFast(0);
    }
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
    DISP_cs_off;
}
//-----------------------------------------------------
void Canvas::writePlus(uint16_t x, uint16_t y)
{
    uint16_t i;
    const char matrix[9*9] = 
    {
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,2,2,0,0,0,
        0,0,0,0,2,2,0,0,0,
        0,0,2,2,2,2,2,2,0,
        0,0,2,2,2,2,2,2,0,
        0,0,0,0,2,2,0,0,0,
        0,0,0,0,2,2,0,0,0,
        0,0,0,0,0,0,0,0,0
    };
    
    display.setAddrWindow(x-4,y-4,9,9);
    DISP_cs_on;
    for (i=0; i<(9*9); i++){
        if (matrix[i] == 2) display.writeColorFast(0x808080);
        else if (matrix[i] == 1) display.writeColorFast(0x101010);
        else display.writeColorFast(0);
    }
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS) == SET);
    DISP_cs_off;
}
//-----------------------------------------------------
