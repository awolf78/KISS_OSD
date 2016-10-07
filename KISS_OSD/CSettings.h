#include <Arduino.h>
#include "Flash.h"

#ifndef CSettingsh
#define CSettingsh

class CSettings
{
  public:
  CSettings();
  ~CSettings();
  void ReadSettings();
  void WriteSettings();
  void FixBatWarning();
  
  uint8_t m_batWarning; // 0 = off, 1 = on
  int16_t m_batMAH[4]; // 300-32000 mAh
  int16_t m_batWarningMAH; // 300-32000 mAh
  uint8_t m_activeBattery; // 0-4
  uint8_t m_batWarningPercent; // battery capacity percentage -> if below, warning will get displayed
  uint8_t m_DVchannel; // AUX1-AUX4
  uint8_t m_tempUnit; //°C or °F
  
  private:
  int16_t ReadInt16_t(byte lsbPos, byte msbPos);
  void WriteInt16_t(byte lsbPos, byte msbPos, int16_t value);
};

#endif
