#include "CSettings.h"
#include <EEPROM.h>

CSettings::CSettings()
{
  m_batWarning = 1; //on by default
  int16_t i;
  for(i=0; i<4; i++)
  {
    m_batMAH[i] = 1300; //1300 mAh by default
  }
  m_activeBattery = 0;
  m_batWarningPercent = 23; //23% by default
  m_batWarningMAH = 1000;
  m_DVchannel = 0; //AUX1 default
  m_tempUnit = 0; //°C default
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
  return value;
}

void CSettings::ReadSettings()
{
  if(EEPROM.read(0x01) < 0x02) //first start of OSD
  //if(true)
  {
    WriteSettings(); //write defaults
    EEPROM.write(0x01,0x02);
  }
  else
  {
    m_batWarningMAH = ReadInt16_t(0x03,0x04);
    m_batWarning = (int16_t) EEPROM.read(0x05);
    m_activeBattery = (int16_t) EEPROM.read(0x06);
    m_DVchannel = (int16_t) EEPROM.read(0x07);
    uint8_t i;
    byte pos = 0x08;
    for(i=0; i < 4; i++)
    {
      m_batMAH[i] = ReadInt16_t(pos, pos+1);
      pos += 2;
    }
    m_batWarningPercent = EEPROM.read(pos);
    pos++;
    m_tempUnit = EEPROM.read(pos);
    pos++;
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
  WriteInt16_t(0x03,0x04,m_batWarningMAH);
  EEPROM.write(0x05,(byte)m_batWarning);
  EEPROM.write(0x06,(byte)m_activeBattery);
  EEPROM.write(0x07,(byte)m_DVchannel);
  uint8_t i;
  byte pos = 0x08;
  for(i=0; i < 4; i++)
  {
    WriteInt16_t(pos, pos+1, m_batMAH[i]);
    pos += 2;
  }
  EEPROM.write(pos, m_batWarningPercent); 
  pos++;
  EEPROM.write(pos, m_tempUnit); 
}

void CSettings::FixBatWarning()
{
  uint32_t temp = ((uint32_t)m_batMAH[m_activeBattery] * (uint32_t)m_batWarningPercent) / (uint32_t)100;
  m_batWarningMAH = m_batMAH[m_activeBattery] - (int16_t)temp;
}
