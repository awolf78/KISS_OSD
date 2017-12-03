#include "CSettings.h"
#include <EEPROM.h>
#include "Config.h"
#include "fixFont.h"

CSettings::CSettings(uint8_t *byteBuf)
{
  #ifdef FORCE_PAL
  m_videoMode = 1;
  ROWS = 15;
  #else
  m_videoMode = 2;
  ROWS = 13;
  #endif
  COLS = 28;
  s.m_maxWatts = 2500;
  LoadDefaults();
  m_byteBuf = byteBuf;
  m_lastMAH = ReadInt16_t(251, 252);
}

bool CSettings::cleanEEPROM()
{
  int i;
  bool cleaned = true;
  byte check = 0xDD;
  for(i=(EEPROM.length()-1); i>(EEPROM.length()-5); i--)
  {
    byte readB = EEPROM.read(i);    
    if(readB != check) cleaned = false;
  }
  #ifdef FORCE_CLEAN_EEPROM
  cleaned = false;
  #endif
  if(!cleaned)
  {
    for(i=0; i<EEPROM.length(); i++) EEPROM.update(i,0);
    for(i=(EEPROM.length()-1); i>(EEPROM.length()-5); i--)
    {
      EEPROM.update(i, check);
    }
  }
  return cleaned;
}

void CSettings::LoadDefaults()
{
  #if defined(STEELE_PDB) | defined(IMPULSERC_VTX)
  s.m_batWarning = 0;
  s.m_batMAH[0] = 1300; //1300 mAh by default
  s.m_batMAH[1] = 1500; //1500 mAh by default
  s.m_batMAH[2] = 1800; //1800 mAh by default
  s.m_batMAH[3] = 2250; //2250 mAh by default
  s.m_activeBattery = 0;
  s.m_batWarningPercent = 25; //25% by default
  FixBatWarning();
  s.m_DVchannel = 4; //fixed positions
  #ifdef IMPULSERC_VTX
  s.m_tempUnit = 0; //°C default
  #else
  s.m_tempUnit = 1; //°F default
  #endif
  m_lastMAH = 0;
  s.m_fontSize = 1;
  s.m_displaySymbols = 1;
  s.m_vTxChannel = 4; // Raceband 5 @ 25mW default
  s.m_vTxBand = 4;
  s.m_vTxPower = 0;
  s.m_xOffset = 0; // Center OSD offsets
  s.m_yOffset = 3;
  s.m_stats = 2;
  m_DISPLAY_DV[DISPLAY_NICKNAME] = 0;
  m_DISPLAY_DV[DISPLAY_TIMER] = 0;
  m_DISPLAY_DV[DISPLAY_RC_THROTTLE] = 0;
  m_DISPLAY_DV[DISPLAY_COMB_CURRENT] = 1;
  m_DISPLAY_DV[DISPLAY_LIPO_VOLTAGE] = 1;
  m_DISPLAY_DV[DISPLAY_MA_CONSUMPTION] = 1;
  m_DISPLAY_DV[DISPLAY_ESC_KRPM] = 0;
  m_DISPLAY_DV[DISPLAY_ESC_CURRENT] = 0;
  m_DISPLAY_DV[DISPLAY_ESC_TEMPERATURE] = 0;
  m_DISPLAY_DV[DISPLAY_RSSI] = 0;
  uint8_t i;
  for(i=0; i < NICKNAME_STR_SIZE; i++)
  {
    s.m_nickname[i] = 0x00;
  }
  m_OSDItems[ESC1kr][0] = 0;
  m_OSDItems[ESC1kr][1] = 2;
  m_OSDItems[ESC1voltage][0] = 0;
  m_OSDItems[ESC1voltage][1] = 2;
  m_OSDItems[ESC1temp][0] = 0;
  m_OSDItems[ESC1temp][1] = 2;
  m_OSDItems[ESC2kr][0] = COLS-1;
  m_OSDItems[ESC2kr][1] = 2;
  m_OSDItems[ESC2voltage][0] = COLS;
  m_OSDItems[ESC2voltage][1] = 2;
  m_OSDItems[ESC2temp][0] = COLS;
  m_OSDItems[ESC2temp][1] = 2;
  m_OSDItems[ESC3kr][0] = COLS-1;
  m_OSDItems[ESC3kr][1] = ROWS-2;
  m_OSDItems[ESC3voltage][0] = COLS;
  m_OSDItems[ESC3voltage][1] = ROWS-2;
  m_OSDItems[ESC3temp][0] = COLS;
  m_OSDItems[ESC3temp][1] = ROWS-2;
  m_OSDItems[ESC4kr][0] = 0;
  m_OSDItems[ESC4kr][1] = ROWS-2;
  m_OSDItems[ESC4voltage][0] = 0;
  m_OSDItems[ESC4voltage][1] = ROWS-2;
  m_OSDItems[ESC4temp][0] = 0;
  m_OSDItems[ESC4temp][1] = ROWS-2;
  m_OSDItems[VOLTAGE][0] = 0;
  m_OSDItems[VOLTAGE][1] = 0;
  m_OSDItems[AMPS][0] = COLS/2-2;
  m_OSDItems[AMPS][1] = 0;
  m_OSDItems[THROTTLE][0] = 0;
  m_OSDItems[THROTTLE][1] = ROWS-4;
  m_OSDItems[STOPW][0] = COLS/2 - 3;
  m_OSDItems[STOPW][1] = ROWS - 2;
  m_OSDItems[NICKNAME][0] = COLS/2 - 4;
  m_OSDItems[NICKNAME][1] = ROWS - 1;
  m_OSDItems[MAH][0] = COLS - 1;
  m_OSDItems[MAH][1] = 0;
  m_OSDItems[RSSIp][0] = COLS/4;
  m_OSDItems[RSSIp][1] = ROWS - 1;
  s.m_goggle = 0; //0 = fatshark, 1 = headplay
  s.m_wattMeter = 0;
  s.m_Moustache = 1;
  s.m_voltWarning = 0;
  s.m_minVolts = 148;
  s.m_timerMode = 1;
  s.m_voltCorrect = 0;
  s.m_crossHair = 0;
  #ifdef BF32_MODE
  s.m_RSSIchannel = 4;
  #else
  s.m_RSSIchannel = -1;
  #endif
  for(i=0; i<ICON_SETTINGS_SIZE; i++)
  {
    m_IconSettings[i] = 1;  
  }
  m_IconSettings[MAH_ICON] = 0;
  s.m_vTxMaxPower = 0;
  s.m_RSSImax = 2000;
  s.m_RSSImin = 2000;
  for(i=0; i<4; i++)
  {
    s.m_ESCCorrection[i] = 100;
  }
  s.m_angleOffset = -20;
  s.m_RCSplitControl = 0;
  s.m_vTxMinPower = 0;
  #ifdef IMPULSERC_VTX
  s.m_AussieChannels = 0;
  #else
  s.m_AussieChannels = 1;
  #endif

  #else
  
  s.m_batWarning = 1; //on by default
  s.m_batMAH[0] = 1300; //1300 mAh by default
  s.m_batMAH[1] = 1500; //1500 mAh by default
  s.m_batMAH[2] = 1800; //1800 mAh by default
  s.m_batMAH[3] = 2250; //2250 mAh by default
  s.m_activeBattery = 0;
  s.m_batWarningPercent = 25; //25% by default
  FixBatWarning();
  s.m_DVchannel = 0; //AUX1 default
  s.m_tempUnit = 0; //°C default
  m_lastMAH = 0;
  s.m_fontSize = 1;
  s.m_displaySymbols = 1;
  s.m_vTxChannel = 4; // Raceband 5 @ 25mW default
  s.m_vTxBand = 4;
  s.m_vTxPower = 0;
  s.m_xOffset = 0; // Center OSD offsets
  s.m_yOffset = 0;
  s.m_stats = 1;
  m_DISPLAY_DV[DISPLAY_NICKNAME] = 9;
  m_DISPLAY_DV[DISPLAY_TIMER] = 2;
  m_DISPLAY_DV[DISPLAY_RC_THROTTLE] = 8;
  m_DISPLAY_DV[DISPLAY_COMB_CURRENT] = 5;
  m_DISPLAY_DV[DISPLAY_LIPO_VOLTAGE] = 0;
  m_DISPLAY_DV[DISPLAY_MA_CONSUMPTION] = 1;
  m_DISPLAY_DV[DISPLAY_ESC_KRPM] = 4;
  m_DISPLAY_DV[DISPLAY_ESC_CURRENT] = 6;
  m_DISPLAY_DV[DISPLAY_ESC_TEMPERATURE] = 7;
  m_DISPLAY_DV[DISPLAY_RSSI] = 3;
  uint8_t i;
  for(i=0; i < NICKNAME_STR_SIZE; i++)
  {
    s.m_nickname[i] = 0x00;
  }
  m_OSDItems[ESC1kr][0] = 0;
  m_OSDItems[ESC1kr][1] = 1;
  m_OSDItems[ESC1voltage][0] = 0;
  m_OSDItems[ESC1voltage][1] = 1;
  m_OSDItems[ESC1temp][0] = 0;
  m_OSDItems[ESC1temp][1] = 1;
  m_OSDItems[ESC2kr][0] = COLS-1;
  m_OSDItems[ESC2kr][1] = 1;
  m_OSDItems[ESC2voltage][0] = COLS;
  m_OSDItems[ESC2voltage][1] = 1;
  m_OSDItems[ESC2temp][0] = COLS;
  m_OSDItems[ESC2temp][1] = 1;
  m_OSDItems[ESC3kr][0] = COLS-1;
  m_OSDItems[ESC3kr][1] = ROWS-2;
  m_OSDItems[ESC3voltage][0] = COLS;
  m_OSDItems[ESC3voltage][1] = ROWS-2;
  m_OSDItems[ESC3temp][0] = COLS;
  m_OSDItems[ESC3temp][1] = ROWS-2;
  m_OSDItems[ESC4kr][0] = 0;
  m_OSDItems[ESC4kr][1] = ROWS-2;
  m_OSDItems[ESC4voltage][0] = 0;
  m_OSDItems[ESC4voltage][1] = ROWS-2;
  m_OSDItems[ESC4temp][0] = 0;
  m_OSDItems[ESC4temp][1] = ROWS-2;
  m_OSDItems[VOLTAGE][0] = 0;
  m_OSDItems[VOLTAGE][1] = ROWS-1;
  m_OSDItems[AMPS][0] = COLS;
  m_OSDItems[AMPS][1] = 0;
  m_OSDItems[THROTTLE][0] = 0;
  m_OSDItems[THROTTLE][1] = 0;
  m_OSDItems[STOPW][0] = COLS/2 - 3;
  m_OSDItems[STOPW][1] = ROWS - 2;
  m_OSDItems[NICKNAME][0] = COLS/2 - 4;
  m_OSDItems[NICKNAME][1] = ROWS - 1;
  m_OSDItems[MAH][0] = COLS - 1;
  m_OSDItems[MAH][1] = ROWS - 1;
  m_OSDItems[RSSIp][0] = COLS/4;
  m_OSDItems[RSSIp][1] = ROWS - 1;
  s.m_goggle = 0; //0 = fatshark, 1 = headplay
  s.m_wattMeter = 1;
  s.m_Moustache = 1;
  s.m_voltWarning = 0;
  s.m_minVolts = 148;
  s.m_timerMode = 1;
  s.m_voltCorrect = 0;
  s.m_crossHair = 0;
  #ifdef BF32_MODE
  s.m_RSSIchannel = 4; //telemetry
  #else
  s.m_RSSIchannel = 3; //AUX4
  #endif
  for(i=0; i<ICON_SETTINGS_SIZE; i++)
  {
    m_IconSettings[i] = 1;  
  }
  s.m_vTxMaxPower = 0;
  s.m_RSSImax = 2000;
  s.m_RSSImin = 2000;
  for(i=0; i<4; i++)
  {
    s.m_ESCCorrection[i] = 100;
  }
  s.m_angleOffset = -20;
  s.m_RCSplitControl = 0;
  s.m_vTxMinPower = 0;
  s.m_AussieChannels = 1;
  
  #endif
}

void CSettings::fixColBorders()
{
  if(m_oldDisplaySymbols != s.m_displaySymbols)
  {
    m_oldDisplaySymbols = s.m_displaySymbols;
    int8_t moveDir = 1;
    if(s.m_displaySymbols == 1) moveDir = -1;
    uint8_t i;
    for(i=0; i < OSD_ITEMS_POS_SIZE; i++)
    {
      if(m_colBorder[i])
      {
        if((i== ESC1kr || i== ESC2kr || i== ESC3kr || i== ESC4kr) && m_IconSettings[PROPS_ICON] == 1)
        {
          m_OSDItems[i][0] += (int8_t)((int8_t)5 * moveDir);
        }
        else
        {
          if(i >= STOPWp && i < RSSIp && m_IconSettings[ESC_ICON] == 1)
          {
             m_OSDItems[i][0] += (int8_t)((int8_t)1 * moveDir);
          }
        }      
      }
    }
  }
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

void CSettings::ReadSettingsInternal(bool readFromBuf, uint8_t sizeOverride)
{
  uint16_t i;
  if(!readFromBuf) for(i=0; i<sizeof(s); i++) m_byteBuf[i] = EEPROM.read(i+3);
  uint8_t sizeRead = sizeof(s);
  uint8_t bufStartIndex = 0;
  if(sizeOverride > 0) 
  {
    sizeRead = sizeOverride;
    bufStartIndex = 1;
  }
  memcpy(&s, &m_byteBuf[bufStartIndex], sizeRead);
  m_DISPLAY_DV[DISPLAY_RSSI] = s.m_DISPLAY_DV_RSSI;
  memcpy(m_DISPLAY_DV, &s.m_DISPLAY_DV_[0], sizeof(s.m_DISPLAY_DV_));
  memcpy(m_OSDItems, &s.m_OSDItems_[0], sizeof(s.m_OSDItems_));
  memcpy(&m_OSDItems[18], s.m_OSDItemsRSSIp, sizeof(s.m_OSDItemsRSSIp));
  m_IconSettings[0] = s.m_IconSettingsPROPS_ICON;
  memcpy(&m_IconSettings[1], s.m_IconSettings_, sizeof(s.m_IconSettings_));

  if(!readFromBuf)
  {
    s.m_maxWatts = ReadInt16_t(253, 254);
  }
}

void CSettings::UpgradeFromPreviousVersion(uint8_t ver)
{
  if(ver >= 0x09)
  {
    ReadSettingsInternal();
    if(ver < 0x0D)
    {
      s.m_wattMeter = 1;
      s.m_Moustache = 1;
      s.m_maxWatts = 2500;    
      s.m_voltWarning = 0;
      s.m_minVolts = 148;
      m_IconSettings[PROPS_ICON] = 1;
      s.m_timerMode = 1;
      s.m_voltCorrect = 0;
    }
    if(ver < 0x0E)
    {
      s.m_crossHair = 0;
    }
    if(ver < 0x0F)
    {
      s.m_RSSIchannel = 3;
      m_DISPLAY_DV[DISPLAY_RSSI] = 9;
      m_OSDItems[RSSIp][0] = COLS/4;
      m_OSDItems[RSSIp][1] = ROWS - 1;
    }
    if(ver < 0x10)
    {
      uint8_t i;
      for(i=1; i<ICON_SETTINGS_SIZE; i++)
      {
        m_IconSettings[i] = 1;
      }
      m_OSDItems[ESC2voltage][0]++;
      m_OSDItems[ESC3voltage][0]++;
    }
    if(ver < 0x13)
    {
      s.m_vTxMaxPower = 0;
      #ifdef STEELE_PDB
      s.m_stats = 2;
      #else
      s.m_stats = 1;
      #endif
    }
    if(ver < 0x14)
    {
      s.m_RSSImax = -1001;
      s.m_RSSImin = -1001;
    }
    if(ver < 0x15)
    {
      for(uint8_t i=0; i<4; i++)
      {
        s.m_ESCCorrection[i] = 100;
      }      
    }
    if(ver < 0x16)
    {
      s.m_angleOffset = -20;
    }
    if(ver < 0x17)
    {
      s.m_RCSplitControl = 0;
    }
    if(ver < 0x18)
    {
      s.m_vTxMinPower = 0;
    }
    if(ver < 0x19)
    {
      #ifdef IMPULSERC_VTX
      s.m_AussieChannels = 1;
      #else
      s.m_AussieChannels = 0;
      #endif
    }
    if(ver < 0x1A)
    {
      s.m_RSSImin = 2000;
      s.m_RSSImax = 2000;
    }
  }
}

void CSettings::ReadSettings(bool readFromBuf, uint8_t sizeOverride)
{
  m_settingVersion = 0x1A;
  uint8_t settingsVer = EEPROM.read(0x01);
  if(settingsVer < m_settingVersion && !readFromBuf) //first start of OSD - or older version
  {
    UpgradeFromPreviousVersion(settingsVer);
    m_lastMAH = 0;
    WriteLastMAH();
    WriteSettings(); //write defaults
  }
  else
  {
    ReadSettingsInternal(readFromBuf, sizeOverride);
  }
  FixBatWarning();
}

void CSettings::WriteInt16_t(byte lsbPos, byte msbPos, int16_t value)
{
  byte msb, lsb;
  lsb = (byte)(value & 0x00FF);
  msb = (byte)((value & 0xFF00) >> 8);
  
  EEPROM.update(lsbPos, lsb); // LSB
  EEPROM.update(msbPos, msb); // MSB
}

void CSettings::WriteSettings(bool bufWriteOnly)
{
  s.m_DISPLAY_DV_RSSI = m_DISPLAY_DV[DISPLAY_RSSI];
  memcpy(&s.m_DISPLAY_DV_[0], m_DISPLAY_DV, sizeof(s.m_DISPLAY_DV_));
  memcpy(&s.m_OSDItems_[0], m_OSDItems, sizeof(s.m_OSDItems_));
  memcpy(s.m_OSDItemsRSSIp, &m_OSDItems[18], sizeof(s.m_OSDItemsRSSIp));
  s.m_IconSettingsPROPS_ICON = m_IconSettings[0];
  memcpy(s.m_IconSettings_, &m_IconSettings[1], sizeof(s.m_IconSettings_));
  uint8_t bufIndex = 0;
  if(bufWriteOnly) bufIndex = 2;
  memcpy(&m_byteBuf[bufIndex], &s, sizeof(s));
  uint16_t i;
  if(!bufWriteOnly) 
  {
    for(i=0; i<sizeof(s); i++) EEPROM.update(i+3, m_byteBuf[i]);
    WriteInt16_t(253, 254, s.m_maxWatts);
    EEPROM.update(0x01,m_settingVersion);
  }
}

void CSettings::FixBatWarning()
{
  uint32_t temp = ((uint32_t)s.m_batMAH[s.m_activeBattery] * (uint32_t)s.m_batWarningPercent) / (uint32_t)100;
  s.m_batWarningMAH = s.m_batMAH[s.m_activeBattery] - (int16_t)temp;
  m_batSlice = s.m_batMAH[s.m_activeBattery] / 8;
}

void CSettings::WriteLastMAH()
{
  WriteInt16_t(251, 252, m_lastMAH);
}

void CSettings::SetupPPMs(int16_t *dv_ppms, bool all)
{
  uint8_t i;
  if(all && s.m_DVchannel < 4)
  {
    for(i=0; i<DISPLAY_DV_SIZE; i++)
    {
      dv_ppms[i] = -1001;      
    }
  }
  else
  {
    for(i=0; i<DISPLAY_DV_SIZE; i++)
    {
      if(s.m_DVchannel < 4) dv_ppms[i] = -1000 + (m_DISPLAY_DV[i] * DV_PPM_INCREMENT);      
      else dv_ppms[i] = m_DISPLAY_DV[i] * -1;
    }
  }
}

void CSettings::UpdateMaxWatt(int16_t maxWatt)
{
  if(maxWatt > s.m_maxWatts)
  {
    s.m_maxWatts = maxWatt;
    WriteInt16_t(253, 254, s.m_maxWatts);
  }
}

