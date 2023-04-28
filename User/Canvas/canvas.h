/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CANVAS_H
#define __CANVAS_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Classes ------------------------------------------------------------------*/
class Canvas
{
    public:
        Canvas(void);
        void writePixel(int16_t x, int16_t y, uint32_t color);
        void writePixelZoom(int16_t x, int16_t y, uint8_t zoom, uint32_t color);
        void writePixelDouble(int16_t x, int16_t y, uint32_t color1, uint32_t color2);
        void writePixelCount(int16_t x, int16_t y, uint32_t *color, uint8_t count);
        void writeFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
        void writeFillRectEffectPoly(uint16_t X, uint16_t Y, uint16_t w, uint16_t h, uint32_t color, uint32_t S);
        void writeFillRectEffect(uint16_t X, uint16_t Y, uint16_t w, uint16_t h, uint32_t color, uint32_t S);
        void drawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color_back, uint32_t color_side, uint32_t color);
        void drawSlider(int16_t x, int16_t y, uint8_t z, char v);
        void drawSliderSlim(int16_t x, int16_t y, uint8_t z, bool isActive, uint32_t color);
        void drawSliderSlimDouble(int16_t x, int16_t y, uint8_t z1, uint8_t z2, uint8_t numSelectSlider);
        void drawSliderDouble(int16_t x, int16_t y, uint8_t z1, uint8_t z2, uint8_t numSelectSlider);
        uint8_t loadImage(uint16_t x, uint16_t y, uint32_t n);
        uint8_t loadImagePointer(uint16_t X, uint16_t Y, uint32_t S);
        uint8_t loadBackground(uint16_t X, uint16_t Y, uint16_t W, uint16_t H, uint32_t N);
        uint8_t loadBackgroundInvert(uint16_t X, uint16_t Y, uint16_t W, uint16_t H, uint32_t N);
        uint8_t loadImageEffectPoly(uint16_t X, uint16_t Y, uint32_t n, uint32_t S, uint8_t F);
        uint8_t loadImageEffect(uint16_t X, uint16_t Y, uint32_t n, uint32_t S, uint8_t F);
        uint8_t loadImageColor(uint16_t x, uint16_t y, uint32_t n, uint32_t color);
        uint8_t loadImageOverColor(uint16_t X, uint16_t Y, uint32_t n, uint32_t S, uint8_t F, uint32_t color);
        uint8_t loadImageEffectInvert(uint16_t X, uint16_t Y, uint32_t n, uint32_t S);
        uint8_t loadImageEffectRed(uint16_t X, uint16_t Y, uint32_t n, uint32_t S);
        uint8_t loadImageEffectBlue(uint16_t X, uint16_t Y, uint32_t n, uint32_t S);
        uint8_t loadImageEffectGrey(uint16_t X, uint16_t Y, uint32_t n, uint32_t S);
        uint8_t loadImageEffectColor(uint16_t X, uint16_t Y, uint32_t n, uint32_t S, uint32_t color);
        uint8_t drawImageEffectColor(uint16_t X, uint16_t Y, uint16_t W, uint16_t H, uint32_t n, uint32_t S, uint32_t color);
        void rotateImage(uint16_t posX, uint16_t posY, int angle, int offsetX, int offsetY);\
        void writeDot(uint16_t x, uint16_t y, char flag, char size);
        void clearDot(uint16_t x, uint16_t y, char flag, char size);
        void drawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
        void writeMinus(uint16_t x, uint16_t y);
        void writePlus(uint16_t x, uint16_t y);
    
        uint32_t color0;
        uint32_t color1;
        uint32_t color2;
        uint32_t color3;
        
    private:
        
        
        
        
        
};
extern Canvas canvas;

/* Info ------------------------------------------------------------------*/

#endif /* __CANVAS_H */
