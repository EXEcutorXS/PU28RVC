/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SETUP_H
#define __SETUP_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Classes ------------------------------------------------------------------*/
class Setup
{
    public:
        Setup(void);
        void viewScreenChange(void);
        void viewScreen(void);
        uint8_t handler(void);
        uint8_t viewPage(uint8_t number, uint8_t mode);
        void viewScreen1(uint8_t mode);
    /*
        void viewScreen2(uint8_t mode);
        void viewScreen3(uint8_t mode);
        void viewScreen4(uint8_t mode);
        void viewScreen5(uint8_t mode);
        void viewScreen6(uint8_t mode);
        void viewScreen7(uint8_t mode);
        void viewScreen8(uint8_t mode);
        void viewScreen9(uint8_t mode);
    */  
        void viewScreen10(uint8_t mode);
        uint8_t viewScreen11(uint8_t mode);
        void viewScreen12(uint8_t mode);
        void viewScreen13(uint8_t mode);
        void viewScreen14(uint8_t mode);
        void viewScreen15(uint8_t mode);
				void viewScreen16(uint8_t mode); //ScheduleMode + TimeAdjusting
        void viewScreen100(uint8_t mode);
        
        uint8_t numberScreen;
        uint8_t maxNumberScreen;
        bool isChange;
        
        static const int EXIT_X = 0;
        static const int EXIT_Y = 0;
        static const int EXIT_SIZE = 60;
        static const int EXIT_STEP = 0;
        static const int NEXT_X = 270;
        static const int NEXT_Y = 0;
        static const int NEXT_SIZE = 60;
        static const int NEXT_STEP = 0;
        static const int SELECTOR_STEP = 408;
        static const int COLOR_TEXT = 0xE0E0E0;
        static const int COLOR_ON = 0xFFFFFF;
        static const int COLOR_OFF = 0xC0C0C0;
    
    private:
        bool isSlider1, isSlider2, isSlider3, isSlider4;
        
};
extern Setup setup;

/* Info ------------------------------------------------------------------*/


#endif /* __SETUP_H */
