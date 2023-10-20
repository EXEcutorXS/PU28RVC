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
// Activity
#include "search.h"
#include "error.h"
#include "system.h"
#include "about.h"
#include "sleep.h"
#include "start_timers.h"
#include "setup.h"
#include "setup_clock.h"
// Objects
#include "hcu.h"
#include "start_timer.h"
#include "slider.h"
#include "checkbox.h"
#include "clock.h"
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
#include "unix_time.h"
#include "can.h"
#include "pgn_rvc.h"
#include "rvc.h"
#include "bluetooth.h"
#include "hcu.h"
#include "ble_connect.h"

/* Prototypes ------------------------------------------------------------------*/
void initAll(void);
void activitySearch(void);
void activityError(void);
void activityAbout(void);
void activityAir(void);
void activitySetup(void);
void activityStartTimers(void);
void activitySetupClock(void);
void activitySleep(void);
void activityBleConnect(void);
void handlerSleep(void);
void handlerSensor(void);
void handlerUsart(void);
void handlerTemperature(void);
void handlerClock(void);
void handlerTimer(void);
void addNewId(uint8_t *id);
void setNewId(void);
void setNewKey(void);
void writeSetup(void);
void readSetup(void);
void minuteHandler(void);
void handlerBluetooth(void);

/* Variables ------------------------------------------------------------------*/
uint8_t screen_visible=SCREEN_VISIBLE_SEARCH, screen_visible_old=SCREEN_VISIBLE_AIR;
bool isBleAccept = false;
bool isBleCancel = false;
bool isBleSendKey = false;

bool isTest = 0;

uint8_t newId[16];
uint32_t timerCancel;
uint32_t counterBleTemp, counterBleSendTemp;
uint32_t counterBle, counterBleSend;

uint32_t BLE_SEND_LONG_PERIOD;
uint32_t BLE_SEND_SHORT_PERIOD;
uint32_t BLE_PAUSE_PERIOD = 0;

const uint8_t _CRC[11]  __attribute__((at(0x0803F800 ))) =
{
  0x55, 0x55, 0x55, 0x55,     // длина ПО
  0x55, 0x55,                 // CRC ПО
  9, 0, 100, 26,              // версия ПО
  0x00                        // резерв
};

const Errors errors[28] =
{
  {1, ERROR_1},
  {2, ERROR_2},
  {3, ERROR_3},
  {4, ERROR_4},
  {5, ERROR_5},
  {6, ERROR_6},
  {9, ERROR_9},
  {10, ERROR_10},
  {12, ERROR_12},
  {13, ERROR_13},
  {14, ERROR_14},
  {15, ERROR_15},
  {16, ERROR_16},
  {17, ERROR_17},
  {20, ERROR_20},
  {21, ERROR_21},
  {22, ERROR_22},
  {24, ERROR_24},
  {25, ERROR_25},
  {26, ERROR_26},
  {27, ERROR_27},
  {28, ERROR_28},
  {29, ERROR_29},
  {30, ERROR_30},
  {37, ERROR_37},
  {50, ERROR_50},
  {78, ERROR_78},
  {100, ERROR_100},
};

//-----------------------------------------------------
int main(void)
{
  readSetup();
  initAll();                                                                  // инициализация микроконтроллера
  display.setLight(display.setup.brightness);                                 // включение подсветки дисплея

  search.viewScreen();
  setNewId();
  setNewKey();

  if (display.setup.celsius & 0x01) air.changeScale(7, 10, 32, 7, BUTTON_CENTRAL_IMAGE, BUTTON_CENTRAL_IMAGE, 5, 5);
  else air.changeScale(45, 50, 90, 45, BUTTON_CENTRAL_IMAGE, BUTTON_CENTRAL_IMAGE, 5, 10);

  while(TRUE)
    {
#ifndef NO_IWDG
      fwdgt_counter_reload();
#endif

      // экраны различных режимов работы пульта
      activitySearch();                                                       // экран ожидания подключения к блоку управления
      activityError();                                                        // экран индикации неисправностей
      activityAbout();                                                        // экран информации о производителе
      activityAir();                                                          // экран управления воздушным отоплением
      activitySetup();                                                        // экран настройки подогревателя и пульта
      activityStartTimers();                                                  // экран настройки таймеров запуска
      activitySetupClock();                                                   // экран настройки текущего времени
      activitySleep();                                                        // экран спящего режима
      // задачи, независимые от режима работы пульта
      handlerSleep();                                                         // проверка входа в спящий режим
      handlerSensor();                                                        // проверка сенсорного экрана
      handlerUsart();                                                         // передача и прием по последовательному каналу
      can.handler();
      rvc.handler();
      handlerTemperature();                                                   // получение данных температуры
      handlerClock();                                                         // обновление текущего времени
      handlerTimer();                                                         // отсчет ограничений по времени работы
      minuteHandler();
			if (display.setup.scheduleMode&1)
				air.checkDayNight();                                                    // проверка дневного/ночного режима
      handlerBluetooth();
      if (isTest)
        usart.linkCnt=0;

    }
}
//-----------------------------------------------------
void initAll(void)
{

  SystemInit();
  clock.initialize();
  rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV2);
  core.remapTable();
  __enable_irq();
  core.initialize();
  //***********************************
  ///*
#ifndef NO_IWDG
  // enable IRC40K
  rcu_osci_on(RCU_IRC40K);
  // wait till IRC40K is ready
  while(SUCCESS != rcu_osci_stab_wait(RCU_IRC40K))
    {
    }
  // confiure FWDGT counter clock: 40KHz(IRC40K) / 64 = 0.625 KHz
  fwdgt_config(0x0FFF,FWDGT_PSC_DIV64);
  // After 1.6 seconds to generate a reset
  fwdgt_enable();
#endif
  //*/
  //***********************************
  display.initialize();
  //***********************************
  can.initialize();
  //***********************************
  usart.initialize();
  //***********************************
  temperature.initialize();
  //***********************************
  memory.initializeQuad();
  memory.lockMemory(true);
  //***********************************
  slider.initialize();
  //***********************************
  sensor.initialize();
  //***********************************
  bluetooth.initialize((char*)"Timberline 1.5");
}
//-----------------------------------------------------
void activitySearch(void)
{
  uint8_t result;

  if (hcu.isConnect == hcu.CONNECT_STATUS_SEARCH)
    {
      if (screen_visible != SCREEN_VISIBLE_SEARCH)
        {
          screen_visible_old = screen_visible;
          screen_visible = SCREEN_VISIBLE_SEARCH;
          search.viewScreen();
        }
    }
  else if (hcu.isConnect == hcu.CONNECT_STATUS_OK)
    {
      if (screen_visible == SCREEN_VISIBLE_SEARCH)
        {
          screen_visible = screen_visible_old;
          switch(screen_visible)
            {
            case SCREEN_VISIBLE_AIR:
              air.viewScreen(true);
              break;
            default:
              screen_visible = SCREEN_VISIBLE_AIR;
              air.viewScreen(true);
              break;
            }
        }
    }

  if (screen_visible == SCREEN_VISIBLE_SEARCH)                              // экран ожидания подключения к блоку управления
    {
      result = search.viewHandler();
      switch(result)
        {
        case 1:     //
          break;
        case 2:     //
          break;
        }
    }
}
//-----------------------------------------------------
void activityError(void)
{
  uint8_t result;

  if (hcu.faultCode != error.codeOld)
    {
      //error.codeOld = hcu.error;
      if (hcu.faultCode != 0)
        {
          if (screen_visible != SCREEN_VISIBLE_ERROR)
            {
              if (screen_visible == SCREEN_VISIBLE_SLEEP)
                {
                  sleep.setLowPower(false);
                  clock.sensorClock = clock.sensorClockOld;
                  display.switchOn();
                  display.setLight(display.setup.brightness);
                  screen_visible = screen_visible_old;
                }
              screen_visible_old = screen_visible;
              screen_visible = SCREEN_VISIBLE_ERROR;
              error.viewScreen();
              air.isFHeaterOn = false;
            }
        }
    }
  else if (hcu.faultCode == 0)
    {
      if (screen_visible == SCREEN_VISIBLE_ERROR)
        {
          screen_visible = screen_visible_old;
          switch(screen_visible)
            {
            case SCREEN_VISIBLE_AIR:
              air.viewScreen(true);
              break;
            default:
              screen_visible = SCREEN_VISIBLE_AIR;
              air.viewScreen(true);
              break;
            }
        }
    }

  if (screen_visible == SCREEN_VISIBLE_ERROR)                              // экран индикации неисправностей
    {
      result = error.viewHandler();
      switch(result)
        {
        case 1:     //
          screen_visible = screen_visible_old;
          switch(screen_visible)
            {
            case SCREEN_VISIBLE_AIR:
              air.viewScreen(true);
              break;
            case SCREEN_VISIBLE_SEARCH:
              search.viewScreen();
              break;
            default:
              if(hcu.faultCode == 20)
                {
                  screen_visible = SCREEN_VISIBLE_SEARCH;
                  search.viewScreen();
                }
              else
                {
                  screen_visible = SCREEN_VISIBLE_AIR;
                  air.viewScreen(true);
                }
              break;
            }
          break;
        case 2:     //
          break;
        }
    }
}
//-----------------------------------------------------
void activityAbout(void)
{
  uint8_t result;

  if (screen_visible == SCREEN_VISIBLE_ABOUT)                              // экран индикации неисправностей
    {
      result = about.viewHandler();
      switch(result)
        {
        case 1:     //
          screen_visible = SCREEN_VISIBLE_AIR;
          air.viewScreen(true);
          break;
        case 2:     //
          break;
        }
    }
}
//-----------------------------------------------------
void activityAir(void)
{
  uint8_t result;
  //bool res;

  if (screen_visible == SCREEN_VISIBLE_AIR)                              // экран управления воздушным отоплением
    {
      //res = false;
      if ((core.getTick() - hcu.lockTimer) <= 1000)
        {
          if (sensor.status == 0)
            {
              if (air.isAirOn)
                {
                  // включено, передвинуть ползунок на уставку
                  if (slider.position == 0)
                    {
                      slider.setPosition(hcu.airHeaterTSetPoint[(air.isDay|air.isSelectDay)&(!air.isSelectNight)]);
                    }
                }
              else
                {
                  // отключено, передвинуть ползунок влево
                  if (slider.position != 0)
                    {
                      slider.position = 0;
                    }
                }
            }
        }
      result = air.viewHandler();
      switch(result)
        {
        case 1:     // about
          screen_visible_old = screen_visible;
          screen_visible = SCREEN_VISIBLE_ABOUT;
          about.viewScreen();
          break;
        case 2:     // настройки
          screen_visible_old = screen_visible;
          screen_visible = SCREEN_VISIBLE_SETUP;
          setup.viewScreen();
          break;
        }
    }
}
//-----------------------------------------------------
void activitySetup(void)
{
  uint8_t result;

  if (screen_visible == SCREEN_VISIBLE_SETUP)                             // экран настройки подогревателя и пульта
    {
      result = setup.handler();
      switch(result)
        {
        case 1:
          screen_visible = screen_visible_old;
          switch(screen_visible)
            {
            case SCREEN_VISIBLE_AIR:
              air.viewScreen(true);
              break;
            default:
              screen_visible = SCREEN_VISIBLE_AIR;
              air.viewScreen(true);
              break;
            }
          writeSetup();
          break;
        case 2:
          screen_visible_old = screen_visible;
          screen_visible = SCREEN_VISIBLE_SETUP_CLOCK;
          clock.sensorClock = clock.SENSOR_HOUR;
          setupClock.viewScreen();
          break;
        }
      /**/
    }
  else
    {
      display.setup.viewClock &= ~2;
      display.setup.celsius &= ~2;
      display.setup.fahrenheit &= ~2;
      display.setup.h12 &= ~2;
      air.isPanelSensor &= ~2;

      hcu.fanAuto &= ~2;
      hcu.fanManual &= ~2;
    }
}
//-----------------------------------------------------
void activityStartTimers(void)
{
  uint8_t result;

  if (screen_visible == SCREEN_VISIBLE_START_TIMERS)                      // экран настройки таймеров запуска
    {
      result = startTimers.handler();
      switch(result)
        {
        case 1:
          screen_visible = screen_visible_old;
          switch(screen_visible)
            {
            case SCREEN_VISIBLE_AIR:
              air.viewScreen(true);
              break;
            default:
              screen_visible = SCREEN_VISIBLE_AIR;
              air.viewScreen(true);
              break;
            }
          break;
        }
    }
}
//-----------------------------------------------------
void activitySetupClock(void)
{
  uint8_t result;

  if (screen_visible == SCREEN_VISIBLE_SETUP_CLOCK)                       // экран настройки текущего времени
    {
      clock.second = 0;
      clock.isWork = FALSE;
      result = setupClock.handler();
      switch(result)
        {
        case 1:
          screen_visible = screen_visible_old;
          switch(screen_visible)
            {
            case SCREEN_VISIBLE_AIR:
              air.viewScreen(true);
              break;
            case SCREEN_VISIBLE_SETUP:
              setup.viewScreen();
              break;
            default:
              screen_visible = SCREEN_VISIBLE_AIR;
              air.viewScreen(true);
              break;
            }
          clock.timeRegulate();
          clock.isWork = TRUE;
          break;
        }
    }
}
//-----------------------------------------------------
void activitySleep(void)
{
  uint8_t result;

  if (screen_visible == SCREEN_VISIBLE_SLEEP)                             // экран спящего режима
    {
      result = sleep.handler();
      switch(result)
        {
        case 1: // выход из спящего режима с показом часов
          sleep.setLowPower(false);
          clock.sensorClock = clock.sensorClockOld;
          display.switchOn();
          display.setLight(display.setup.brightness);
          screen_visible = screen_visible_old;
          if (sleep.isViewClock == true)
            {
              if (screen_visible == SCREEN_VISIBLE_AIR) air.viewScreen(true);       // показываем тот экран, который был до отключения
              else if (screen_visible == SCREEN_VISIBLE_SETUP) setup.viewScreen();
              else if (screen_visible == SCREEN_VISIBLE_START_TIMERS) startTimers.viewScreen();
              else
                {
                  screen_visible = SCREEN_VISIBLE_AIR;
                  screen_visible_old = screen_visible;
                  air.viewScreen(true);
                }
            }
          else
            {
              sensor.touch = 1;       // исключаем нажатие элементов управления
            }
          break;
        }
    }
}

void activityBleConnect(void)
{
  uint8_t result;

  if (screen_visible == SCREEN_VISIBLE_BLE_CONNECT)                             // экран запроса подключения по Bluetooth
    {
      result = bleConnect.viewHandler();

      switch(result)
        {
        case 1: // подтверждение
          isBleAccept = true;
          isBleCancel = false;
          timerCancel = core.getTick();
          addNewId(newId); // было ///
          isBleSendKey = true;
          screen_visible = screen_visible_old;
          if (screen_visible == SCREEN_VISIBLE_WORK) air.viewScreen(1);       // показываем тот экран, который был до отключения
          else if (screen_visible == SCREEN_VISIBLE_SETUP) setup.viewScreen();
          else
            {
              screen_visible = SCREEN_VISIBLE_WORK;
              screen_visible_old = screen_visible;
              air.viewScreen(1);
            }
          break;
        case 2: // отмена
          isBleAccept = false;
          isBleCancel = true;
          timerCancel = core.getTick();
          screen_visible = screen_visible_old;
          if (screen_visible == SCREEN_VISIBLE_WORK) air.viewScreen(1);       // показываем тот экран, который был до отключения
          else if (screen_visible == SCREEN_VISIBLE_SETUP) setup.viewScreen();
          else
            {
              screen_visible = SCREEN_VISIBLE_WORK;
              screen_visible_old = screen_visible;
              air.viewScreen(1);
            }
          break;
        }
    }
}

//-----------------------------------------------------
void handlerSleep(void)
{
  if (screen_visible != SCREEN_VISIBLE_SLEEP)
    {
      if (sleep.checkSleepMode())                                             // проверка и вход в спящий режим
        {
          if (screen_visible!=SCREEN_VISIBLE_SETUP_CLOCK)
            {
              screen_visible_old = screen_visible;
              screen_visible = SCREEN_VISIBLE_SLEEP;
              sleep.viewScreen();
            }
        }
    }
}
//-----------------------------------------------------
void handlerSensor(void)
{
  uint8_t result;
  if (screen_visible != SCREEN_VISIBLE_SLEEP)
    {
      result = sensor.handler();                                              // проверка сенсорного экрана
      if (result)                                                             // было касание сенсорного экрана
        {
          if (sensor.status) display.setTimer(display.setup.timeout*1000);
        }
    }
}
//-----------------------------------------------------
void handlerUsart(void)
{
  hcu.handler();
}
//-----------------------------------------------------
void handlerTemperature(void)
{
  static uint32_t timer = 0;

  if ((core.getTick()-timer) > 100)
    {
      timer = core.getTick();
      temperature.request();                                                  // измерение температуры воздуха
    }
}
//-----------------------------------------------------
void handlerClock(void)
{
  clock.handler();
}
//-----------------------------------------------------
void handlerTimer(void)
{
  if (air.isWaterOn && (hcu.durationDomesticWater < 300))
    {
      if ((core.getTick() - hcu.timerOffDomesticWater) > (hcu.durationDomesticWater*60000))
        {
          // Time is up
          air.isWaterOn = false;
        }
    }
  if ((air.isFHeaterOn || air.isEHeaterOn) && (hcu.durationSystem < 7200))
    {
      if ((core.getTick() - hcu.timerOffSystem) > (hcu.durationSystem*60000))
        {
          // Time is up
          air.isFHeaterOn = false;
          air.isEHeaterOn = false;
        }
    }
}
//-----------------------------------------------------
void addNewId(uint8_t *id)
{
  uint32_t a, N;
  uint8_t i, x, array[256+16+16+64];
  bool isEmpty;

  fmc_unlock();
  while(fmc_flag_get(FMC_FLAG_BANK0_BUSY) == SET)
    {
    }
  for (i=0; i<16; i++)
    {
      isEmpty = true;
      for (x=0; x<16; x++)
        {
          if (*(__IO uint8_t*)(BLE_ID_ADDRESS+16+16+i*16+x) != 0xFF) isEmpty = false;
        }
      if (isEmpty == true)
        {
          for (a=0; a<8; a+=4)
            {
              N = id[0+a];
              N += id[1+a]<<8;
              N += id[2+a]<<16;
              N += id[3+a]<<24;
              fmc_word_program(BLE_ID_ADDRESS+16+16+i*16+a, N);
            }
          fmc_lock();
          return;
        }
    }
  for (a=0; a<(256+16+16+64); a++)
    {
      array[a] = *(__IO uint8_t*)(BLE_ID_ADDRESS+a);
    }
  fmc_page_erase(BLE_ID_ADDRESS);
  while(fmc_flag_get(FMC_FLAG_BANK0_BUSY) == SET)
    {
    }
  for (a=32; a<(240+16+16); a++)
    {
      array[a] = array[a+16];
    }
  for (a=0; a<8; a++)
    {
      array[240+a+16+16] = id[a];
    }
  for (a=0; a<(256+16+16+64); a+=4)
    {
      N = array[0+a];
      N += array[1+a]<<8;
      N += array[2+a]<<16;
      N += array[3+a]<<24;
      fmc_word_program(BLE_ID_ADDRESS+a, N);
    }
  fmc_lock();
}
//-----------------------------------------------------
void setNewId(void)
{
  uint32_t a, N;
  uint8_t i, x, array[256+16+16+64];

  if (*(__IO uint8_t*)(BLE_ID_ADDRESS+5) == ' ' &&
      *(__IO uint8_t*)(BLE_ID_ADDRESS+6) == ' ' &&
      *(__IO uint8_t*)(BLE_ID_ADDRESS+7) == ' ')
    {

      fmc_unlock();
      while(fmc_flag_get(FMC_FLAG_BANK0_BUSY) == SET)
        {
        }
      for (a=0; a<(256+16+16+64); a++)
        {
          array[a] = *(__IO uint8_t*)(BLE_ID_ADDRESS+a);
        }
      fmc_page_erase(BLE_ID_ADDRESS);
      while(fmc_flag_get(FMC_FLAG_BANK0_BUSY) == SET)
        {
        }
      // ID
      for (a=0; a<3; a++)
        {
          x = 0;
          for (i=0; i<2; i++)
            {
              x |= temperature.array[a*2+i] & 0x0F;
              x = x<<4;
            }
          x = (x+0x20) & 0x3F;
          array[a+5] = x;
        }
      // KEY
      for (a=0; a<8; a++)
        {
          x = 0;
          for (i=0; i<4; i++)
            {
              x |= temperature.array[a*2+i] & 0x03;
              x = x<<2;
            }
          array[a+16] = x;
        }
      for (a=0; a<(256+16+16+64); a+=4)
        {
          N = array[0+a];
          N += array[1+a]<<8;
          N += array[2+a]<<16;
          N += array[3+a]<<24;
          fmc_word_program(BLE_ID_ADDRESS+a, N);
        }
      fmc_lock();
    }
}
//-----------------------------------------------------
void setNewKey(void)
{
  uint32_t a, N;
  uint8_t i;


  for (a=0; a<8; a++)
    {
      if (*(__IO uint8_t*)(BLE_ID_ADDRESS+8+a) != 0xFF)
        {
          return;
        }
    }

  fmc_unlock();
  while(fmc_flag_get(FMC_FLAG_BANK0_BUSY) == SET)
    {
    }
  for (a=0; a<2; a++)
    {
      N = 0;
      for (i=0; i<32; i++)
        {
          N |= temperature.array[a*6+i] & 0x01;
          N = N<<1;
        }
      fmc_word_program(BLE_ID_ADDRESS+8+a*4, N);
    }
  fmc_lock();
}
//-----------------------------------------------------
void writeSetup(void)
{
  uint32_t a, N;
  uint8_t x, array[256+16+64+16];

  fmc_unlock();
  while(fmc_flag_get(FMC_FLAG_BANK0_BUSY) == SET)
    {
    }
  for (a=0; a<(256+16+16+64); a++)
    {
      array[a] = *(__IO uint8_t*)(BLE_ID_ADDRESS+a);
    }
  fmc_page_erase(BLE_ID_ADDRESS);
  while(fmc_flag_get(FMC_FLAG_BANK0_BUSY) == SET)
    {
    }

  array[256+16+16+0] = display.setup.brightness;
  array[256+16+16+1] = display.setup.timeout & 0xFF;
  array[256+16+16+2] = (display.setup.timeout>>8) & 0xFF;
  array[256+16+16+3] = display.setup.viewClock;
  array[256+16+16+4] = display.setup.displayHeater;
  array[256+16+16+5] = display.setup.displayEngine;
  array[256+16+16+6] = display.setup.displaySleep;
  array[256+16+16+7] = display.setup.language;
  array[256+16+16+8] = display.setup.celsius;
  array[256+16+16+9] = display.setup.fahrenheit;
  array[256+16+16+10] = display.setup.h12;

  array[256+16+16+11] = air.isPanelSensor;
  array[256+16+16+12] = hcu.fanAuto;
  array[256+16+16+13] = hcu.fanManual;
  array[256+16+16+14] = hcu.fanPower;
  array[256+16+16+15] = hcu.pumpOn;
  array[256+16+16+16] = hcu.durationDomesticWater & 0xFF;
  array[256+16+16+17] = (hcu.durationDomesticWater>>8) & 0xFF;
  array[256+16+16+18] = hcu.durationSystem & 0xFF;
  array[256+16+16+19] = (hcu.durationSystem>>8) & 0xFF;

  array[256+16+16+20] = air.dayTimeH;
  array[256+16+16+21] = air.dayTimeM;
  array[256+16+16+22] = air.nightTimeH;
  array[256+16+16+23] = air.nightTimeM;

  array[256+16+16+24] = core.calibrationValue & 0xFF;
  array[256+16+16+25] = (core.calibrationValue>>8) & 0xFF;

  array[256+16+16+26] = hcu.airHeaterTSetPoint[0];
  array[256+16+16+27] = hcu.airHeaterTSetPoint[1];
	array[256+16+16+28] = display.setup.scheduleMode&1;
	array[256+16+16+29] = display.setup.tempShift;

  x = 0;
  for (a=0; a<63; a++)
    {
      x += array[256+16+16+a];
    }
  array[256+16+16+63] = x;       // контрольная сумма

  for (a=0; a<(256+16+16+64); a+=4)
    {
      N = array[0+a];
      N += array[1+a]<<8;
      N += array[2+a]<<16;
      N += array[3+a]<<24;
      fmc_word_program(BLE_ID_ADDRESS+a, N);
    }
  fmc_lock();
}
//-----------------------------------------------------
void readSetup(void)
{
  uint8_t x, array[64], a;
  bool result = false;

  a = 0;
  for (x=0; x<63; x++)
    {
      array[x] = *(__IO uint8_t*)(BLE_ID_ADDRESS+256+16+16+x);
      a += array[x];
    }
  if (a == *(__IO uint8_t*)(BLE_ID_ADDRESS+256+16+16+63))
    {
      result = true;
    }
  if (result == true)
    {
      display.setup.brightness = array[0];
      display.setup.timeout = array[1] + array[2]*256;
      display.setup.viewClock = array[3];
      display.setup.displayHeater = array[4];
      display.setup.displayEngine = array[5];
      display.setup.displaySleep = array[6];
      display.setup.language = array[7];
      display.setup.celsius = array[8];
      display.setup.fahrenheit = array[9];
      display.setup.h12 = array[10];

      air.isPanelSensor = array[11];
      hcu.fanAuto = array[12];
      hcu.fanManual = array[13];
      hcu.fanPower = array[14];
      hcu.pumpOn = array[15];
      hcu.durationDomesticWater = array[16] + array[17]*256;
      hcu.durationSystem = array[18] + array[19]*256;

      air.dayTimeH = array[20];
      air.dayTimeM = array[21];
      air.nightTimeH = array[22];
      air.nightTimeM = array[23];

      core.calibrationValue = array[24] + array[25]*256;

      hcu.airHeaterTSetPoint[0] = array[26];
      hcu.airHeaterTSetPoint[1] = array[27];
			
			display.setup.scheduleMode = array[28];
			display.setup.tempShift = array[29];

      if (display.setup.brightness < 10 || display.setup.brightness > 100) result = false;
      if (display.setup.timeout < 20 || display.setup.timeout > 300) result = false;
      if (display.setup.viewClock > 3) result = false;
      if (display.setup.displayHeater > 3) result = false;
      if (display.setup.displayEngine > 3) result = false;
      if (display.setup.displaySleep > 3) result = false;
      if (display.setup.celsius > 3) result = false;
      if (display.setup.fahrenheit > 3) result = false;
      if (display.setup.h12 > 3) result = false;

      if (air.isPanelSensor > 3) result = false;
      if (hcu.fanAuto > 3) result = false;
      if (hcu.fanManual > 3) result = false;
      if (hcu.fanPower > 100) result = false;
      if (hcu.pumpOn > 3) result = false;
      if (hcu.durationDomesticWater < 30 || hcu.durationDomesticWater > 60) result = false;
      if (hcu.durationSystem < 10 || hcu.durationSystem > 21600) result = false;

      if (air.dayTimeH > 23) result = false;
      if (air.dayTimeM > 59) result = false;
      if (air.nightTimeH > 23) result = false;
      if (air.nightTimeM > 59) result = false;
			if (display.setup.scheduleMode>1) result = false;
			if (display.setup.tempShift > 10 || display.setup.tempShift < -10) result = false;

      if (display.setup.celsius & 0x01)
        {
          if (hcu.airHeaterTSetPoint[0] < 7 || hcu.airHeaterTSetPoint[0] > 32) result = false;
          if (hcu.airHeaterTSetPoint[1] < 7 || hcu.airHeaterTSetPoint[1] > 32) result = false;
        }
      else
        {
          if (hcu.airHeaterTSetPoint[0] < 45 || hcu.airHeaterTSetPoint[0] > 90) result = false;
          if (hcu.airHeaterTSetPoint[1] < 45 || hcu.airHeaterTSetPoint[1] > 90) result = false;
        }
      if (core.calibrationValue > 720 || core.calibrationValue < -720) result  = false;
    }
  if (result == false)
    {
      display.setup.brightness = 100;
      display.setup.timeout = 90;
      display.setup.viewClock = 1;
      display.setup.displayHeater = 0;
      display.setup.displayEngine = 0;
      display.setup.displaySleep = 1;
      display.setup.resolution = 3;
      display.setup.language = 0;
      display.setup.celsius = 0;
      display.setup.fahrenheit = 1;
      display.setup.h12 = 1;

      air.isPanelSensor = 0;
      hcu.fanAuto = 1;
      hcu.fanManual = 0;
      hcu.fanPower = 50;
      hcu.pumpOn = 0;
      hcu.durationDomesticWater = 30;
      hcu.durationSystem = 7200;

      air.dayTimeH = 7;
      air.dayTimeM = 0;
      air.nightTimeH = 22;
      air.nightTimeM = 0;

      if (display.setup.celsius)
        {
          hcu.airHeaterTSetPoint[0] = 22;
          hcu.airHeaterTSetPoint[1] = 22;
        }
      else
        {
          hcu.airHeaterTSetPoint[0] = core.celToFar(22);
          hcu.airHeaterTSetPoint[1] = core.celToFar(22);
        }
      core.calibrationValue = 0;
			display.setup.scheduleMode = 0;
			display.setup.tempShift = 0;
      writeSetup();
    }
}

void minuteHandler()
{
  static uint32_t lastTick=0;
  if (core.getTick()-lastTick>60000)
    {
      lastTick+=60000;
      canPGNRVC.minutesSinceStart++;
    }
}
//-----------------------------------------------------
void handlerBluetooth(void)
{
  /*
    uint8_t group;
    uint8_t buf[20], i, x;
    bool isAnswer = false;
    static uint8_t heater_device_old = 0xFF;
    static uint8_t dataNumb=0;
    static uint8_t fastSendCount=0;

    static uint32_t timer1000=0;

  bluetooth.handler();
    if ((core.getTick()-timer1000) > 1000){
        timer1000 = core.getTick();
        counterBle = counterBleTemp;
        counterBleTemp = 0;
        counterBleSend = counterBleSendTemp;
        counterBleSendTemp = 0;
    }
    counterBleTemp++;
   */
}

