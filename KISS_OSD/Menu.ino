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
static uint8_t activeCustomTPAMenuItem = 0;
static uint8_t activeLPFMenuItem = 0;
static uint8_t activeMAHCorrectionMenuItem = 0;
#ifdef BF32_MODE
static const int16_t P_STEP = 10;
static const int16_t I_STEP = 10;
static const int16_t D_STEP = 10;
static const int16_t TPA_STEP = 10;
static const int16_t RATE_STEP = 10;
#else
static const int16_t P_STEP = 100;
static const int16_t I_STEP = 1;
static const int16_t D_STEP = 1000;
static const int16_t TPA_STEP = 50;
static const int16_t RATE_STEP = 50;
#endif

static const char SAVE_EXIT_STR[] PROGMEM = "save+exit";
static const char BACK_STR[] PROGMEM =      "back";
static const char ON_OFF_STR[][4] PROGMEM = { "off", "on " };
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

boolean checkCode(uint8_t &value, int16_t STEP, int16_t minVal = 0, int16_t maxVal = 32000)
{
  int16_t tempValue = (int16_t)value;
  boolean changed = checkCode(tempValue, STEP, minVal, maxVal);
  value = (uint8_t) tempValue;
  return changed;
}

boolean checkCode(uint16_t &value, int16_t STEP, int16_t minVal = 0, int16_t maxVal = 32000)
{
  int16_t tempValue = (int16_t)value;
  boolean changed = checkCode(tempValue, STEP, minVal, maxVal);
  value = (uint16_t) tempValue;
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

void* ThreeItemPlusBackMenu(bool &settingChanged, uint8_t &active3MenuItem, uint16_t &item1, uint16_t &item2, uint16_t &item3, int16_t item1_step, int16_t item2_step, int16_t item3_step, char* title, void* prevPage, void* thisPage, const char *itemDescription1 = 0, const char *itemDescription2 = 0, const char *itemDescription3 = 0)
{
  #ifdef BF32_MODE
  const int16_t minVal = 0;
  const int16_t maxVal = 255;
  #else
  const int16_t minVal = 0;
  const int16_t maxVal = 32000;
  #endif  
  switch(active3MenuItem)
  {
    case 0:
      settingChanged |= checkCode(item1, item1_step, minVal, maxVal);
    break;
    case 1:
      settingChanged |= checkCode(item2, item2_step, minVal, maxVal);        
    break;
    case 2:
      settingChanged |= checkCode(item3, item3_step, minVal, maxVal);        
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
  
  static const uint8_t MENU_ITEMS = 5;
  
  active3MenuItem = checkMenuItem(active3MenuItem, MENU_ITEMS);
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(itemDescription1)+6)/2;
  OSD.setCursor( settings.COLS/2 - strlen(title)/2, ++startRow );
  OSD.print( fixStr(title) );

  #ifdef BF32_MODE
  const uint8_t dec = 0;
  #else
  const uint8_t dec = 3;
  #endif
  OSD.printIntArrow( startCol, ++startRow, itemDescription1, item1, dec, active3MenuItem, "", 1);
  OSD.printIntArrow( startCol, ++startRow, itemDescription2, item2, dec, active3MenuItem, "", 1);
  OSD.printIntArrow( startCol, ++startRow, itemDescription3, item3, dec, active3MenuItem, "", 1);
  OSD.printP( startCol, ++startRow, BACK_STR, active3MenuItem);
  OSD.printP( startCol, ++startRow, SAVE_EXIT_STR, active3MenuItem);
  
  return thisPage;
}

void* ThreeItemPlusBackMenu(bool &settingChanged, uint8_t &active3MenuItem, uint8_t &item1, uint8_t &item2, uint8_t &item3, int16_t item1_step, int16_t item2_step, int16_t item3_step, char* title, void* prevPage, void* thisPage, const char *itemDescription1 = 0, const char *itemDescription2 = 0, const char *itemDescription3 = 0)
{
  uint16_t tempItem1 = item1;
  uint16_t tempItem2 = item2;
  uint16_t tempItem3 = item3;
  void* temp = ThreeItemPlusBackMenu(settingChanged, active3MenuItem, tempItem1, tempItem2, tempItem3, item1_step, item2_step, item3_step, title, prevPage, thisPage, itemDescription1, itemDescription2, itemDescription3);
  item1 = tempItem1;
  item2 = tempItem2;
  item3 = tempItem3;
  return temp;
}

#ifdef BF32_MODE
static const char RATES_DESC_STR1[] PROGMEM = "rc rate   :"; 
static const char RATES_DESC_STR2[] PROGMEM = "super rate:"; 
static const char RATES_DESC_STR3[] PROGMEM = "rc expo   :";
#else
static const char RATES_DESC_STR1[] PROGMEM = "rc rate  : "; 
static const char RATES_DESC_STR2[] PROGMEM = "rate     : "; 
static const char RATES_DESC_STR3[] PROGMEM = "rc curve : ";
#endif

void* RatesRollMenu()
{
  return ThreeItemPlusBackMenu(fcSettingModeChanged[FC_RATES], activeRatesRollMenuItem, rcrate[_ROLL], rate[_ROLL], rccurve[_ROLL], RATE_STEP, RATE_STEP, RATE_STEP, "roll", (void*) RatesMenu, (void*) RatesRollMenu, RATES_DESC_STR1, RATES_DESC_STR2, RATES_DESC_STR3);
}

void* RatesPitchMenu()
{
  return ThreeItemPlusBackMenu(fcSettingModeChanged[FC_RATES], activeRatesPitchMenuItem, rcrate[_PITCH], rate[_PITCH], rccurve[_PITCH], RATE_STEP, RATE_STEP, RATE_STEP, "pitch", (void*)RatesMenu, (void*) RatesPitchMenu, RATES_DESC_STR1, RATES_DESC_STR2, RATES_DESC_STR3);
}

void* RatesYawMenu()
{
  return ThreeItemPlusBackMenu(fcSettingModeChanged[FC_RATES], activeRatesPitchMenuItem, rcrate[_YAW], rate[_YAW], rccurve[_YAW], RATE_STEP, RATE_STEP, RATE_STEP, "yaw", (void*)RatesMenu, (void*) RatesYawMenu, RATES_DESC_STR1, RATES_DESC_STR2, RATES_DESC_STR3);
}


#ifdef BF32_MODE
void* RatesMenu()
{
  OSD.topOffset = 1;
  boolean newrateProfile;
  switch(activeRatesMenuItem)
  {
    case 0:
      newrateProfile = checkCode(rateProfile, 1, 0, 2);
      if(newrateProfile)
      {
        rateProfileChanged = true;
        rateProfileSelected = rateProfile;
        SendFCSettings(210); //MSP_SELECT_SETTING
        fcSettingModeChanged[FC_RATES] |= rateProfileChanged;
        newrateProfile = false;
      }
      if(rateProfileSelected != rateProfile) mspRequest(111);//MSP_RC_TUNING
    break;
    case 1:
      fcSettingModeChanged[FC_RATES] |= checkCode(bf32_rates.rcRate, 10, 0, 255);
    break;
    case 2:
      fcSettingModeChanged[FC_RATES] |= checkCode(bf32_rates.rates[0], 10, 0, 100);
    break;
    case 3:
      fcSettingModeChanged[FC_RATES] |= checkCode(bf32_rates.rates[1], 10, 0, 100);
    break;
    case 4:
      fcSettingModeChanged[FC_RATES] |= checkCode(bf32_rates.rcExpo, 10, 0, 100);
    break;
    case 5:
      fcSettingModeChanged[FC_RATES] |= checkCode(bf32_rates.rc_yawRate, 10, 0, 255);
    break;
    case 6:
      fcSettingModeChanged[FC_RATES] |= checkCode(bf32_rates.rates[2], 10, 0, 100);
    break;
    case 7:
      fcSettingModeChanged[FC_RATES] |= checkCode(bf32_rates.rc_yawExpo, 10, 0, 100);
    break;
    case 8:
      fcSettingModeChanged[FC_RATES] |= checkCode(bf32_rates.thr_Mid, 10, 0, 100);
    break;
    case 9:
      fcSettingModeChanged[FC_RATES] |= checkCode(bf32_rates.thr_Expo, 10, 0, 100);
    break;
    case 10:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        activeRatesMenuItem = 0;
        menuActive = false;
        menuWasActive = true;
        OSD.topOffset = 3;
      }
    break;
    case 11:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        activeRatesMenuItem = 0;
        OSD.topOffset = 3;
        return (void*)MainMenu;
      }
    break;
  }

  static const uint8_t RATES_MENU_ITEMS = 12;
  activeRatesMenuItem = checkMenuItem(activeRatesMenuItem, RATES_MENU_ITEMS);

  static const char RATE_PROFILE_STR[] PROGMEM =     "rate profile:";
  static const char RC_RATE_RP_STR[] PROGMEM =       "rc rate r/p :";
  static const char SUPER_RATE_ROLL_STR[] PROGMEM =  "s rate roll :";
  static const char SUPER_RATE_PITCH_STR[] PROGMEM = "s rate pitch:";
  static const char RC_EXPO_RP_STR[] PROGMEM =       "rc expo r/p :";
  static const char RC_RATE_YAW_STR[] PROGMEM =      "rc rate yaw :";
  static const char SUPER_RATE_YAW_STR[] PROGMEM =   "s rate yaw  :";
  static const char RC_EXPO_YAW[] PROGMEM =          "rc expo yaw :";
  static const char THR_MID_STR[] PROGMEM =          "thrtl mid   :";
  static const char THR_EXPO_STR[] PROGMEM =         "thrtl expo  :";
//static const char SAVE_EXIT_STR[] PROGMEM =        "save+exit";
//static const char BACK_STR[] PROGMEM =             "back";
  
  uint8_t startRow = 0;
  uint8_t startCol = settings.COLS/2 - (strlen_P(RC_RATE_RP_STR)+4)/2;
  static const char RATES_TITLE_STR[] PROGMEM = "rates menu";
  OSD.printP(settings.COLS/2 - strlen_P(RATES_TITLE_STR)/2, startRow, RATES_TITLE_STR);

  OSD.printIntArrow( startCol, ++startRow, RATE_PROFILE_STR, rateProfile, 0, activeRatesMenuItem);
  OSD.printIntArrow( startCol, ++startRow, RC_RATE_RP_STR, bf32_rates.rcRate, 2, activeRatesMenuItem, "", 1);
  OSD.printIntArrow( startCol, ++startRow, SUPER_RATE_ROLL_STR, bf32_rates.rates[0], 2, activeRatesMenuItem, "", 1);
  OSD.printIntArrow( startCol, ++startRow, SUPER_RATE_PITCH_STR, bf32_rates.rates[1], 2, activeRatesMenuItem, "", 1);
  OSD.printIntArrow( startCol, ++startRow, RC_EXPO_RP_STR, bf32_rates.rcExpo, 2, activeRatesMenuItem, "", 1);
  OSD.printIntArrow( startCol, ++startRow, RC_RATE_YAW_STR, bf32_rates.rc_yawRate, 2, activeRatesMenuItem, "", 1);
  OSD.printIntArrow( startCol, ++startRow, SUPER_RATE_YAW_STR, bf32_rates.rates[2], 2, activeRatesMenuItem, "", 1);
  OSD.printIntArrow( startCol, ++startRow, RC_EXPO_YAW, bf32_rates.rc_yawExpo, 2, activeRatesMenuItem, "", 1);
  OSD.printIntArrow( startCol, ++startRow, THR_MID_STR, bf32_rates.thr_Mid, 2, activeRatesMenuItem, "", 1);
  OSD.printIntArrow( startCol, ++startRow, THR_EXPO_STR, bf32_rates.thr_Expo, 2, activeRatesMenuItem, "", 1);
  OSD.printP( startCol, ++startRow, SAVE_EXIT_STR, activeRatesMenuItem );
  OSD.printP( startCol, ++startRow, BACK_STR, activeRatesMenuItem );

  return (void*)RatesMenu;
}


#else


void* RatesMenu()
{
  switch(activeRatesMenuItem)
  {
    case 0:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        return (void*)RatesRollMenu;
      }
    break;
    case 1:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        return (void*)RatesPitchMenu;
      }
    break;
    case 2:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        return (void*)RatesYawMenu;
      }
    break;
    case 3:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        activeRatesMenuItem = 0;
        return (void*)MainMenu;
      }
    break;
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
#endif

extern void* TuneMenu();

static const char PID_DESC_STR1[] PROGMEM = "p : "; 
static const char PID_DESC_STR2[] PROGMEM = "i : "; 
static const char PID_DESC_STR3[] PROGMEM = "d : ";

void* PIDRollMenu()
{
  return ThreeItemPlusBackMenu(fcSettingModeChanged[FC_PIDS], activePIDRollMenuItem,  pid_p[_ROLL], pid_i[_ROLL], pid_d[_ROLL], P_STEP, I_STEP, D_STEP, "roll", (void*)TuneMenu, (void*) PIDRollMenu, PID_DESC_STR1, PID_DESC_STR2, PID_DESC_STR3);
}

void* PIDPitchMenu()
{
  return ThreeItemPlusBackMenu(fcSettingModeChanged[FC_PIDS], activePIDPitchMenuItem,  pid_p[_PITCH], pid_i[_PITCH], pid_d[_PITCH], P_STEP, I_STEP, D_STEP, "pitch", (void*)TuneMenu, (void*) PIDPitchMenu, PID_DESC_STR1, PID_DESC_STR2, PID_DESC_STR3);
}


#ifdef BF32_MODE
void* PIDYawMenu()
{
  switch(activePIDYawMenuItem)
  {
    case 0:
      fcSettingModeChanged[FC_PIDS] |= checkCode(pid_p[_YAW], 10, 0, 255);
    break;
    case 1:
      fcSettingModeChanged[FC_PIDS] |= checkCode(pid_i[_YAW], 10, 0, 255);
    break;     
    case 2:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        activePIDYawMenuItem = 0;
        menuActive = false;
        menuWasActive = true;
      }
    case 3:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        activePIDYawMenuItem = 0;
        return (void*)TuneMenu;
      }
    break;
  }

  static const uint8_t PID_YAW_MENU_ITEMS = 4;
  activePIDYawMenuItem = checkMenuItem(activePIDYawMenuItem, PID_YAW_MENU_ITEMS);
  
//static const char PID_DESC_STR1[] PROGMEM =    "p : "; 
//static const char PID_DESC_STR2[] PROGMEM =    "i : "; 
//static const char SAVE_EXIT_STR[] PROGMEM =    "save+exit";
//static const char BACK_STR[] PROGMEM =         "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - strlen_P(SAVE_EXIT_STR)/2;
  static const char PID_YAW_MENU_TITLE_STR[] PROGMEM = "yaw";
  OSD.printP(settings.COLS/2 - strlen_P(PID_YAW_MENU_TITLE_STR)/2, ++startRow, PID_YAW_MENU_TITLE_STR);
  
  OSD.printIntArrow( startCol, ++startRow, PID_DESC_STR1, pid_p[_YAW], 0, activePIDYawMenuItem, "", 1);

  OSD.printIntArrow( startCol, ++startRow, PID_DESC_STR2, pid_i[_YAW], 0, activePIDYawMenuItem, "", 1);
  
  OSD.printP( startCol, ++startRow, SAVE_EXIT_STR, activePIDYawMenuItem );
  OSD.printP( startCol, ++startRow, BACK_STR, activePIDYawMenuItem);
  
  return(void*)PIDYawMenu;
}
#else
void* PIDYawMenu()
{
  return ThreeItemPlusBackMenu(fcSettingModeChanged[FC_PIDS], activePIDYawMenuItem,  pid_p[_YAW], pid_i[_YAW], pid_d[_YAW], P_STEP, I_STEP, D_STEP, "yaw", (void*)TuneMenu, (void*) PIDYawMenu, PID_DESC_STR1, PID_DESC_STR2, PID_DESC_STR3);
}
#endif

#ifdef BF32_MODE
void* TPAMenu()
{
  switch(activeTPAMenuItem)
  {
    case 0:
      fcSettingModeChanged[FC_TPA] |= checkCode(bf32_rates.dynThrPID, 10, 0, 100);
    break;
    case 1:
      fcSettingModeChanged[FC_TPA] |= checkCode(bf32_rates.tpa_breakpoint, 10, 1000, 2000);
    break;     
    case 2:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        activeTPAMenuItem = 0;
        menuActive = false;
        menuWasActive = true;
      }
    case 3:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        activeTPAMenuItem = 0;
        return (void*)TuneMenu;
      }
    break;
  }

  static const uint8_t TPA_MENU_ITEMS = 4;
  activeTPAMenuItem = checkMenuItem(activeTPAMenuItem, TPA_MENU_ITEMS);
  
  static const char TPA_BF32_STR[] PROGMEM =     "tpa       :";
  static const char TPA_BREAK_STR[] PROGMEM =    "tpa breakp:";
//static const char SAVE_EXIT_STR[] PROGMEM =    "save+exit";
//static const char BACK_STR[] PROGMEM =         "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(TPA_BF32_STR)+3)/2;
  static const char TPA_MENU_TITLE_STR[] PROGMEM = "tpa menu";
  OSD.printP(settings.COLS/2 - strlen_P(TPA_MENU_TITLE_STR)/2, ++startRow, TPA_MENU_TITLE_STR);
  
  OSD.printIntArrow( startCol, ++startRow, TPA_BF32_STR, bf32_rates.dynThrPID, 2, activeTPAMenuItem, "", 1);

  OSD.printIntArrow( startCol, ++startRow, TPA_BREAK_STR, bf32_rates.tpa_breakpoint, 0, activeTPAMenuItem, "", 1);
  
  OSD.printP( startCol, ++startRow, SAVE_EXIT_STR, activeTPAMenuItem );
  OSD.printP( startCol, ++startRow, BACK_STR, activeTPAMenuItem);
  
  return (void*)TPAMenu;
}
#else
void* TPAMenu()
{
  static const char TPA_DESC_STR1[] PROGMEM = "tpa p : "; 
  static const char TPA_DESC_STR2[] PROGMEM = "tpa i : "; 
  static const char TPA_DESC_STR3[] PROGMEM = "tpa d : ";
  return ThreeItemPlusBackMenu(fcSettingModeChanged[FC_TPA], activeTPAMenuItem,  fc_tpa.tpa[0], fc_tpa.tpa[1], fc_tpa.tpa[2], TPA_STEP, TPA_STEP, TPA_STEP, "tpa menu", (void*)TuneMenu, (void*) TPAMenu, TPA_DESC_STR1, TPA_DESC_STR2, TPA_DESC_STR3);
}
#endif


#ifndef BF32_MODE
static const char LPF_FRQ_STR[][9] PROGMEM = { {"off     "}, 
                                               {"high    "}, 
                                               {"med high"}, 
                                               {"medium  "}, 
                                               {"med low "},
                                               {"low     "},
                                               {"very low"}, 
                                               {"advanced"} };

void* LPFMenu()
{
  switch(activeLPFMenuItem)
  {
    case 0:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(fc_filters.lpf_frq, 1, 0, 6);
    break;
    case 1:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(fc_filters.yawLpF, 1, 0, 6);
    break;
    case 2:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(fc_filters.DLpF, 1, 0, 6);
    break;     
    case 3:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        activeLPFMenuItem = 0;
        menuActive = false;
        menuWasActive = true;
      }
    case 4:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        activeLPFMenuItem = 0;
        return (void*)FilterMenu;
      }
    break;
  }

  static const uint8_t LPF_MENU_ITEMS = 5;
  activeLPFMenuItem = checkMenuItem(activeLPFMenuItem, LPF_MENU_ITEMS);
  
  static const char LPF_ROLL_PITCH_STR[] PROGMEM =     "roll/pitch lpf:";
  static const char LPF_YAW_STR[] PROGMEM =            "yaw lpf       :";
  static const char LPF_DTERM_STROLL_STR[] PROGMEM =   "dterm lpf     :";
//static const char SAVE_EXIT_STR[] PROGMEM =          "save+exit";
//static const char BACK_STR[] PROGMEM =               "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(LPF_ROLL_PITCH_STR)+8)/2;
  static const char LPF_MENU_TITLE_STR[] PROGMEM = "advanced lpf menu";
  OSD.printP(settings.COLS/2 - strlen_P(LPF_MENU_TITLE_STR)/2, ++startRow, LPF_MENU_TITLE_STR);
  
  OSD.printP( startCol, ++startRow, LPF_ROLL_PITCH_STR, activeLPFMenuItem);
  OSD.print( fixPStr(LPF_FRQ_STR[fc_filters.lpf_frq]) );

  OSD.printP( startCol, ++startRow, LPF_YAW_STR, activeLPFMenuItem);
  OSD.print( fixPStr(LPF_FRQ_STR[fc_filters.yawLpF]) );

  OSD.printP( startCol, ++startRow, LPF_DTERM_STROLL_STR, activeLPFMenuItem);
  OSD.print( fixPStr(LPF_FRQ_STR[fc_filters.DLpF]) );
  
  OSD.printP( startCol, ++startRow, SAVE_EXIT_STR, activeLPFMenuItem );
  OSD.printP( startCol, ++startRow, BACK_STR, activeLPFMenuItem);
  
  return (void*)LPFMenu;
}

void* FilterMenu()
{
  switch(activeFilterMenuItem)
  {
    case 0:
      if(moreLPFfilters)
      {
        if(code &  inputChecker.ROLL_RIGHT)
        {
          cleanScreen();
          return (void*)LPFMenu;
        }
      }
      else fcSettingModeChanged[FC_FILTERS] |= checkCode(fc_filters.lpf_frq, 1, 0, 6);
    break;
    case 1:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(fc_filters.yawFilterCut, 10, 0, 97);
    break;
    case 2:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(fc_filters.notchFilterEnabledR, 1, 0, 1);
    break;
    case 3:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(fc_filters.notchFilterCenterR, 10, 0, 490);
    break;
    case 4:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(fc_filters.notchFilterCutR, 10, 0, 490);
    break;
    case 5:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(fc_filters.notchFilterEnabledP, 1, 0, 1);
    break;
    case 6:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(fc_filters.notchFilterCenterP, 10, 0, 490);
    break;
    case 7:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(fc_filters.notchFilterCutP, 10, 0, 490);
    break;        
    case 8:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        activeFilterMenuItem = 0;
        menuActive = false;
        menuWasActive = true;
      }
    case 9:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        activeFilterMenuItem = 0;
        return (void*)MainMenu;
      }
    break;
  }

  static const uint8_t FILTER_MENU_ITEMS = 10;
  activeFilterMenuItem = checkMenuItem(activeFilterMenuItem, FILTER_MENU_ITEMS);

  static const char LPF_STR[] PROGMEM =                 "lpf      : ";
  static const char LPF2_STR[] PROGMEM =                "lpf";
  static const char YAW_FLTR_STR[] PROGMEM =            "yaw fltr strength:";
  static const char NOTCH_ROLL_STR[] PROGMEM =          "notch roll fltr  :";
  static const char NOTCH_ROLL_CENTER_STR[] PROGMEM =   "roll center freq :";
  static const char NOTCH_ROLL_CUTOFF_STR[] PROGMEM =   "roll cutoff freq :";
  static const char NOTCH_PITCH_STR[] PROGMEM =         "notch pitch fltr :";
  static const char NOTCH_PITCH_CENTER_STR[] PROGMEM =  "pitch center freq:";
  static const char NOTCH_PITCH_CUTOFF_STR[] PROGMEM =  "pitch cutoff freq:";
//static const char SAVE_EXIT_STR[] PROGMEM =           "save+exit";
//static const char BACK_STR[] PROGMEM =                "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(YAW_FLTR_STR)+8)/2;
  static const char FILTER_MENU_TITLE_STR[] PROGMEM = "filter menu";
  OSD.printP(settings.COLS/2 - strlen_P(FILTER_MENU_TITLE_STR)/2, ++startRow, FILTER_MENU_TITLE_STR);

  if(moreLPFfilters) OSD.printP( startCol, ++startRow, LPF2_STR, activeFilterMenuItem);
  else
  {
    OSD.printP( startCol, ++startRow, LPF_STR, activeFilterMenuItem);
    OSD.print( fixPStr(LPF_FRQ_STR[fc_filters.lpf_frq]) );
  }
  OSD.printIntArrow( startCol, ++startRow, YAW_FLTR_STR, fc_filters.yawFilterCut, 0, activeFilterMenuItem, "", 1);
  OSD.printP( startCol, ++startRow, NOTCH_ROLL_STR, activeFilterMenuItem);
  OSD.print( fixPStr(ON_OFF_STR[fc_filters.notchFilterEnabledR]) );
  OSD.printIntArrow( startCol, ++startRow, NOTCH_ROLL_CENTER_STR, fc_filters.notchFilterCenterR, 0, activeFilterMenuItem, "hz", 1);
  OSD.printIntArrow( startCol, ++startRow, NOTCH_ROLL_CUTOFF_STR, fc_filters.notchFilterCutR, 0, activeFilterMenuItem, "hz", 1);
  OSD.printP( startCol, ++startRow, NOTCH_PITCH_STR, activeFilterMenuItem);
  OSD.print( fixPStr(ON_OFF_STR[fc_filters.notchFilterEnabledP]) );
  OSD.printIntArrow( startCol, ++startRow, NOTCH_PITCH_CENTER_STR, fc_filters.notchFilterCenterP, 0, activeFilterMenuItem, "hz", 1); 
  OSD.printIntArrow( startCol, ++startRow, NOTCH_PITCH_CUTOFF_STR, fc_filters.notchFilterCutP, 0, activeFilterMenuItem, "hz", 1);
  OSD.printP( startCol, ++startRow, SAVE_EXIT_STR, activeFilterMenuItem );
  OSD.printP( startCol, ++startRow, BACK_STR, activeFilterMenuItem);
  
  return (void*)FilterMenu;  
}

void* CustomTPAMenu()
{
  switch(activeCustomTPAMenuItem)
  {
    case 0:
      fcSettingModeChanged[FC_TPA] |= checkCode(fc_tpa.customTPAEnabled, 1, 0, 1);
    break;
    case 1:
      fcSettingModeChanged[FC_TPA] |= checkCode(fc_tpa.ctpa_infl[0], 10, 0, 100);
    break;
    case 2:
      fcSettingModeChanged[FC_TPA] |= checkCode(fc_tpa.ctpa_bp1, 10, 0, 100);
    break;
    case 3:
      fcSettingModeChanged[FC_TPA] |= checkCode(fc_tpa.ctpa_infl[1], 10, 0, 100);
    break;
    case 4:
      fcSettingModeChanged[FC_TPA] |= checkCode(fc_tpa.ctpa_bp2, 10, 0, 100);
    break;
    case 5:
      fcSettingModeChanged[FC_TPA] |= checkCode(fc_tpa.ctpa_infl[2], 10, 0, 100);
    break;
    case 6:
      fcSettingModeChanged[FC_TPA] |= checkCode(fc_tpa.ctpa_infl[3], 10, 0, 100);
    break;
    case 7:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        activeCustomTPAMenuItem = 0;
        menuActive = false;
        menuWasActive = true;
      }
    break;
    case 8:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        activeCustomTPAMenuItem = 0;
        return (void*)TuneMenu;
      }
    break;
  }
  static const uint8_t CUSTOM_TPA_MENU_ITEMS = 9;
  activeCustomTPAMenuItem = checkMenuItem(activeCustomTPAMenuItem, CUSTOM_TPA_MENU_ITEMS);
  
  static const char CSTM_TPA_ACTIVE_STR[] PROGMEM =  "custom tpa    : ";
  static const char INFLUENCE_ZERO_STR[] PROGMEM =   "influence 0%  : ";
  static const char BREAKPOINT_ONE_STR[] PROGMEM =   "breakpoint 1  : ";
  static const char INFLUENCE_BP1_STR[] PROGMEM =    "influence bp1 : ";
  static const char BREAKPOINT_TWO_STR[] PROGMEM =   "breakpoint 2  : ";
  static const char INFLUENCE_BP2_STR[] PROGMEM =    "influence bp2 : ";
  static const char INFLUENCE_MAX_STR[] PROGMEM =    "influence 100%: ";
//static const char SAVE_EXIT_STR[] PROGMEM =        "save+exit";
//static const char BACK_STR[] PROGMEM =             "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(INFLUENCE_ZERO_STR)+4)/2;
  static const char CUSTOM_TPA_TITLE_STR[] PROGMEM = "custom tpa menu";
  OSD.printP(settings.COLS/2 - strlen_P(CUSTOM_TPA_TITLE_STR)/2, ++startRow, CUSTOM_TPA_TITLE_STR);
  
  OSD.printP( startCol, ++startRow, CSTM_TPA_ACTIVE_STR, activeCustomTPAMenuItem );
  OSD.print( fixPStr(ON_OFF_STR[fc_tpa.customTPAEnabled]) );
  
  OSD.printIntArrow( startCol, ++startRow, INFLUENCE_ZERO_STR, fc_tpa.ctpa_infl[0], 0, activeCustomTPAMenuItem, "%", 1 );

  OSD.printIntArrow( startCol, ++startRow, BREAKPOINT_ONE_STR, fc_tpa.ctpa_bp1, 0, activeCustomTPAMenuItem, "%", 1 );

  OSD.printIntArrow( startCol, ++startRow, INFLUENCE_BP1_STR, fc_tpa.ctpa_infl[1], 0, activeCustomTPAMenuItem, "%", 1 );

  OSD.printIntArrow( startCol, ++startRow, BREAKPOINT_TWO_STR, fc_tpa.ctpa_bp2, 0, activeCustomTPAMenuItem, "%", 1 );

  OSD.printIntArrow( startCol, ++startRow, INFLUENCE_BP2_STR, fc_tpa.ctpa_infl[2], 0, activeCustomTPAMenuItem, "%", 1 );

  OSD.printIntArrow( startCol, ++startRow, INFLUENCE_MAX_STR, fc_tpa.ctpa_infl[3], 0, activeCustomTPAMenuItem, "%", 1 );
  
  OSD.printP( startCol, ++startRow, SAVE_EXIT_STR, activeCustomTPAMenuItem );
  OSD.printP( startCol, ++startRow, BACK_STR, activeCustomTPAMenuItem );
  return (void*)CustomTPAMenu; 
}
#endif

#ifdef BF32_MODE
void* FilterMenu()
{
  OSD.topOffset = 1;
  switch(activeFilterMenuItem)
  {
    case 0:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(bf32_filters.gyro_soft_lpf_hz, 10, 0, 255);
    break;
    case 1:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(bf32_filters.gyro_soft_notch_hz_1, 10, 0, 1000);
    break;
    case 2:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(bf32_filters.gyro_soft_notch_cutoff_1, 10, 0, 1000);
    break;
    case 3:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(bf32_filters.gyro_soft_notch_hz_2, 10, 0, 1000);
    break;
    case 4:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(bf32_filters.gyro_soft_notch_cutoff_2, 10, 0, 1000);
    break;
    case 5:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(bf32_filters.dterm_filter_type, 1, 0, 2);
    break;
    case 6:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(bf32_filters.dterm_lpf_hz, 10, 0, 1000);
    break;
    case 7:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(bf32_filters.dterm_notch_hz, 10, 0, 1000);
    break;
    case 8:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(bf32_filters.dterm_notch_cutoff, 10, 0, 1000);
    break;
    case 9:
      fcSettingModeChanged[FC_FILTERS] |= checkCode(bf32_filters.yaw_lpf_hz, 10, 0, 1000);
    break;        
    case 10:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        activeFilterMenuItem = 0;
        menuActive = false;
        menuWasActive = true;
        OSD.topOffset = 3;
      }
    break;
    case 11:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        activeFilterMenuItem = 0;
        OSD.topOffset = 3;
        return (void*)MainMenu;
      }
    break;
  }

  static const uint8_t FILTER_MENU_ITEMS = 12;
  activeFilterMenuItem = checkMenuItem(activeFilterMenuItem, FILTER_MENU_ITEMS);

  static const char GYRO_SOFT_LPF_STR[] PROGMEM =       "gyro soft lpf   :";
  static const char GYRO_NOTCH1_STR[] PROGMEM =         "gyro notch1 freq:";
  static const char GYRO_NOTCH1_CUT_STR[] PROGMEM =     "gyro notch1 cut :";
  static const char GYRO_NOTCH2_STR[] PROGMEM =         "gyro notch2 freq:";
  static const char GYRO_NOTCH2_CUT_STR[] PROGMEM =     "gyro notch2 cut :";
  static const char DTERM_TYPE_STR[] PROGMEM =          "dterm type      :";
  static const char DTERM_LPF_STR[] PROGMEM =           "dterm lpf       :";
  static const char DTERM_NOTCH_STR[] PROGMEM =         "dterm notch freq:";
  static const char DTERM_NOTCH_CUT_STR[] PROGMEM =     "dterm notch cut :";
  static const char YAW_LPF_STR[] PROGMEM =             "yaw lpf         :";
//static const char SAVE_EXIT_STR[] PROGMEM =           "save+exit";
//static const char BACK_STR[] PROGMEM =                "back";
  
  uint8_t startRow = 0;
  uint8_t startCol = settings.COLS/2 - (strlen_P(GYRO_SOFT_LPF_STR)+7)/2;
  static const char FILTER_MENU_TITLE_STR[] PROGMEM = "filter menu";
  OSD.printP(settings.COLS/2 - strlen_P(FILTER_MENU_TITLE_STR)/2, startRow, FILTER_MENU_TITLE_STR);

  static const char HZ_SUFFIX[] = {"hz"};
  OSD.printIntArrow( startCol, ++startRow, GYRO_SOFT_LPF_STR, bf32_filters.gyro_soft_lpf_hz, 0, activeFilterMenuItem, HZ_SUFFIX, 1);
  OSD.printIntArrow( startCol, ++startRow, GYRO_NOTCH1_STR, bf32_filters.gyro_soft_notch_hz_1, 0, activeFilterMenuItem, HZ_SUFFIX, 1);
  OSD.printIntArrow( startCol, ++startRow, GYRO_NOTCH1_CUT_STR, bf32_filters.gyro_soft_notch_cutoff_1, 0, activeFilterMenuItem, HZ_SUFFIX, 1);
  OSD.printIntArrow( startCol, ++startRow, GYRO_NOTCH2_STR, bf32_filters.gyro_soft_notch_hz_2, 0, activeFilterMenuItem, HZ_SUFFIX, 1);
  OSD.printIntArrow( startCol, ++startRow, GYRO_NOTCH2_CUT_STR, bf32_filters.gyro_soft_notch_cutoff_2, 0, activeFilterMenuItem, HZ_SUFFIX, 1);
  static const char DTERM_TYPE_STRS[][7] PROGMEM = { {"pt1   "}, 
                                                     {"biquad"}, 
                                                     {"fir   "}};
  OSD.printP( startCol, ++startRow, DTERM_TYPE_STR, activeFilterMenuItem);
  OSD.print( fixPStr(DTERM_TYPE_STRS[bf32_filters.dterm_filter_type]) );
  OSD.printIntArrow( startCol, ++startRow, DTERM_LPF_STR, bf32_filters.dterm_lpf_hz, 0, activeFilterMenuItem, HZ_SUFFIX, 1);
  OSD.printIntArrow( startCol, ++startRow, DTERM_NOTCH_STR, bf32_filters.dterm_notch_hz, 0, activeFilterMenuItem, HZ_SUFFIX, 1);
  OSD.printIntArrow( startCol, ++startRow, DTERM_NOTCH_CUT_STR, bf32_filters.dterm_notch_cutoff, 0, activeFilterMenuItem, HZ_SUFFIX, 1);
  OSD.printIntArrow( startCol, ++startRow, YAW_LPF_STR, bf32_filters.yaw_lpf_hz, 0, activeFilterMenuItem, HZ_SUFFIX, 1);
  OSD.printP( startCol, ++startRow, SAVE_EXIT_STR, activeFilterMenuItem );
  OSD.printP( startCol, ++startRow, BACK_STR, activeFilterMenuItem);
  return (void*)FilterMenu;
}
#endif

void* TuneMenu()
{
  if(code &  inputChecker.ROLL_RIGHT || code &  inputChecker.ROLL_LEFT)
  {
    switch(activeTuneMenuItem)
    {
      case 0:
          cleanScreen();
          return (void*)PIDRollMenu;
      break;
      case 1:
          cleanScreen();
          return (void*)PIDPitchMenu;
      break;
      case 2:
          cleanScreen();
          return (void*)PIDYawMenu;
      break;
      case 3:
          cleanScreen();
          return (void*)TPAMenu;
      break;
      #ifdef CUSTOM_TPA 
      case 4:
          cleanScreen();
          return (void*)CustomTPAMenu;
      break;        
      case 5:
      #elif defined(BF32_MODE)
      case 4:
          pidProfileChanged |= checkCode(pidProfile, 1, 0, 2);
          if(pidProfileChanged)
          {
            SendFCSettings(210); //MSP_SELECT_SETTING
            fcSettingModeChanged[FC_PIDS] = pidProfileChanged;
          }
      break;        
      case 5:
      #else
      case 4:
      #endif
          cleanScreen();
          activeTuneMenuItem = 0;
          return (void*)MainMenu;
      break;
    }
  }

  #if defined(CUSTOM_TPA) || defined(BF32_MODE)
  static const uint8_t TUNE_MENU_ITEMS = 6;
  #else
  static const uint8_t TUNE_MENU_ITEMS = 5;
  #endif
  activeTuneMenuItem = checkMenuItem(activeTuneMenuItem, TUNE_MENU_ITEMS);
  
//static const char ROLL_STR[] PROGMEM =        "roll";
//static const char PITCH_STR[] PROGMEM =       "pitch";
//static const char YAW_STR[] PROGMEM =         "yaw";
  static const char TPA_STR[] PROGMEM =         "tpa";
  #ifdef CUSTOM_TPA
  static const char CUSTOM_TPA_STR[] PROGMEM =  "custom tpa";
  #endif
  #ifdef BF32_MODE
  static const char PID_PROFILE_STR[] PROGMEM = "pid profile:";
  #endif  
//static const char BACK_STR[] PROGMEM =        "back";
  
  uint8_t startRow = 1;
  #ifdef CUSTOM_TPA
  uint8_t startCol = settings.COLS/2 - strlen_P(CUSTOM_TPA_STR)/2;
  #else
  uint8_t startCol = settings.COLS/2 - strlen_P(PITCH_STR)/2;
  #endif
  static const char TUNE_MENU_TITLE_STR[] PROGMEM = "tune menu";
  OSD.printP(settings.COLS/2 - strlen_P(TUNE_MENU_TITLE_STR)/2, ++startRow, TUNE_MENU_TITLE_STR);
  
  OSD.printP( startCol, ++startRow, ROLL_STR, activeTuneMenuItem);
  OSD.printP( startCol, ++startRow, PITCH_STR, activeTuneMenuItem);
  OSD.printP( startCol, ++startRow, YAW_STR, activeTuneMenuItem);
  OSD.printP( startCol, ++startRow, TPA_STR, activeTuneMenuItem);
  #ifdef CUSTOM_TPA
  OSD.printP( startCol, ++startRow, CUSTOM_TPA_STR, activeTuneMenuItem);  
  #endif
  #ifdef BF32_MODE
  OSD.printIntArrow( startCol, ++startRow, PID_PROFILE_STR, pidProfile, 0, activeTuneMenuItem);
  #endif
  OSD.printP( startCol, ++startRow, BACK_STR, activeTuneMenuItem);
  
  return (void*)TuneMenu;
}


#ifdef MAH_CORRECTION
void* MAHCorrectionMenu()
{
  switch(activeMAHCorrectionMenuItem)
  {
    case 0:
    case 1:
    case 2:
    case 3:
      settingChanged |= checkCode(settings.s.m_ESCCorrection[activeMAHCorrectionMenuItem], 10, 50, 200);
    break;
    case 4:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        menuActive = false;
        menuWasActive = true;
        activeMAHCorrectionMenuItem = 0;
      }
    break;
    case 5:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        activeMAHCorrectionMenuItem = 0;
        return (void*)BatteryMenu;
      }
    break;
  }

  static const uint8_t MAH_MENU_ITEMS = 6;
  
  activeMAHCorrectionMenuItem = checkMenuItem(activeMAHCorrectionMenuItem, MAH_MENU_ITEMS);

  char* ESC_STR2 = ESC_STAT_STR1;
  if(settings.s.m_displaySymbols == 1 && settings.m_IconSettings[ESC_ICON] == 1)
  {
    ESC_STR2 = ESCSymbol;
  } 

  static const char ESC_MAH_STR[] PROGMEM = " mah : ";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen(ESC_STR2)+2+strlen_P(ESC_MAH_STR)+3)/2;
  static const char MAH_TITLE_STR[] PROGMEM = "mah correction menu";
  OSD.printP(settings.COLS/2 - strlen_P(MAH_TITLE_STR)/2, ++startRow, MAH_TITLE_STR);
  
  for(uint8_t i=0; i<4; i++)
  {
    OSD.setCursor(startCol, ++startRow);
    OSD.checkArrow(startRow, activeMAHCorrectionMenuItem);
    OSD.setCursor(startCol+1, startRow);
    OSD.print(fixStr(ESC_STR2));
    OSD.printInt16( startCol + strlen(ESC_STR2) + 1, startRow, (int16_t)(i+1), 0);
    OSD.printInt16P( startCol + strlen(ESC_STR2) + 2, startRow, ESC_MAH_STR, settings.s.m_ESCCorrection[i], 0, "%", 1);
  }
  OSD.printP( startCol, ++startRow, SAVE_EXIT_STR, activeMAHCorrectionMenuItem );
  OSD.printP( startCol, ++startRow, BACK_STR, activeMAHCorrectionMenuItem );
  
  return (void*)MAHCorrectionMenu;
  
}
#endif


void* BatteryMenu()
{
  boolean changed = false;
  int16_t temp;
  switch(activeBatteryMenuItem)
  {
    case 0:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        batterySelect = true;
      }
    break;
    case 1:
      settingChanged |= checkCode(settings.s.m_batWarning, 1, 0, 1);
    break;
    case 2:
      changed = checkCode(settings.s.m_batWarningPercent, 1, 0, 100);
      settingChanged |= changed;
      if(changed)
      {
        settings.FixBatWarning();
      }
    break;
    case 3:
      settingChanged |= checkCode(settings.s.m_voltWarning, 1, 0, 1);
    break;
    case 4:
      settingChanged |= checkCode(settings.s.m_minVolts, 1, 90, 250);
    break;
    case 5:
      temp = (int16_t)settings.s.m_voltCorrect;
      settingChanged |= checkCode(temp, 1, -10, 10);
      settings.s.m_voltCorrect = (int8_t)temp;
    break;
    #ifdef MAH_CORRECTION
    case 6:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        return (void*)MAHCorrectionMenu;
      }
    break;
    case 7:
    #else
    case 6:
    #endif
      checkCode(settings.s.m_maxWatts, (int16_t)1000, (int16_t)1000, (int16_t)30000);
    break;
    #ifdef MAH_CORRECTION
    case 8:
    #else
    case 7:
    #endif
      if(code &  inputChecker.ROLL_RIGHT)
      {
        menuActive = false;
        menuWasActive = true;
        activeBatteryMenuItem = 0;
        settings.UpdateMaxWatt(settings.s.m_maxWatts);
      }
    break;
    #ifdef MAH_CORRECTION
    case 9:
    #else
    case 8:
    #endif
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        activeBatteryMenuItem = 0;
        settings.UpdateMaxWatt(settings.s.m_maxWatts);
        return (void*)MainMenu;
      }
    break;
  }
  #ifdef MAH_CORRECTION
  static const uint8_t BATTERY_MENU_ITEMS = 10;
  #else
  static const uint8_t BATTERY_MENU_ITEMS = 9;
  #endif
  activeBatteryMenuItem = checkMenuItem(activeBatteryMenuItem, BATTERY_MENU_ITEMS);
  
  static const char SELECT_BATTERY_STR[] PROGMEM =  "select battery ";
  static const char BATTERY_WARNING_STR[] PROGMEM = "batt. warning: ";
  static const char BATTERY_PERCENT_STR[] PROGMEM = "batt. % alarm: ";
  static const char VOLTAGE_WARN_STR[] PROGMEM =    "volt. warning: ";
  static const char MIN_VOLT_STR[] PROGMEM =        "min voltage  : ";
  static const char VOLT_CORRECT_STR[] PROGMEM =    "voltage corr : ";
  #ifdef MAH_CORRECTION
  static const char MAH_CORRECT_STR[] PROGMEM =     "mah correction ";
  #endif
  static const char MAX_BEER_WATT_STR[] PROGMEM =   "wattmeter max: ";
//static const char SAVE_EXIT_STR[] PROGMEM =       "save+exit";
//static const char BACK_STR[] PROGMEM =            "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(BATTERY_WARNING_STR)+6)/2;
  static const char BATTERY_TITLE_STR[] PROGMEM = "battery menu";
  OSD.printP(settings.COLS/2 - strlen_P(BATTERY_TITLE_STR)/2, ++startRow, BATTERY_TITLE_STR);
  
  OSD.printP( startCol, ++startRow, SELECT_BATTERY_STR, activeBatteryMenuItem );
  
  OSD.printP( startCol, ++startRow, BATTERY_WARNING_STR, activeBatteryMenuItem );
  OSD.print( fixPStr(ON_OFF_STR[settings.s.m_batWarning]) );
  
  OSD.printIntArrow( startCol, ++startRow, BATTERY_PERCENT_STR, settings.s.m_batWarningPercent, 0, activeBatteryMenuItem, "%", true );

  OSD.printP( startCol, ++startRow, VOLTAGE_WARN_STR, activeBatteryMenuItem );
  OSD.print( fixPStr(ON_OFF_STR[settings.s.m_voltWarning]) );
  
  OSD.printIntArrow( startCol, ++startRow, MIN_VOLT_STR, settings.s.m_minVolts, 1, activeBatteryMenuItem, "v", 1 );

  OSD.printIntArrow( startCol, ++startRow, VOLT_CORRECT_STR, settings.s.m_voltCorrect, 1, activeBatteryMenuItem, "v", 1 );

  #ifdef MAH_CORRECTION
  OSD.printP( startCol, ++startRow, MAH_CORRECT_STR, activeBatteryMenuItem );
  #endif

  OSD.printIntArrow( startCol, ++startRow, MAX_BEER_WATT_STR, settings.s.m_maxWatts/10, 0, activeBatteryMenuItem, "w", 1 );
  
  OSD.printP( startCol, ++startRow, SAVE_EXIT_STR, activeBatteryMenuItem );
  OSD.printP( startCol, ++startRow, BACK_STR, activeBatteryMenuItem );
  
  return (void*)BatteryMenu;
}

static bool vTxSettingChanged = false;

#ifdef IMPULSERC_VTX
void* vTxMenu()
{
  uint8_t maxPower = 2;
  #ifdef AUSSIE_CHANNELS
  if(settings.s.m_AussieChannels == 0) maxPower = 0;
  int8_t _oldvTxBand = (int8_t)settings.s.m_vTxBand;
  #endif
  switch(activeVTXMenuItem)
  {
    case 0:
      vTxSettingChanged |= checkCode(settings.s.m_vTxMinPower, 1, 0, maxPower);
    break;
    case 1:
      vTxSettingChanged |= checkCode(settings.s.m_vTxPower, 1, 0, maxPower);
    break;
    case 2:
      vTxSettingChanged |= checkCode(settings.s.m_vTxBand, 1, 0, VTX_BAND_COUNT);
      #ifdef AUSSIE_CHANNELS
      if(settings.s.m_vTxBand == 2 && settings.s.m_AussieChannels == 0)
      {
        settings.s.m_vTxBand += (int8_t)settings.s.m_vTxBand - (int8_t)_oldvTxBand;
      }
      #endif
    break;
    case 3:
      vTxSettingChanged |= checkCode(settings.s.m_vTxChannel, 1, 0, VTX_CHANNEL_COUNT);
    break;
    case 4:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        vTxMinPower = settings.s.m_vTxMinPower;
        vTxPower = settings.s.m_vTxPower;
        vTxBand = settings.s.m_vTxBand;
        vTxChannel = settings.s.m_vTxChannel;
        settingChanged |= vTxSettingChanged;
        menuActive = false;
        menuWasActive = true;
        vtx_set_frequency(vTxBand, vTxChannel);
        return (void*)MainMenu;
      }
    break;
    case 5:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        activeVTXMenuItem = 0;
        if(vTxSettingChanged)
        {
          settings.s.m_vTxMinPower = vTxMinPower;
          settings.s.m_vTxPower = vTxPower;
          settings.s.m_vTxBand = vTxBand;
          settings.s.m_vTxChannel = vTxChannel;
        }
        vTxSettingChanged = false;
        cleanScreen();
        return (void*)MainMenu;
      }
    break;
  }
  static const uint8_t VTX_MENU_ITEMS = 6;
  activeVTXMenuItem = checkMenuItem(activeVTXMenuItem, VTX_MENU_ITEMS);
  
  static const char VTX_MIN_POWER_STR[] PROGMEM =  "power disarmed:";
  static const char VTX_POWER_STR[] PROGMEM =      "power armed   :";
  static const char VTX_BAND_STR[] PROGMEM =       "band          :";
  static const char VTX_CHANNEL_STR[] PROGMEM =    "channel       :";
  static const char SET_EXIT_STR[] PROGMEM =       "set+exit";
//static const char BACK_STR[] PROGMEM =           "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(VTX_POWER_STR)+9)/2;
  static const char VTX_TITLE_STR[] PROGMEM = "vtx menu";
  OSD.printP( settings.COLS/2 - strlen_P(VTX_TITLE_STR)/2, ++startRow, VTX_TITLE_STR );

  static const char VTX_POWERS_STR[][6] PROGMEM = { {"25mw "}, {"200mw"}, {"500mw"} };
  OSD.printP( startCol, ++startRow, VTX_MIN_POWER_STR, activeVTXMenuItem );
  OSD.print( fixPStr(VTX_POWERS_STR[settings.s.m_vTxMinPower]) );
  
  OSD.printP( startCol, ++startRow, VTX_POWER_STR, activeVTXMenuItem );
  OSD.print( fixPStr(VTX_POWERS_STR[settings.s.m_vTxPower]) );

  OSD.printP( startCol, ++startRow, VTX_BAND_STR, activeVTXMenuItem );
  OSD.print( fixPStr(bandSymbols[settings.s.m_vTxBand]) );
  
  OSD.printIntArrow( startCol, ++startRow, VTX_CHANNEL_STR, settings.s.m_vTxChannel+1, 0, activeVTXMenuItem, "=" );
  OSD.printInt16( startCol + strlen_P(VTX_CHANNEL_STR) + 3, startRow, (int16_t)pgm_read_word(&vtx_frequencies[settings.s.m_vTxBand][settings.s.m_vTxChannel]), 0, "mhz" );
  
  OSD.printP( startCol, ++startRow, SET_EXIT_STR, activeVTXMenuItem );
  OSD.printP( startCol, ++startRow, BACK_STR, activeVTXMenuItem );
  
  return (void*)vTxMenu;
}


#else

#ifdef BF32_MODE
static const char TRAMP_VTX_POWERS_STR[][6] PROGMEM = { {"25mw "}, {"200mw"}, {"400mw"}, {"600mw"} };
static const char UNIFY_VTX_POWERS_STR[][6] PROGMEM = { {"25mw "}, {"200mw"}, {"500mw"}, {"800mw"} };
#endif

void* vTxMenu()
{
  #ifdef BF32_MODE
  uint8_t maxPwrIdx = 3;
  if(settings.s.m_vTxMaxPower == 200) maxPwrIdx = 1;
  #else
  int16_t maxWatt = 600;
  if(settings.s.m_vTxMaxPower > 0) maxWatt = settings.s.m_vTxMaxPower;
  else if(vTxType == 3) maxWatt = 800;
  #endif
  switch(activeVTXMenuItem)
  {
    case 0:
      #ifdef BF32_MODE
        fcSettingModeChanged[FC_VTX] |= checkCode(vTx_powerIDX, 1, 0, maxPwrIdx);
      #else
      if(vTxPowerKnobChannel == -1) fcSettingModeChanged[FC_VTX] |= checkCode(vTxLowPower, 10, 5, maxWatt);
      #endif
    break;
    case 1:
      #ifdef BF32_MODE
        fcSettingModeChanged[FC_VTX] |= checkCode(vTx_powerIDX, 1, 0, maxPwrIdx);
      #else
      if(vTxPowerKnobChannel == -1) fcSettingModeChanged[FC_VTX] |= checkCode(vTxHighPower, 25, 25, maxWatt);
      #endif
    break;
    case 2:
      fcSettingModeChanged[FC_VTX] |= checkCode(vTxBand, 1, 0, 4);
    break;
    case 3:
      fcSettingModeChanged[FC_VTX] |= checkCode(vTxChannel, 1, 0, 7);
    break;
    case 4:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        activeVTXMenuItem = 0;
        menuActive = false;
        menuWasActive = true;
        oldvTxBand = vTxBand;
        oldvTxChannel = vTxChannel;
        #ifdef BF32_MODE
        oldvTx_powerIDX = vTx_powerIDX;
        #else
        oldvTxLowPower = vTxLowPower;
        oldvTxHighPower = vTxHighPower;
        #endif
      }
    break;
    case 5:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        activeVTXMenuItem = 0;
        cleanScreen();
        vTxBand = oldvTxBand;
        vTxChannel = oldvTxChannel;
        #ifdef BF32_MODE
        vTx_powerIDX = oldvTx_powerIDX;
        #else
        vTxLowPower = oldvTxLowPower;
        vTxHighPower = oldvTxHighPower;
        #endif
        fcSettingModeChanged[FC_VTX] = false;
        return (void*)MainMenu;
      }
    break;
  }
    
  static const uint8_t VTX_MENU_ITEMS = 6;
  activeVTXMenuItem = checkMenuItem(activeVTXMenuItem, VTX_MENU_ITEMS);
  
  static const char VTX_LOW_POWER_STR[] PROGMEM =  "power disarmed:";
  static const char VTX_HIGH_POWER_STR[] PROGMEM = "power armed   :";
  static const char VTX_BAND_STR[] PROGMEM =       "band          :";
  static const char VTX_CHANNEL_STR[] PROGMEM =    "channel       :";
  static const char SET_EXIT_STR[] PROGMEM =       "set+exit";
//static const char BACK_STR[] PROGMEM =           "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(VTX_HIGH_POWER_STR)+9)/2;
  static const char VTX_TITLE_STR[] PROGMEM = "vtx menu";
  OSD.printP( settings.COLS/2 - strlen_P(VTX_TITLE_STR)/2, ++startRow, VTX_TITLE_STR );

  static const char KNOB_STR[] PROGMEM = "knob";
  OSD.printP(startCol, ++startRow, VTX_LOW_POWER_STR, activeVTXMenuItem);
  #ifdef BF32_MODE
  if(vTxType == 4) OSD.printP(startCol + strlen_P(VTX_LOW_POWER_STR) + 1, startRow, TRAMP_VTX_POWERS_STR[vTx_powerIDX]);
  else OSD.printP(startCol + strlen_P(VTX_LOW_POWER_STR) + 1, startRow, UNIFY_VTX_POWERS_STR[vTx_powerIDX]);
  #else  
  if(vTxPowerKnobChannel > -1) OSD.printP(startCol + strlen_P(VTX_LOW_POWER_STR) + 1, startRow, KNOB_STR);
  else OSD.printInt16(startCol + strlen_P(VTX_LOW_POWER_STR) + 1, startRow, vTxLowPower, 0, "mw", 1);
  #endif
  
  OSD.printP(startCol, ++startRow, VTX_HIGH_POWER_STR, activeVTXMenuItem);
  if(vTxPowerKnobChannel > -1) OSD.printP(startCol + strlen_P(VTX_HIGH_POWER_STR) + 1, startRow, KNOB_STR);
  else OSD.printInt16(startCol + strlen_P(VTX_HIGH_POWER_STR) + 1, startRow, vTxHighPower, 0, "mw", 1);

  OSD.printP( startCol, ++startRow, VTX_BAND_STR, activeVTXMenuItem );
  OSD.print( fixPStr(bandSymbols[vTxBand]) );
  
  OSD.printIntArrow( startCol, ++startRow, VTX_CHANNEL_STR, vTxChannel+1, 0, activeVTXMenuItem, "=" );
  OSD.printInt16( startCol + strlen_P(VTX_CHANNEL_STR) + 3, startRow, (int16_t)pgm_read_word(&vtx_frequencies[vTxBand][vTxChannel]), 0, "mhz" );
  
  OSD.printP( startCol, ++startRow, SET_EXIT_STR, activeVTXMenuItem );
  OSD.printP( startCol, ++startRow, BACK_STR, activeVTXMenuItem );
  
  return (void*)vTxMenu;
}
#endif


void* MainMenu()
{
  uint8_t i;
  bool crossHairChanged = false;
  int16_t temp;

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
        #ifdef BF32_MODE
        rateProfileSelected = rateProfile;
        #endif
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
#ifdef BF32_MODE
      if(code &  inputChecker.ROLL_RIGHT && vTxType > 2)
#else
      if(code &  inputChecker.ROLL_RIGHT && vTxType > 0)
#endif
      {
        cleanScreen();
        return (void*)vTxMenu;
      }
#endif
    break;
    case 5:
      #ifdef CROSSHAIR_ANGLE
      temp = settings.s.m_angleOffset;
      settingChanged |= checkCode(temp, 1, -70, 0);
      settings.s.m_angleOffset = temp;
      #else
      symbolOnOffChanged = checkCode(settings.s.m_displaySymbols, 1, 0, 1);
      settingChanged |= symbolOnOffChanged;
      #endif
    break;
    case 6:
      settingChanged |= checkCode(settings.s.m_timerMode, 1, 0, 2);        
    break;
    #ifdef CROSSHAIR
    case 7:
      crossHairChanged |= checkCode(settings.s.m_crossHair, 1, 0, 8);
      if(crossHairChanged) logoDone = true;
      settingChanged |= crossHairChanged;
    break;
    case 8:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        menuActive = false;
        menuWasActive = true;
      }
    break;
    case 9:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        menuActive = false;
        menuWasActive = true;
        settingChanged = false;          
        for(i=0; i<MAX_SETTING_MODES; i++)
        {
          fcSettingModeChanged[i] = false;
        }
        settings.ReadSettings();
        fcSettingsReceived = false;
      }
    #else
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
        for(i=0; i<MAX_SETTING_MODES; i++)
        {
          fcSettingModeChanged[i] = false;
        }
        settings.ReadSettings();
        fcSettingsReceived = false;
        #ifdef BF32_MODE
        pidProfile = oldPidProfile;
        pidProfileChanged = true;
        SendFCSettings(210); //MSP_SELECT_SETTING
        rateProfile = oldRateProfile;
        pidProfileChanged = false;
        rateProfileChanged = true;
        SendFCSettings(210); //MSP_SELECT_SETTING
        rateProfileChanged = false;
        #endif
      }
    #endif
    break;
  }

  #ifdef CROSSHAIR
  static const uint8_t MAIN_MENU_ITEMS = 10;
  #else
  static const uint8_t MAIN_MENU_ITEMS = 9;
  #endif
  activeMenuItem = checkMenuItem(activeMenuItem, MAIN_MENU_ITEMS);
  
  static const char PID_STR[] PROGMEM =             "tune";
  static const char RATES_STR[] PROGMEM =           "rates";
  static const char FILTER_STR[] PROGMEM =          "filters";
  static const char BATTERY_PAGE_STR[] PROGMEM =    "battery";
  static const char VTX_PAGE_STR[] PROGMEM =        "vtx";
  #ifdef CROSSHAIR_ANGLE
  static const char CROSS_ANGLE_STR[] PROGMEM =     "angle corr:";
  #else
  static const char SYMBOLS_SIZE_STR[] PROGMEM =    "icons     :";
  #endif
  static const char AIR_TIMER_STR[] PROGMEM =       "timer mode:";
  #ifdef CROSSHAIR
  static const char CROSSHAIR_STR[] PROGMEM =       "crosshair :";
  #endif
//static const char SAVE_EXIT_STR[] PROGMEM =       "save+exit";
  static const char CANCEL_STR[] PROGMEM =          "cancel";
  
  uint8_t startRow = 0;
  uint8_t startCol = settings.COLS/2 - (strlen_P(AIR_TIMER_STR)+5)/2;
  OSD.setCursor( settings.COLS/2 - strlen_P(KISS_OSD_VER)/2, ++startRow );
  OSD.print( fixPStr(KISS_OSD_VER) );
  static const char MAIN_TITLE_STR[] PROGMEM = "main menu";
  OSD.printP( settings.COLS/2 - strlen_P(MAIN_TITLE_STR)/2, ++startRow, MAIN_TITLE_STR );
  
  OSD.printP( startCol, ++startRow, PID_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, RATES_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, FILTER_STR, activeMenuItem);  
  OSD.printP( startCol, ++startRow, BATTERY_PAGE_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, VTX_PAGE_STR, activeMenuItem );
  #ifdef CROSSHAIR_ANGLE
  OSD.printIntArrow( startCol, ++startRow, CROSS_ANGLE_STR, settings.s.m_angleOffset, 0, activeMenuItem, "", 1 );
  #else
  OSD.printP( startCol, ++startRow, SYMBOLS_SIZE_STR, activeMenuItem );
  OSD.print( fixPStr(ON_OFF_STR[settings.s.m_displaySymbols]) );
  #endif
  OSD.printP( startCol, ++startRow, AIR_TIMER_STR, activeMenuItem );
  static const char TIMER_TYPES_STR[][6] PROGMEM = { "reset", "auto ", "race " };
  OSD.print( fixPStr(TIMER_TYPES_STR[settings.s.m_timerMode]) );
  #ifdef CROSSHAIR
  OSD.printP( startCol, ++startRow, CROSSHAIR_STR, activeMenuItem );
  static const char ON_OFF_STR_CROSS[][4] PROGMEM = { "off", "on ", "-3 ", "-2 ", "-1 ", "0  ", "+1 ", "+2 ", "+3 " };
  OSD.print( fixPStr(ON_OFF_STR_CROSS[settings.s.m_crossHair]) );
  #endif
  OSD.printP( startCol, ++startRow, SAVE_EXIT_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, CANCEL_STR, activeMenuItem );
  
  return (void*)MainMenu;
}
