#include "CSettings.h"
#include <EEPROM.h>

CSettings::CSettings()
{
  m_batWarning = 1; //on by default
  m_batMAH[0] = 1300; //1300 mAh by default
  m_batMAH[1] = 1500; //1500 mAh by default
  m_batMAH[2] = 1800; //1800 mAh by default
  m_batMAH[3] = 2250; //2250 mAh by default
  m_activeBattery = 0;
  m_batWarningPercent = 23; //23% by default
  FixBatWarning();
  m_DVchannel = 0; //AUX1 default
  m_tempUnit = 0; //°C default
  m_lastMAH = 0;
  m_fontSize = 1;
  m_displaySymbols = 1;
  m_goggle = 0;
  m_vTxChannel = 4; // Raceband 5 @ 25mW default
  m_vTxBand = 4;
  m_vTxPower = 0;
  m_xOffset = 0; // Center OSD offsets
  m_yOffset = 0;
  
  /*itemPositions.nick[0] = m_COLS/2 - 4;
  itemPositions.nick[1] = -1;
  itemPositions.time[0] = COLS/2 - 3;
  itemPositions.time[1] = -2;
  itemPositions.voltage[0] = 0;
  itemPositions.voltage[1] = -1;
  itemPositions.mAh[0] = -1; //FIXME
  itemPositions.mAh[1] = -1;
  itemPositions.amps[0] = -1; //FIXME
  itemPositions.amps[1] = 0;
  itemPositions.throttle[0] = 0;
  itemPositions.throttle[1] = 0;*/
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
  if(EEPROM.read(0x01) < 0x05) //first start of OSD
  //if(true)
  {
    WriteSettings(); //write defaults
    EEPROM.write(0x01,0x05);
  }
  else
  {
    m_batWarningMAH = ReadInt16_t(0x03,0x04);
    m_batWarning = EEPROM.read(0x05);
    m_activeBattery = EEPROM.read(0x06);
    m_DVchannel = EEPROM.read(0x07);
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
    m_fontSize = EEPROM.read(pos);
    pos++;
    m_displaySymbols = EEPROM.read(pos);
    pos++;
    m_goggle = EEPROM.read(pos);
    pos++;
    m_vTxChannel = EEPROM.read(pos);
    pos++;
    m_vTxBand = EEPROM.read(pos);
    pos++;
    m_vTxPower = EEPROM.read(pos);
    pos++;
    m_xOffset = EEPROM.read(pos);
    pos++;
    m_yOffset = EEPROM.read(pos);
    
    m_lastMAH = ReadInt16_t(100, 101);
  }
  FixBatWarning();
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
  EEPROM.write(pos, (byte)m_batWarningPercent); 
  pos++;
  EEPROM.write(pos, (byte)m_tempUnit);
  pos++;
  EEPROM.write(pos, (byte)m_fontSize);
  pos++;
  EEPROM.write(pos, (byte)m_displaySymbols);
  pos++;
  EEPROM.write(pos, (byte)m_goggle);
  pos++;
  EEPROM.write(pos, (byte)m_vTxChannel);
  pos++;
  EEPROM.write(pos, (byte)m_vTxBand);
  pos++;
  EEPROM.write(pos, (byte)m_vTxPower);
  pos++;
  EEPROM.write(pos, (byte)m_xOffset);
  pos++;
  EEPROM.write(pos, (byte)m_yOffset);
}

void CSettings::FixBatWarning()
{
  uint32_t temp = ((uint32_t)m_batMAH[m_activeBattery] * (uint32_t)m_batWarningPercent) / (uint32_t)100;
  m_batWarningMAH = m_batMAH[m_activeBattery] - (int16_t)temp;
  m_batSlice = m_batMAH[m_activeBattery] / 8;
}

void CSettings::WriteLastMAH()
{
  WriteInt16_t(100, 101, m_lastMAH);
}
