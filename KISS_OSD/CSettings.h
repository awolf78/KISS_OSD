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
  CSettings();
  void ReadSettings();
  void WriteSettings();
  void FixBatWarning();
  void WriteLastMAH();
  void LoadDefaults();
  void SetupPPMs(int16_t *dv_ppms, bool all = false);
  void fixColBorders();
  bool cleanEEPROM();
  void UpdateMaxWatt(int16_t maxWatt);
  
  uint8_t m_batWarning; // 0 = off, 1 = on
  uint16_t m_batMAH[4]; // 300-32000 mAh
  uint16_t m_batWarningMAH; // 300-32000 mAh
  uint16_t m_batSlice; // calculated for battery symbol
  uint8_t m_activeBattery; // 0-4
  uint8_t m_batWarningPercent; // battery capacity percentage -> if below, warning will get displayed
  uint8_t m_DVchannel; // AUX1-AUX4
  uint8_t m_tempUnit; //°C or °F
  int16_t m_lastMAH; // mAh value from previous run
  uint8_t m_fontSize; // 0 = normal font; 1 = large font
  uint8_t m_displaySymbols; // 0 = no; 1 = yes (symbols such as battery and timer)
  uint8_t m_vTxChannel, m_vTxBand, m_vTxPower; // vTx settings
  int8_t m_xOffset, m_yOffset; // OSD screen offsets
  uint8_t m_stats;
  static const uint8_t NICKNAME_STR_SIZE = 9;
  char m_nickname[NICKNAME_STR_SIZE]; //Nickname to be displayed on OSD
  static const uint8_t OSD_ITEMS_SIZE = 11;
  static const uint8_t OSD_ITEMS_POS_SIZE = 19;
  uint8_t m_OSDItems[OSD_ITEMS_POS_SIZE][2]; // OSD item positions
  bool m_colBorder[OSD_ITEMS_POS_SIZE];
  static const uint8_t DISPLAY_DV_SIZE = 10;
  uint8_t m_DISPLAY_DV[DISPLAY_DV_SIZE];
  static const uint8_t DV_PPM_INCREMENT = 150;
  uint8_t m_goggle; //0 = fatshark, 1 = headplay
  uint8_t m_videoMode; //2 = NTSC, 1 = PAL
  uint8_t ROWS, COLS;
  uint8_t m_wattMeter, m_Moustache;
  uint16_t m_maxWatts;
  uint8_t m_voltWarning, m_minVolts;
  uint8_t m_airTimer;
  uint16_t m_voltCorrect;
  uint8_t m_crossHair;
  int8_t m_RSSIchannel;
  int16_t m_RSSImax, m_RSSImin;
  static const uint8_t ICON_SETTINGS_SIZE = 7;
  uint8_t m_IconSettings[ICON_SETTINGS_SIZE];
  uint8_t m_oldDisplaySymbols;
  uint16_t m_vTxMaxPower;
  #ifdef MAH_CORRECTION
  uint8_t m_ESCCorrection[4];
  #endif
  
  private:
  void UpgradeFromPreviousVersion(uint8_t ver);
  void ReadSettingsInternal();
  int16_t ReadInt16_t(byte lsbPos, byte msbPos);
  void WriteInt16_t(byte lsbPos, byte msbPos, int16_t value);
};

#endif
