#ifndef CSettingsh
#define CSettingsh
#include <EEPROM.h>
#include <Arduino.h>

class CSettings
{
  public:
  CSettings();
  ~CSettings();
  void ReadSettings();
  void WriteSettings();
  
  int16_t m_aspectRatio; //4:3 or 16:9 etc.
  int16_t m_batWarning; // 0 = off, 1 = on
  int16_t m_bat_mAh_warning; // 300-32000 mAh
  int16_t m_BAT_AUX_DV_CHANNEL; // AUX1-AUX4
  int16_t m_DVchannel; // AUX1-AUX4
  static const uint8_t MAX_DISPLAY_ITEMS = 10; //Order in which the OSD items are displayed using the defined DV control
  char m_displayOrder[MAX_DISPLAY_ITEMS][20];
  
  private:
  char m_defaultOrder[MAX_DISPLAY_ITEMS][20];
  int16_t ReadInt16_t(byte lsbPos, byte msbPos);
  void WriteInt16_t(byte lsbPos, byte msbPos, int16_t value);
};

#endif
