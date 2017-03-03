static uint8_t active3MenuItem = 0;
static uint8_t activeTuneMenuItem = 0;
static uint8_t activePIDRollMenuItem = 0;
static uint8_t activePIDYawMenuItem = 0;
static uint8_t activePIDPitchMenuItem = 0;
static uint8_t activeTPAMenuItem = 0;
static uint8_t activeRatesMenuItem = 0;
static uint8_t activeRatesRollMenuItem = 0;
static uint8_t activeRatesPitchMenuItem = 0;
static uint8_t activeRatesYawMenuItem = 0;
static uint8_t activeBatteryMenuItem = 0;
static uint8_t activeDisplayMenuItem = 0;
static uint8_t activeNotchMenuItem = 0;
static uint8_t activeVTXMenuItem = 0;
static uint8_t activeFilterMenuItem = 0;
static const int16_t P_STEP = 100;
static const int16_t I_STEP = 1;
static const int16_t D_STEP = 1000;
static const int16_t TPA_STEP = 50;
static const int16_t RATE_STEP = 50;

static const char SAVE_EXIT_STR[] PROGMEM = "save+exit";
static const char BACK_STR[] PROGMEM =      "back";
static char ON_OFF_STR[][4] = { "off", "on " };
static const char ROLL_STR[] PROGMEM =  "roll  ";
static const char PITCH_STR[] PROGMEM = "pitch ";
static const char YAW_STR[] PROGMEM =   "yaw   ";

extern void* MainMenu();
extern void* RatesMenu();

boolean checkCode(int16_t &value, int16_t STEP, int16_t minVal = 0, int16_t maxVal = 32000)
{
  boolean changed = false;
  if(code &  inputChecker.ROLL_LEFT)
  {
    if((value-STEP) >= minVal)
    {
      value -= STEP;
      changed = true;
    }
    else
    {
      if(value > minVal)
      {
        changed = true;
      }
      value = minVal;        
    }
  }
  if(code &  inputChecker.YAW_LEFT)
  {
    if((value-(STEP/10)) >= minVal)
    {
      changed = true;
      value -= STEP/10;
    }
    else
    {
      if(value > minVal)
      {
        changed = true;
      }
      value = minVal;
    }
  }
  if(code &  inputChecker.ROLL_RIGHT)
  {
    if((value+STEP) <= maxVal)
    {
      changed = true;
      value += STEP;
    }
    else
    {
      if(value < maxVal)
      {
        changed = true;
      }
      value = maxVal;
    }
  }
  if(code &  inputChecker.YAW_RIGHT)
  {
    if((value+(STEP/10)) <= maxVal)
    {
      changed = true;
      value += STEP/10;
    }
    else
    {
      if(value < maxVal)
      {
        changed = true;
      }
      value = maxVal;
    }
  }
  return changed;
}

boolean checkCode(volatile int16_t &value, int16_t STEP, int16_t minVal = 0, int16_t maxVal = 32000)
{
  int16_t value2 = value;
  boolean changed = checkCode(value2, STEP, minVal, maxVal);
  value = value2;
  return changed;
}

boolean checkCode(volatile uint8_t &value, int16_t STEP, int16_t minVal = 0, int16_t maxVal = 32000)
{
  int16_t tempValue = value;
  boolean changed = checkCode(tempValue, STEP, minVal, maxVal);
  value = (uint8_t) tempValue;
  return changed;
}

uint8_t checkMenuItem(uint8_t menuItem, uint8_t maxItems)
{
  if((code &  inputChecker.PITCH_UP) && menuItem > 0)
  {
    return menuItem - 1;
  }
  
  if((code &  inputChecker.PITCH_UP) && menuItem == 0)
  {
    return maxItems - 1;
  }
  
  if((code &  inputChecker.PITCH_DOWN) && menuItem < (maxItems-1))
  {
    return menuItem + 1;
  } 
  
  if((code &  inputChecker.PITCH_DOWN) && menuItem == (maxItems-1))
  {
    return 0;
  }
  
  return menuItem; 
}

static char emptySuffix[][3] = { "", "", "" };

void* ThreeItemPlusBackMenu(uint8_t &active3MenuItem, int16_t &item1, int16_t &item2, int16_t &item3, int16_t item1_step, int16_t item2_step, int16_t item3_step, char* title, void* prevPage, void* thisPage, const char *itemDescription1 = 0, const char *itemDescription2 = 0, const char *itemDescription3 = 0, char (*suffix)[3] = NULL, uint8_t dec = 3)
{
  switch(active3MenuItem)
  {
    case 0:
      fcSettingChanged |= checkCode(item1, item1_step);
    break;
    case 1:
      fcSettingChanged |= checkCode(item2, item2_step);        
    break;
    case 2:
      fcSettingChanged |= checkCode(item3, item3_step);        
    break;
    case 3:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        active3MenuItem = 0;
        cleanScreen();
        return prevPage;
      }
    case 4:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        active3MenuItem = 0;
        menuActive = false;
        menuWasActive = true;
      }
  }
  
  if(suffix == NULL)
  {
    suffix = emptySuffix;
  }
  
  static const uint8_t MENU_ITEMS = 5;
  
  active3MenuItem = checkMenuItem(active3MenuItem, MENU_ITEMS);
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(itemDescription1)+6)/2;
  OSD.setCursor( settings.COLS/2 - strlen(title)/2, ++startRow );
  OSD.print( fixStr(title) );
  
  OSD.printIntArrow( startCol, ++startRow, itemDescription1, item1, dec, 1, active3MenuItem, suffix[0], true);
  OSD.printIntArrow( startCol, ++startRow, itemDescription2, item2, dec, 1, active3MenuItem, suffix[1], true);
  OSD.printIntArrow( startCol, ++startRow, itemDescription3, item3, dec, 1, active3MenuItem, suffix[2], true);
  OSD.printP( startCol, ++startRow, BACK_STR, active3MenuItem);
  OSD.printP( startCol, ++startRow, SAVE_EXIT_STR, active3MenuItem);
  
  return thisPage;
}

static const char RATES_DESC_STR1[] PROGMEM = "rc rate  : "; 
static const char RATES_DESC_STR2[] PROGMEM = "rate     : "; 
static const char RATES_DESC_STR3[] PROGMEM = "rc curve : ";

void* RatesRollMenu()
{
  return ThreeItemPlusBackMenu(activeRatesRollMenuItem, rcrate_roll, rate_roll, rccurve_roll, RATE_STEP, RATE_STEP, RATE_STEP, "roll", (void*) RatesMenu, (void*) RatesRollMenu, RATES_DESC_STR1, RATES_DESC_STR2, RATES_DESC_STR3);
}

void* RatesPitchMenu()
{
  return ThreeItemPlusBackMenu(activeRatesPitchMenuItem, rcrate_pitch, rate_pitch, rccurve_pitch, RATE_STEP, RATE_STEP, RATE_STEP, "pitch", (void*)RatesMenu, (void*) RatesPitchMenu, RATES_DESC_STR1, RATES_DESC_STR2, RATES_DESC_STR3);
}

void* RatesYawMenu()
{
  return ThreeItemPlusBackMenu(activeRatesPitchMenuItem, rcrate_yaw, rate_yaw, rccurve_yaw, RATE_STEP, RATE_STEP, RATE_STEP, "yaw", (void*)RatesMenu, (void*) RatesYawMenu, RATES_DESC_STR1, RATES_DESC_STR2, RATES_DESC_STR3);
}

void* RatesMenu()
{
  if(code &  inputChecker.ROLL_RIGHT)
  {
    switch(activeRatesMenuItem)
    {
      case 0:
        cleanScreen();
        return (void*)RatesRollMenu;
      break;
      case 1:
        cleanScreen();
        return (void*)RatesPitchMenu;
      break;
      case 2:
        cleanScreen();
        return (void*)RatesYawMenu;
      break;
      case 3:
        cleanScreen();
        activeRatesMenuItem = 0;
        return (void*)MainMenu;
      break;
    }
  }
  
  static const uint8_t RATES_MENU_ITEMS = 4;
  activeRatesMenuItem = checkMenuItem(activeRatesMenuItem, RATES_MENU_ITEMS);
  
//static const char ROLL_STR[] PROGMEM =  "roll  ";
//static const char PITCH_STR[] PROGMEM = "pitch ";
//static const char YAW_STR[] PROGMEM =   "yaw   ";
//static const char BACK_STR[] PROGMEM =  "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - strlen_P(ROLL_STR)/2;
  static const char RATES_TITLE_STR[] PROGMEM = "rates menu";
  OSD.printP(settings.COLS/2 - strlen_P(RATES_TITLE_STR)/2, ++startRow, RATES_TITLE_STR);
  
  OSD.printP( startCol, ++startRow, ROLL_STR, activeRatesMenuItem );
  OSD.printP( startCol, ++startRow, PITCH_STR, activeRatesMenuItem );
  OSD.printP( startCol, ++startRow, YAW_STR, activeRatesMenuItem );
  OSD.printP( startCol, ++startRow, BACK_STR, activeRatesMenuItem );

  return (void*)RatesMenu;
}

extern void* TuneMenu();

static const char PID_DESC_STR1[] PROGMEM = "p : "; 
static const char PID_DESC_STR2[] PROGMEM = "i : "; 
static const char PID_DESC_STR3[] PROGMEM = "d : ";

void* PIDRollMenu()
{
  return ThreeItemPlusBackMenu(activePIDRollMenuItem,  p_roll, i_roll, d_roll, P_STEP, I_STEP, D_STEP, "roll", (void*)TuneMenu, (void*) PIDRollMenu, PID_DESC_STR1, PID_DESC_STR2, PID_DESC_STR3);
}

void* PIDPitchMenu()
{
  return ThreeItemPlusBackMenu(activePIDPitchMenuItem,  p_pitch, i_pitch, d_pitch, P_STEP, I_STEP, D_STEP, "pitch", (void*)TuneMenu, (void*) PIDPitchMenu, PID_DESC_STR1, PID_DESC_STR2, PID_DESC_STR3);
}

void* PIDYawMenu()
{
  return ThreeItemPlusBackMenu(activePIDYawMenuItem,  p_yaw, i_yaw, d_yaw, P_STEP, I_STEP, D_STEP, "yaw", (void*)TuneMenu, (void*) PIDYawMenu, PID_DESC_STR1, PID_DESC_STR2, PID_DESC_STR3);
}

void* TPAMenu()
{
  static const char TPA_DESC_STR1[] PROGMEM = "tpa p : "; 
  static const char TPA_DESC_STR2[] PROGMEM = "tpa i : "; 
  static const char TPA_DESC_STR3[] PROGMEM = "tpa d : ";
  return ThreeItemPlusBackMenu(activeTPAMenuItem,  p_tpa, i_tpa, d_tpa, TPA_STEP, TPA_STEP, TPA_STEP, "tpa menu", (void*)TuneMenu, (void*) TPAMenu, TPA_DESC_STR1, TPA_DESC_STR2, TPA_DESC_STR3);
}

void* FilterMenu()
{
  switch(activeFilterMenuItem)
  {
    case 0:
      fcSettingChanged |= checkCode(lpf_frq, 1, 0, 6);
    break;
    case 1:
      fcSettingChanged |= checkCode(yawFilterCut, 1, 0, 97);
    break;
    case 2:
      fcSettingChanged |= checkCode(notchFilterEnabledR, 1, 0, 1);
    break;
    case 3:
      fcSettingChanged |= checkCode(notchFilterCenterR, 1, 0, 490);
    break;
    case 4:
      fcSettingChanged |= checkCode(notchFilterCutR, 1, 0, 490);
    break;
    case 5:
      fcSettingChanged |= checkCode(notchFilterEnabledP, 1, 0, 1);
    break;
    case 6:
      fcSettingChanged |= checkCode(notchFilterCenterP, 1, 0, 490);
    break;
    case 7:
      fcSettingChanged |= checkCode(notchFilterCutP, 1, 0, 490);
    break;        
    case 8:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        activeFilterMenuItem = 0;
        return (void*)MainMenu;
      }
    break;
  }

  static const uint8_t FILTER_MENU_ITEMS = 9;
  activeFilterMenuItem = checkMenuItem(activeFilterMenuItem, FILTER_MENU_ITEMS);
  
  static const char LPF_STR[] PROGMEM =                 "lpf      : ";
  static const char YAW_FLTR_STR[] PROGMEM =            "yaw fltr strength:";
  static const char NOTCH_ROLL_STR[] PROGMEM =          "notch roll fltr  :";
  static const char NOTCH_ROLL_CENTER_STR[] PROGMEM =   "roll center freq :";
  static const char NOTCH_ROLL_CUTOFF_STR[] PROGMEM =   "roll cutoff freq :";
  static const char NOTCH_PITCH_STR[] PROGMEM =         "notch pitch fltr :";
  static const char NOTCH_PITCH_CENTER_STR[] PROGMEM =  "pitch center freq:";
  static const char NOTCH_PITCH_CUTOFF_STR[] PROGMEM =  "pitch cutoff freq:";
//static const char BACK_STR[] PROGMEM =     "back";
  
  static const char LPF1_STR[] PROGMEM = "off ";
  static const char LPF2_STR[] PROGMEM = "high    ";
  static const char LPF3_STR[] PROGMEM = "med high";
  static const char LPF4_STR[] PROGMEM = "medium  ";
  static const char LPF5_STR[] PROGMEM = "med low ";
  static const char LPF6_STR[] PROGMEM = "low     ";
  static const char LPF7_STR[] PROGMEM = "very low";
  static const char* LPF_FRQ_STR[] = { LPF1_STR, LPF2_STR, LPF3_STR, LPF4_STR, LPF5_STR, LPF6_STR, LPF7_STR };
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(YAW_FLTR_STR)+8)/2;
  static const char FILTER_MENU_TITLE_STR[] PROGMEM = "filter menu";
  OSD.printP(settings.COLS/2 - strlen_P(FILTER_MENU_TITLE_STR)/2, ++startRow, FILTER_MENU_TITLE_STR);
  
  OSD.printP( startCol, ++startRow, LPF_STR, activeFilterMenuItem);
  OSD.print( fixPStr(LPF_FRQ_STR[lpf_frq]) );
  OSD.printIntArrow( startCol, ++startRow, YAW_FLTR_STR, yawFilterCut, 0, 1, activeFilterMenuItem, "", 1);
  OSD.printP( startCol, ++startRow, NOTCH_ROLL_STR, activeFilterMenuItem);
  OSD.print( fixStr(ON_OFF_STR[notchFilterEnabledR]) );
  OSD.printIntArrow( startCol, ++startRow, NOTCH_ROLL_CENTER_STR, notchFilterCenterR, 0, 1, activeFilterMenuItem, "hz", 1);
  OSD.printIntArrow( startCol, ++startRow, NOTCH_ROLL_CUTOFF_STR, notchFilterCutR, 0, 1, activeFilterMenuItem, "hz", 1);
  OSD.printP( startCol, ++startRow, NOTCH_PITCH_STR, activeFilterMenuItem);
  OSD.print( fixStr(ON_OFF_STR[notchFilterEnabledP]) );
  OSD.printIntArrow( startCol, ++startRow, NOTCH_PITCH_CENTER_STR, notchFilterCenterP, 0, 1, activeFilterMenuItem, "hz", 1); 
  OSD.printIntArrow( startCol, ++startRow, NOTCH_PITCH_CUTOFF_STR, notchFilterCutP, 0, 1, activeFilterMenuItem, "hz", 1);
  OSD.printP( startCol, ++startRow, BACK_STR, activeFilterMenuItem);
  
  return (void*)FilterMenu;  
}

void* TuneMenu()
{
  switch(activeTuneMenuItem)
  {
    case 0:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        return (void*)PIDRollMenu;
      }
    break;
    case 1:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        return (void*)PIDPitchMenu;
      }
    break;
    case 2:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        return (void*)PIDYawMenu;
      }
    break;
    case 3:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        return (void*)TPAMenu;
      }
    break;         
    case 4:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        activeTuneMenuItem = 0;
        return (void*)MainMenu;
      }
    break;
  }

  static const uint8_t TUNE_MENU_ITEMS = 5;
  activeTuneMenuItem = checkMenuItem(activeTuneMenuItem, TUNE_MENU_ITEMS);
  
//static const char ROLL_STR[] PROGMEM =     "roll";
//static const char PITCH_STR[] PROGMEM =    "pitch";
//static const char YAW_STR[] PROGMEM =      "yaw";
  static const char TPA_STR[] PROGMEM =      "tpa";  
//static const char BACK_STR[] PROGMEM =     "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - strlen_P(PITCH_STR)/2;
  static const char TUNE_MENU_TITLE_STR[] PROGMEM = "tune menu";
  OSD.printP(settings.COLS/2 - strlen_P(TUNE_MENU_TITLE_STR)/2, ++startRow, TUNE_MENU_TITLE_STR);
  
  OSD.printP( startCol, ++startRow, ROLL_STR, activeTuneMenuItem);
  OSD.printP( startCol, ++startRow, PITCH_STR, activeTuneMenuItem);
  OSD.printP( startCol, ++startRow, YAW_STR, activeTuneMenuItem);
  OSD.printP( startCol, ++startRow, TPA_STR, activeTuneMenuItem);  
  OSD.printP( startCol, ++startRow, BACK_STR, activeTuneMenuItem);
  
  return (void*)TuneMenu;
}



void* BatteryMenu()
{
  boolean changed = false;
  switch(activeBatteryMenuItem)
  {
    case 0:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        batterySelect = true;
      }
    break;
    case 1:
      settingChanged |= checkCode(settings.m_batWarning, 1, 0, 1);
    break;
    case 2:
      changed = checkCode(settings.m_batWarningPercent, 1, 0, 100);
      settingChanged |= changed;
      if(changed)
      {
        settings.FixBatWarning();
      }
    break;
    case 3:
      settingChanged |= checkCode(settings.m_voltWarning, 1, 0, 1);
    break;
    case 4:
      settingChanged |= checkCode(settings.m_minVolts, 1, 90, 250);
    break;
    case 5:
      settingChanged |= checkCode(settings.m_voltCorrect, 1, -10, 10);
    break;
    case 6:
      checkCode(settings.m_maxWatts, (int16_t)1000, (int16_t)1000, (int16_t)30000);
    break;
    case 7:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        menuActive = false;
        menuWasActive = true;
        settings.UpdateMaxWatt(settings.m_maxWatts);
      }
    break;
    case 8:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        activeBatteryMenuItem = 0;
        settings.UpdateMaxWatt(settings.m_maxWatts);
        return (void*)MainMenu;
      }
    break;
  }
  static const uint8_t BATTERY_MENU_ITEMS = 9;
  activeBatteryMenuItem = checkMenuItem(activeBatteryMenuItem, BATTERY_MENU_ITEMS);
  
  static const char SELECT_BATTERY_STR[] PROGMEM =  "select battery ";
  static const char BATTERY_WARNING_STR[] PROGMEM = "batt. warning: ";
  static const char BATTERY_PERCENT_STR[] PROGMEM = "batt. % alarm: ";
  static const char VOLTAGE_WARN_STR[] PROGMEM =    "volt. warning: ";
  static const char MIN_VOLT_STR[] PROGMEM =        "min voltage  : ";
  static const char VOLT_CORRECT_STR[] PROGMEM =    "voltage corr : ";
  static const char MAX_BEER_WATT_STR[] PROGMEM =   "wattmeter max: ";
//static const char SAVE_EXIT_STR[] PROGMEM =       "save+exit";
//static const char BACK_STR[] PROGMEM =            "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(BATTERY_WARNING_STR)+6)/2;
  static const char BATTERY_TITLE_STR[] PROGMEM = "battery menu";
  OSD.printP(settings.COLS/2 - strlen_P(BATTERY_TITLE_STR)/2, ++startRow, BATTERY_TITLE_STR);
  
  OSD.printP( startCol, ++startRow, SELECT_BATTERY_STR, activeBatteryMenuItem );
  
  OSD.printP( startCol, ++startRow, BATTERY_WARNING_STR, activeBatteryMenuItem );
  OSD.print( fixStr(ON_OFF_STR[settings.m_batWarning]) );
  
  OSD.printIntArrow( startCol, ++startRow, BATTERY_PERCENT_STR, settings.m_batWarningPercent, 0, 1, activeBatteryMenuItem, "%", true );

  OSD.printP( startCol, ++startRow, VOLTAGE_WARN_STR, activeBatteryMenuItem );
  OSD.print( fixStr(ON_OFF_STR[settings.m_voltWarning]) );

  OSD.printIntArrow( startCol, ++startRow, MIN_VOLT_STR, settings.m_minVolts, 1, 1, activeBatteryMenuItem, "v", 1 );

  OSD.printIntArrow( startCol, ++startRow, VOLT_CORRECT_STR, settings.m_voltCorrect, 1, 1, activeBatteryMenuItem, "v", 1 );

  OSD.printIntArrow( startCol, ++startRow, MAX_BEER_WATT_STR, settings.m_maxWatts/10, 0, 1, activeBatteryMenuItem, "w", 1 );
  
  OSD.printP( startCol, ++startRow, SAVE_EXIT_STR, activeBatteryMenuItem );
  OSD.printP( startCol, ++startRow, BACK_STR, activeBatteryMenuItem );
  
  return (void*)BatteryMenu;
}

static bool vTxSettingChanged = false;

#ifdef IMPULSERC_VTX
void* vTxMenu()
{
  if((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT))
  {
    switch(activeVTXMenuItem)
    {
      case 0:
        vTxSettingChanged |= checkCode(settings.m_vTxPower, 1, 0, 2);
      break;
      case 1:
        vTxSettingChanged |= checkCode(settings.m_vTxBand, 1, 0, 4);
      break;
      case 2:
        vTxSettingChanged |= checkCode(settings.m_vTxChannel, 1, 0, 7);
      break;
      case 3:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          vTxPower = settings.m_vTxPower;
          vTxBand = settings.m_vTxBand;
          vTxChannel = settings.m_vTxChannel;
          settingChanged |= vTxSettingChanged;
          menuActive = false;
          menuWasActive = true;
          vtx_set_frequency(vTxBand, vTxChannel);
          return (void*)MainMenu;
        }
      break;
      case 4:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          activeVTXMenuItem = 0;
          if(!vTxSettingChanged)
          {
            settings.m_vTxPower = vTxPower;
            settings.m_vTxBand = vTxBand;
            settings.m_vTxChannel = vTxChannel;
          }
          vTxSettingChanged = false;
          cleanScreen();
          return (void*)MainMenu;
        }
      break;
    }
  }
  static const uint8_t VTX_MENU_ITEMS = 5;
  activeVTXMenuItem = checkMenuItem(activeVTXMenuItem, VTX_MENU_ITEMS);
  
  static const char VTX_POWER_STR[] PROGMEM =      "power   : ";
  static const char VTX_BAND_STR[] PROGMEM =       "band    : ";
  static const char VTX_CHANNEL_STR[] PROGMEM =    "channel : ";
//static const char SAVE_EXIT_STR[] PROGMEM =      "save+exit";
//static const char BACK_STR[] PROGMEM =           "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(VTX_POWER_STR)+6)/2;
  static const char VTX_TITLE_STR[] PROGMEM = "vtx menu";
  OSD.printP( settings.COLS/2 - strlen_P(VTX_TITLE_STR)/2, ++startRow, VTX_TITLE_STR );

  static const char _25MW_STR[] PROGMEM =   "25mw ";
  static const char _200MW_STR[] PROGMEM =  "200mw";
  static const char _500MW_STR[] PROGMEM =  "500mw";
  static const char* VTX_POWERS_STR[] = { _25MW_STR, _200MW_STR, _500MW_STR };
  OSD.printP( startCol, ++startRow, VTX_POWER_STR, activeVTXMenuItem );
  OSD.print( fixPStr(VTX_POWERS_STR[settings.m_vTxPower]) );

  OSD.printP( startCol, ++startRow, VTX_BAND_STR, activeVTXMenuItem );
  OSD.print( fixStr(bandSymbols[settings.m_vTxBand]) );
  
  OSD.printIntArrow( startCol, ++startRow, VTX_CHANNEL_STR, settings.m_vTxChannel+1, 0, 1, activeVTXMenuItem, "=" );
  OSD.printInt16( startCol + strlen_P(VTX_CHANNEL_STR) + 3, startRow, (int16_t)pgm_read_word(&vtx_frequencies[settings.m_vTxBand][settings.m_vTxChannel]), 0, 1, "mhz" );
  
  OSD.printP( startCol, ++startRow, SAVE_EXIT_STR, activeVTXMenuItem );
  OSD.printP( startCol, ++startRow, BACK_STR, activeVTXMenuItem );
  
  return (void*)vTxMenu;
}
#else
void* vTxMenu()
{
  switch(activeVTXMenuItem)
  {
    case 0:
      vTxSettingChanged |= checkCode(vTxLowPower, 10, 5, 800);
    break;
    case 1:
      vTxSettingChanged |= checkCode(vTxHighPower, 10, 25, 800);
    break;
    case 2:
      vTxSettingChanged |= checkCode(vTxBand, 1, 0, 4);
    break;
    case 3:
      vTxSettingChanged |= checkCode(vTxChannel, 1, 0, 7);
    break;
    case 4:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        activeVTXMenuItem = 0;
        menuActive = false;
        menuWasActive = true;
        oldvTxBand = vTxBand;
        oldvTxChannel = vTxChannel;
        oldvTxLowPower = vTxLowPower;
        oldvTxHighPower = vTxHighPower;
        fcSettingChanged |= vTxSettingChanged;
        vTxSettingChanged = false;
        return (void*)MainMenu;
      }
    break;
    case 5:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        activeVTXMenuItem = 0;
        cleanScreen();
        vTxBand = oldvTxBand;
        vTxChannel = oldvTxChannel;
        vTxLowPower = oldvTxLowPower;
        vTxHighPower = oldvTxHighPower;
        vTxSettingChanged = false;
        return (void*)MainMenu;
      }
    break;
  }
    
  static const uint8_t VTX_MENU_ITEMS = 6;
  activeVTXMenuItem = checkMenuItem(activeVTXMenuItem, VTX_MENU_ITEMS);
  
  static const char VTX_LOW_POWER_STR[] PROGMEM =  "low power : ";
  static const char VTX_HIGH_POWER_STR[] PROGMEM = "high power: ";
  static const char VTX_BAND_STR[] PROGMEM =       "band      : ";
  static const char VTX_CHANNEL_STR[] PROGMEM =    "channel   : ";
//static const char SAVE_EXIT_STR[] PROGMEM =      "save+exit";
//static const char BACK_STR[] PROGMEM =           "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(VTX_HIGH_POWER_STR)+6)/2;
  static const char VTX_TITLE_STR[] PROGMEM = "vtx menu";
  OSD.printP( settings.COLS/2 - strlen_P(VTX_TITLE_STR)/2, ++startRow, VTX_TITLE_STR );

  OSD.printIntArrow(startCol, ++startRow, VTX_LOW_POWER_STR, vTxLowPower, 0, 1, activeVTXMenuItem, "mw", 1);

  OSD.printIntArrow(startCol, ++startRow, VTX_HIGH_POWER_STR, vTxHighPower, 0, 1, activeVTXMenuItem, "mw", 1);

  OSD.printP( startCol, ++startRow, VTX_BAND_STR, activeVTXMenuItem );
  OSD.print( fixStr(bandSymbols[vTxBand]) );
  
  OSD.printIntArrow( startCol, ++startRow, VTX_CHANNEL_STR, vTxChannel+1, 0, 1, activeVTXMenuItem, "=" );
  OSD.printInt16( startCol + strlen_P(VTX_CHANNEL_STR) + 3, startRow, (int16_t)pgm_read_word(&vtx_frequencies[vTxBand][vTxChannel]), 0, 1, "mhz" );
  
  OSD.printP( startCol, ++startRow, SAVE_EXIT_STR, activeVTXMenuItem );
  OSD.printP( startCol, ++startRow, BACK_STR, activeVTXMenuItem );
  
  return (void*)vTxMenu;
}
#endif


void* MainMenu()
{
  if(code &  inputChecker.ROLL_RIGHT || code &  inputChecker.ROLL_LEFT)
  {
    switch(activeMenuItem)
    {
      case 0:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          cleanScreen();
          return (void*)TuneMenu;
        }
      break;
      case 1:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          cleanScreen();
          return (void*)RatesMenu;
        }
      break;
      case 2:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          cleanScreen();
          return (void*)FilterMenu;
        }
      break;      
      case 3:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          cleanScreen();
          return (void*)BatteryMenu;
        }
      break;
      case 4:
#ifdef IMPULSERC_VTX
        if(code &  inputChecker.ROLL_RIGHT)
        {
          cleanScreen();
          return (void*)vTxMenu;
        }
#else
        if(code &  inputChecker.ROLL_RIGHT && vTxType > 0)
        {
          cleanScreen();
          return (void*)vTxMenu;
        }
#endif
      break;
      case 5:
        symbolOnOffChanged = checkCode(settings.m_displaySymbols, 1, 0, 1);
        settingChanged |= symbolOnOffChanged;
      break;
      case 6:
        settingChanged |= checkCode(settings.m_airTimer, 1, 0, 1);        
      break;
      case 7:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          menuActive = false;
          menuWasActive = true;
        }
      break;
      case 8:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          menuActive = false;
          menuWasActive = true;
          settingChanged = false;
          fcSettingChanged = false;
          settings.ReadSettings();
          fcSettingsReceived = false;
        }
      break;
    }
  }
  static const uint8_t MAIN_MENU_ITEMS = 9;
  activeMenuItem = checkMenuItem(activeMenuItem, MAIN_MENU_ITEMS);
  
  static const char PID_STR[] PROGMEM =             "tune";
  static const char RATES_STR[] PROGMEM =           "rates";
  static const char FILTER_STR[] PROGMEM =          "filters";
  static const char BATTERY_PAGE_STR[] PROGMEM =    "battery";
  static const char VTX_PAGE_STR[] PROGMEM =        "vtx";
  static const char SYMBOLS_SIZE_STR[] PROGMEM =    "symbols  : ";
  static const char AIR_TIMER_STR[] PROGMEM =       "air timer: ";
//static const char SAVE_EXIT_STR[] PROGMEM =       "save+exit";
  static const char CANCEL_STR[] PROGMEM =          "cancel";
  
  uint8_t startRow = 0;
  uint8_t startCol = settings.COLS/2 - strlen_P(SYMBOLS_SIZE_STR)/2;
  OSD.setCursor( settings.COLS/2 - strlen(KISS_OSD_VER)/2, ++startRow );
  OSD.print( fixStr(KISS_OSD_VER) );
  static const char MAIN_TITLE_STR[] PROGMEM = "main menu";
  OSD.printP( settings.COLS/2 - strlen_P(MAIN_TITLE_STR)/2, ++startRow, MAIN_TITLE_STR );
  
  OSD.printP( startCol, ++startRow, PID_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, RATES_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, FILTER_STR, activeMenuItem);  
  OSD.printP( startCol, ++startRow, BATTERY_PAGE_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, VTX_PAGE_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, SYMBOLS_SIZE_STR, activeMenuItem );
  OSD.print( fixStr(ON_OFF_STR[settings.m_displaySymbols]) );
  OSD.printP( startCol, ++startRow, AIR_TIMER_STR, activeMenuItem );
  OSD.print( fixStr(ON_OFF_STR[settings.m_airTimer]) );
  OSD.printP( startCol, ++startRow, SAVE_EXIT_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, CANCEL_STR, activeMenuItem );
  
  return (void*)MainMenu;
}
