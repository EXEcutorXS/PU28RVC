/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"
#include "core.h"

/* Functions ------------------------------------------------------------------*/

/* Defines ------------------------------------------------------------------*/
//#define NO_IWDG

#undef NULL
#define NULL    0
#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif

#define ALIGN                  volatile __align(4) 

#define BLE_ID_ADDRESS       0x0803F000
#define CRC_FIRMWARE_ADDRESS 0x0803F800


#define SLIDER_RADIUS_ALL       85              // радиус шкалы (было 85)
#define SLIDER_HELP_STEP        0.15f            // шаг мерцания подсказки шкалы
#define MAIN_SLIDER_RADIUS      50              // размер сенсорного "зацепа" ползунка
#define SLIDER_STEP             25              // шаг изменения положения ползунка (было 24)
#define SLIDER_CLEAR_MIN        -8              // длина перекраски области внутри ползунка 0
#define SLIDER_CLEAR_MAX        9               // длина перекраски области снаружи ползунка 16
#define SLIDER_CHANGE_CLEAR     4               // ширина очистки области вокруг ползунка при смене направления
#define SLIDER_FONT             Font_16x26      // шрифт для выбранного значения ползунка (Font_16x26)
#define VALUE_FONT              Font_11x18      // шрифт для десятых(или четных) значений шкалы (Font_11x18)
#define VALUE_SMALL_FONT        Font_7x10       // шрифт для пятых(или нечетных) значений шкалы (Font_7x10)
#define COLOR_DIG_GRID          0xA0A0A0        // цвет символов цифровой шкалы 0x808080
//#define SLIDER_COLOR_COLD       (data[2]<<16)+(data[1]<<8)+data[0]    // BLUE
//#define SLIDER_COLOR_COLD       (data[2]<<16)+(data[0]<<8)+data[1]    // GREEN
//#define SLIDER_COLOR_COLD       (data[0]<<16)+(data[0]<<8)+data[0]    // GRAY
//#define SLIDER_COLOR_COLD       (data[2]<<16)+(data[0]<<8)+data[0]    // MAGENTA
//#define SLIDER_COLOR_COLD       (data[0]<<16)+(data[1]<<8)+data[0]    // PURPLE
//#define SLIDER_COLOR_COLD       (data[0]<<16)+(data[0]<<8)+data[1]    // YELLOW
//#define SLIDER_COLOR_COLD       (data[0]<<16)+(data[0]/2<<8)+data[1]  // ORANGE
#define SLIDER_COLOR_COLD       (data[2]<<16)+(data[0]/2<<8)+data[0]  // LIGHT_BLUE

#define SLIDER_COLOR_1          display.WHITE
#define SLIDER_COLOR_2          0x03A5FC        // display.BLUE
#define SLIDER_COLOR_3          0xF01A14        // display.RED

#define VALUE_RADIUS_ALL        115             // радиус значений шкалы
///#define VALUE_LEN               504             // размер буферов шкалы и ползунка

#define DELTA_RADIUS_ALL        (float)(VALUE_RADIUS_ALL - SLIDER_RADIUS_ALL)

#define IMAGE_BCK_X 55
#define IMAGE_BCK_Y 25

#define BUTTON_DEC_X            (105-11)
#define BUTTON_DEC_Y            180
#define BUTTON_DEC_SIZE         35

#define BUTTON_INC_X            215
#define BUTTON_INC_Y            180
#define BUTTON_INC_SIZE         35

#define BUTTON_TEMP_X           160
#define BUTTON_TEMP_Y           205
#define BUTTON_TEMP_SIZE        25

#define BUTTON_TIMER_X          0
#define BUTTON_TIMER_Y          0
#define BUTTON_TIMER_SIZE       60
#define BUTTON_TIMER_STEP       408

#define BUTTON_SETUP_X          260
#define BUTTON_SETUP_Y          0
#define BUTTON_SETUP_SIZE       60
#define BUTTON_SETUP_STEP       408

#define BUTTON_FHEAT_X          32
#define BUTTON_FHEAT_Y          64
#define BUTTON_FHEAT_SIZE       112
#define BUTTON_FHEAT_STEP       8

#define BUTTON_EHEAT_X          176
#define BUTTON_EHEAT_Y          64
#define BUTTON_EHEAT_SIZE       112
#define BUTTON_EHEAT_STEP       8

#define BUTTON_SYSTEM_X         104
#define BUTTON_SYSTEM_Y         74
#define BUTTON_SYSTEM_SIZE      112
#define BUTTON_SYSTEM_STEP      8

#define BUTTON_LEFT_X           5
#define BUTTON_LEFT_Y           195
#define BUTTON_LEFT_SIZE        60
#define BUTTON_LEFT_STEP        408

#define BUTTON_RIGHT_X          275
#define BUTTON_RIGHT_Y          195
#define BUTTON_RIGHT_SIZE       60
#define BUTTON_RIGHT_STEP       408

#define BUTTON_WORK_STEP        4

#define BUTTON_UP_LEFT_X            0
#define BUTTON_UP_LEFT_Y            0
#define BUTTON_UP_LEFT_SIZE         60

#define BUTTON_UP_RIGHT_X           260
#define BUTTON_UP_RIGHT_Y           0
#define BUTTON_UP_RIGHT_SIZE        60

#define BUTTON_DOWN_LEFT_X          0
#define BUTTON_DOWN_LEFT_Y          190
#define BUTTON_DOWN_LEFT_X_SIZE     80
#define BUTTON_DOWN_LEFT_Y_SIZE     40

#define BUTTON_DOWN_RIGHT_X         240
#define BUTTON_DOWN_RIGHT_Y         190
#define BUTTON_DOWN_RIGHT_X_SIZE    80
#define BUTTON_DOWN_RIGHT_Y_SIZE    40

#define BUTTON_DAY_NIGHT_X          235
#define BUTTON_DAY_NIGHT_Y          205
#define BUTTON_DAY_NIGHT_X_SIZE     30
#define BUTTON_DAY_NIGHT_Y_SIZE     30

#define BUTTON_BLE_X          67
#define BUTTON_BLE_Y          210
#define BUTTON_BLE_X_SIZE     13
#define BUTTON_BLE_Y_SIZE     20

#define STEP_ZERO                   0

#define BACKGROUND_IMAGE        0               // 
#define BUTTON_SETUP_IMAGE      1               // 
#define SETUP_SAVE_IMAGE        2               // 
#define SETUP_EXIT_IMAGE        3               // 
#define SETUP_IMAGE_OFF         4               // 
#define SETUP_IMAGE_ON          5               // 
#define SETUP_CLOCK_IMAGE       6               // 
#define BLACK_FHEAT_IMAGE       7               // 
#define BLUE_FHEAT_IMAGE        8               // 
#define BLACK_EHEAT_IMAGE       9               // 
#define BLUE_EHEAT_IMAGE        10              // 
#define BLACK_WATER_IMAGE       11              // 
#define BLUE_WATER_IMAGE        12              // 
#define BUTTON_CENTRAL_IMAGE    13              // 
#define BUTTON_ERROR_IMAGE      14              //
#define ABOUT_IMAGE             15              // 
#define LOGO_IMAGE              16              // 

#define TIMER_AM_IMAGE          17              //
#define TIMER_PM_IMAGE          18              //
#define FONT_16x26_0            19              //
#define FONT_16x26_1            20              //
#define FONT_16x26_2            21              //
#define FONT_16x26_3            22              //
#define FONT_16x26_4            23              //
#define FONT_16x26_5            24              //
#define FONT_16x26_6            25              //
#define FONT_16x26_7            26              //
#define FONT_16x26_8            27              //
#define FONT_16x26_9            28              //
#define FONT_16x26_C            29              //
#define FONT_16x26_F            30              //
#define FONT_16x26_V            31              //
#define FONT_16x26_COL          32              //
#define FONT_16x26_DOT          33              //
#define FONT_16x26_DASH         34              //
#define FONT_16x26_DEG          35              //

#define TEXT_DAY_IMAGE          36              //
#define TEXT_NIGHT_IMAGE        37              //
#define TEXT_ZONE_1             38              //
#define TEXT_ZONE_2             39              //
#define TEXT_BLE_IMAGE          40              //
#define EMPTY_IMAGE             41              //

#define BUTTON_TIMER_OFF_IMAGE  100             // 
#define BUTTON_TIMER_ON_IMAGE   100             // 
#define BUTTON_TIMER_RED_IMAGE  100             // 

#define SCREEN_VISIBLE_WORK             0
#define SCREEN_VISIBLE_SETUP            1
#define SCREEN_VISIBLE_START_TIMERS     2
#define SCREEN_VISIBLE_SETUP_CLOCK      3
#define SCREEN_VISIBLE_WATER            4
#define SCREEN_VISIBLE_RADIATOR         5
#define SCREEN_VISIBLE_AIR              6
#define SCREEN_VISIBLE_SEARCH           7
#define SCREEN_VISIBLE_ERROR            8
#define SCREEN_VISIBLE_ABOUT            9
#define SCREEN_VISIBLE_BLE_CONNECT      10
#define SCREEN_VISIBLE_BOOT             0xA5
#define SCREEN_VISIBLE_SLEEP            0xFF

struct Errors{
    uint8_t code;
    const char *text;       
}; extern const Errors errors[];

const char ERROR_1[] = "Overheat";
const char ERROR_2[] = "Possible overheat";
const char ERROR_3[] = "Overheat temp. sensor";
const char ERROR_4[] = "Liquid temp. sensor";
const char ERROR_5[] = "Flame temp. sensor";
const char ERROR_6[] = "PCB temp. sensor";
const char ERROR_9[] = "Glow plug failure";
const char ERROR_10[] = "Revol. mismatch";
const char ERROR_12[] = "Overvolt.";
const char ERROR_13[] = "Non ignition";
const char ERROR_14[] = "Pump short circuit";
const char ERROR_15[] = "Undervolt";
const char ERROR_16[] = "Flame sensor not_cold";
const char ERROR_17[] = "Fuel pump broken";
const char ERROR_20[] = "Connect error";
const char ERROR_21[] = "Flame failure";
const char ERROR_22[] = "Fuel pump broken";
const char ERROR_24[] = "Temp. sensor oversh.";
const char ERROR_25[] = "Overheat during ignition";
const char ERROR_26[] = "Motor overload";
const char ERROR_27[] = "Motor not rotate";
const char ERROR_28[] = "Motor self rotation";
const char ERROR_29[] = "Flame break";
const char ERROR_30[] = "No connect heater";
const char ERROR_37[] = "Blocking by_non ignition";
const char ERROR_41[] = "Air temp sens fault";
const char ERROR_50[] = "No connect heater";
const char ERROR_78[] = "Flame failure during";
const char ERROR_100[] = "Wrong device";

/* Variables ------------------------------------------------------------------*/
extern uint8_t screen_visible, screen_visible_old;


/* Functions ------------------------------------------------------------------*/
extern void readSetup(void);
extern void writeSetup(void);

/* Info ------------------------------------------------------------------*/
//-----------------------------------------------------
// назначение выводов микроконтроллера:

// LCD: (SPI1)
// Enable  PB1
// Reset   PB11
// Led     PA3
// CS      PB0
// SCK     PA5
// MOSI    PA7

// CCP: (I2C1)
// Reset   PB8
// Int     PB5 (EXTI5)
// Scl     PB6
// Sda     PB7

// BLE: (SPI2)
// Reset   PB12
// IRQ     PB13
// CS      PA8
// SCK     PB10
// MISO    PB14
// MOSI    PB15

// FLASH: (SPI Quad)
// Reset(IO3)   PB9
// WP(IO2)      PC15
// CS           PC13
// DO(IO1)      PC14
// DI(IO0)      PA4
// CLK          PB4

// CAN:
// Standby PA15
// Shtdn   PB3
// Rx      PA11
// Tx      PA12

// Heater: (USART1, COMP1)
// Rx      PA10
// Tx      PA9
// Comp_M  PA4
// Comp_P  PA1
// Comp_O  PA6

// Temperature: (ADC2)
//         PA2

// Button: (EXTI0)
//         PA0

//-----------------------------------------------------

/*

*/

//-----------------------------------------------------


#endif /* __MAIN_H */
