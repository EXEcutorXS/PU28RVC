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
#include "checkbox.h"
#include "text.h"
#include "canvas.h"

Checkbox checkbox;
//-----------------------------------------------------
Checkbox::Checkbox(void)
{
    
}
//-----------------------------------------------------
void Checkbox::draw(uint16_t x, uint16_t y, const char* str, char* flag)
{
    if (*flag == 3){
        text.writeString(x+30,y,str,Font_11x18,this->COLOR_ON,this->COLOR_BACK);
        canvas.loadImageEffect(x,y,SETUP_IMAGE_ON,this->SELECTOR_STEP,0);
        *flag = 1;
    }
    else if (*flag == 2){
        text.writeString(x+30,y,str,Font_11x18,this->COLOR_OFF,this->COLOR_BACK);
        canvas.loadImageEffect(x,y,SETUP_IMAGE_OFF,this->SELECTOR_STEP,0);
        *flag = 0;
    }
}
//-----------------------------------------------------
void Checkbox::draw(uint16_t x, uint16_t y, const char* str, bool flag)
{
    if (flag == TRUE){
        text.writeString(x+30,y,str,Font_11x18,this->COLOR_ON,this->COLOR_BACK);
        canvas.loadImageEffect(x,y,SETUP_IMAGE_ON,this->SELECTOR_STEP,0);
    }
    else {
        text.writeString(x+30,y,str,Font_11x18,this->COLOR_OFF,this->COLOR_BACK);
        canvas.loadImageEffect(x,y,SETUP_IMAGE_OFF,this->SELECTOR_STEP,0);
    }
}
//-----------------------------------------------------
