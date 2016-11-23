#include <Arduino.h>
#include "Flash.h"

#ifndef CSettingsh
#define CSettingsh

/*struct ItemPos
{
  public:
  int8_t nick[2];
  int8_t time[2];
  int8_t voltage[2];
  int8_t mAh[2];
  int8_t amps[2];
  int8_t throttle[2];
};

static ItemPos itemPositions;*/

class CSettings
{
  public:
  CSettings();
  void ReadSettings();
  void WriteSettings();
  void FixBatWarning();
  void WriteLastMAH();
  
  volatile uint8_t m_batWarning; // 0 = off, 1 = on
  volatile int16_t m_batMAH[4]; // 300-32000 mAh
  volatile int16_t m_batWarningMAH; // 300-32000 mAh
  volatile int16_t m_batSlice; // calculated for battery symbol
  volatile uint8_t m_activeBattery; // 0-4
  volatile uint8_t m_batWarningPercent; // battery capacity percentage -> if below, warning will get displayed
  volatile uint8_t m_DVchannel; // AUX1-AUX4
  volatile uint8_t m_tempUnit; //°C or °F
  volatile int16_t m_lastMAH; // mAh value from previous run
  volatile uint8_t m_fontSize; // 0 = normal font; 1 = large font
  volatile uint8_t m_displaySymbols; // 0 = no; 1 = yes (symbols such as battery and timer)
  
  private:
  int16_t ReadInt16_t(byte lsbPos, byte msbPos);
  void WriteInt16_t(byte lsbPos, byte msbPos, int16_t value);
};

#endif
