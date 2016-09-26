#include "CSettings.h"

CSettings::CSettings()
{
  m_aspectRatio = 0; //4:3 as standard
  m_batWarning = 1; //on by default
  m_bat_mAh_warning = 1000; // 1000 mAh default
  m_BAT_AUX_DV_CHANNEL = m_DVchannel = 1; //AUX2 default
  strcpy(m_displayOrder[0], "lipo voltage");
  strcpy(m_displayOrder[1], "ma consumption");
  strcpy(m_displayOrder[2], "statistics");
  strcpy(m_displayOrder[3], "timer");
  strcpy(m_displayOrder[4], "nickname");
  strcpy(m_displayOrder[5], "total current");
  strcpy(m_displayOrder[6], "esc current");
  strcpy(m_displayOrder[7], "esc rpm");
  strcpy(m_displayOrder[8], "throttle %");
  strcpy(m_displayOrder[9], "esc temperature");
  uint8_t i;
  for(i=0; i < MAX_DISPLAY_ITEMS; i++)
  {
    strcpy(m_defaultOrder[i], m_displayOrder[i]);
  }
}

CSettings::~CSettings()
{
}

int16_t CSettings::ReadInt16_t(byte lsbPos, byte msbPos)
{
  byte msb, lsb;
  
  lsb = EEPROM.read(lsbPos);
  msb = EEPROM.read(msbPos);
  
  int16_t value = msb;
  value = value << 8;
  value |= lsb;
}

void CSettings::ReadSettings()
{
  if(EEPROM.read(0x01) == 0x00) //first start of OSD
  {
    WriteSettings(); //write defaults
    EEPROM.write(0x01,0x01);
  }
  else
  {
    m_bat_mAh_warning = ReadInt16_t(0x03,0x04);
    m_aspectRatio = (int16_t) EEPROM.read(0x05);
    m_batWarning = (int16_t) EEPROM.read(0x06);
    m_BAT_AUX_DV_CHANNEL = (int16_t) EEPROM.read(0x07);
    m_DVchannel = (int16_t) EEPROM.read(0x08);
    uint8_t i;
    byte pos = 0x09;
    for(i = 0; i < MAX_DISPLAY_ITEMS; i++)
    {
      strcpy(m_displayOrder[EEPROM.read(pos)], m_defaultOrder[i]);
      pos++;
    }
  }
}

void CSettings::WriteInt16_t(byte lsbPos, byte msbPos, int16_t value)
{
  byte msb, lsb;
  lsb = (byte)(value & 0x00FF);
  msb = (byte)((value & 0xFF00) >> 8);
  
  EEPROM.write(lsbPos, lsb); // LSB
  EEPROM.write(msbPos, msb); // MSB
}

void CSettings::WriteSettings()
{
  WriteInt16_t(0x03,0x04,m_bat_mAh_warning);
  EEPROM.write(0x05,(byte)m_aspectRatio);
  EEPROM.write(0x06,(byte)m_batWarning);
  EEPROM.write(0x07,(byte)m_BAT_AUX_DV_CHANNEL);
  EEPROM.write(0x08,(byte)m_DVchannel);
  uint8_t i;
  byte pos = 0x09;
  for(i = 0; i < MAX_DISPLAY_ITEMS; i++)
  {
    uint8_t j;
    for(j = 0; j < MAX_DISPLAY_ITEMS; j++)
    {
      if(strcmp(m_defaultOrder[i], m_displayOrder[j]) == 0)
      {
        break;
      }
    }
    EEPROM.write(pos,(byte)j);
    pos++;
  }
}
