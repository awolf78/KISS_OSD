#include <Arduino.h>
#include "Config.h"

#ifndef CSettingsh
#define CSettingsh

enum _DISPLAY 
{ 
  DISPLAY_NICKNAME, 
  DISPLAY_TIMER, 
  DISPLAY_RC_THROTTLE, 
  DISPLAY_COMB_CURRENT, 
  DISPLAY_LIPO_VOLTAGE, 
  DISPLAY_MA_CONSUMPTION, 
  DISPLAY_ESC_KRPM, 
  DISPLAY_ESC_CURRENT, 
  DISPLAY_ESC_TEMPERATURE,
  DISPLAY_RSSI 
};

enum _OSDItems
{
  THROTTLE,
  AMPS,
  MAH,
  NICKNAME,
  VOLTAGE,
  STOPW,
  ESC1,
  ESC2,
  ESC3,
  ESC4,
  RSSI_
};

enum _OSDItemPos
{
  THROTTLEp,
  AMPSp,
  MAHp,
  NICKNAMEp,
  VOLTAGEp,
  STOPWp,
  ESC1kr,
  ESC1voltage,
  ESC1temp,
  ESC2kr,
  ESC2voltage,
  ESC2temp,
  ESC3kr,
  ESC3voltage,
  ESC3temp,
  ESC4kr,
  ESC4voltage,
  ESC4temp,
  RSSIp
};

enum _IconSettings
{
  PROPS_ICON,
  ESC_ICON,
  WATT_ICON,
  MAH_ICON,
  RSSI_ICON,
  TIMER_ICON,
  KISS_ICON
};

class CSettings
{
  public:
  CSettings(uint8_t *byteBuf);
  void ReadSettings(bool readFromBuf = false, uint8_t sizeOverride = 0);
  void WriteSettings(bool bufWriteOnly = false);
  void FixBatWarning();
  void WriteLastMAH();
  void LoadDefaults();
  void SetupPPMs(int16_t *dv_ppms, bool all = false);
  void fixColBorders();
  bool cleanEEPROM();
  void UpdateMaxWatt(int16_t maxWatt);

  struct _EEPROM_SETTINGS
  {
    uint16_t m_batWarningMAH; // 300-32000 mAh
    uint8_t m_batWarning; // 0 = off, 1 = on
    uint8_t m_activeBattery; // 0-4
    uint8_t m_DVchannel; // AUX1-AUX4
    uint16_t m_batMAH[4]; // 300-32000 mAh
    uint8_t m_batWarningPercent; // battery capacity percentage -> if below, warning will get displayed
    uint8_t m_tempUnit; //°C or °F
    uint8_t m_fontSize; // 0 = normal font; 1 = large font
    uint8_t m_displaySymbols; // 0 = no; 1 = yes (symbols such as battery and timer)
    uint8_t m_vTxChannel; // vTx settings
    uint8_t m_vTxBand; // vTx settings
    uint8_t m_vTxPower; // vTx settings
    int8_t m_xOffset; // OSD screen offsets
    int8_t m_yOffset; // OSD screen offsets
    uint8_t m_DISPLAY_DV_[9]; //order of OSD items
    char m_nickname[9]; //Nickname to be displayed on OSD
    uint8_t m_OSDItems_[18][2]; // OSD item positions
    uint8_t m_goggle; //0 = fatshark, 1 = headplay
    uint8_t m_wattMeter; // 0 = off, 1 = on
    uint8_t m_Moustache; //unused
    uint16_t m_maxWatts; // max watts ever recorded
    uint8_t m_voltWarning; // 0 = off, 1 = on
    uint8_t m_minVolts; // min volts for voltage warning
    uint8_t m_IconSettingsPROPS_ICON; // 0 = off, 1 = on
    uint8_t m_timerMode; // 0 = reset, 1 = continuous, 2 = race timer
    int8_t m_voltCorrect;
    uint8_t m_crossHair;
    uint8_t m_RSSIchannel; // 0-3 for AUX1-AUX4
    uint8_t m_OSDItemsRSSIp[2]; // RSSI OSD item position
    uint8_t m_DISPLAY_DV_RSSI; //order of RSSI OSD item
    uint8_t m_IconSettings_[6]; // for each icon in m_IconSettings_: 0 = off, 1 = on
    uint16_t m_vTxMaxPower;
    uint8_t m_stats; // 0 = off, 1 = on (always shown when disarmed), 2 = hidden (roll left to show them when disarmed, roll right to hide)
    int16_t m_RSSImax;
    int16_t m_RSSImin;
    uint8_t m_ESCCorrection[4]; // for each ESC: 50-200. Corrects amps and mAh in % for each ESC.
    int8_t m_angleOffset;
    uint8_t m_RCSplitControl; // 0 = off, 1 = on
    uint8_t m_vTxMinPower; // 0-2 for Helix/Mercury power modes: 25mW, 200mW, 500mW
    uint8_t m_AussieChannels;
  } s;
  uint16_t m_batSlice; // calculated for battery symbol
  int16_t m_lastMAH; // mAh value from previous run
  static const uint8_t NICKNAME_STR_SIZE = 9;
  static const uint8_t OSD_ITEMS_SIZE = 11;
  static const uint8_t OSD_ITEMS_POS_SIZE = 19;
  uint8_t m_OSDItems[OSD_ITEMS_POS_SIZE][2]; // OSD item positions
  bool m_colBorder[OSD_ITEMS_POS_SIZE];
  static const uint8_t DISPLAY_DV_SIZE = 10;
  uint8_t m_DISPLAY_DV[DISPLAY_DV_SIZE];
  static const uint8_t DV_PPM_INCREMENT = 150;
  uint8_t m_videoMode; //2 = NTSC, 1 = PAL
  uint8_t ROWS, COLS;
  static const uint8_t ICON_SETTINGS_SIZE = 7;
  uint8_t m_IconSettings[ICON_SETTINGS_SIZE];
  uint8_t m_oldDisplaySymbols;
  uint8_t m_settingVersion;
  
  private:
  uint8_t *m_byteBuf;
  void UpgradeFromPreviousVersion(uint8_t ver);
  void ReadSettingsInternal(bool readFromBuf = false, uint8_t sizeOverride = 0);
  int16_t ReadInt16_t(byte lsbPos, byte msbPos);
  void WriteInt16_t(byte lsbPos, byte msbPos, int16_t value);
};

#endif
