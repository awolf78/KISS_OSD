static uint8_t active3MenuItem = 0;
static uint8_t activePIDMenuItem = 0;
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
static const int16_t P_STEP = 100;
static const int16_t I_STEP = 1;
static const int16_t D_STEP = 1000;
static const int16_t TPA_STEP = 50;
static const int16_t RATE_STEP = 50;

FLASH_STRING(SAVE_EXIT_STR, "save+exit");
FLASH_STRING(BACK_STR,      "back");
static char ON_OFF_STR[][4] = { "off", "on " };
FLASH_STRING(ROLL_STR,  "roll  ");
FLASH_STRING(PITCH_STR, "pitch ");
FLASH_STRING(YAW_STR,   "yaw   ");

extern void* MainMenu();
extern void* RatesMenu();

int16_t checkCode(int16_t value, int16_t STEP)
{
  if((code &  inputChecker.ROLL_LEFT) && (value-STEP) >= 0)
  {
    value -= STEP;
  }
  if((code &  inputChecker.YAW_LEFT) && (value-(STEP/10)) >= 0)
  {
    value -= STEP/10;
  }
  if((code &  inputChecker.ROLL_RIGHT) && (value+STEP) <= 32000)
  {
    value += STEP;
  }
  if((code &  inputChecker.YAW_RIGHT) && (value+(STEP/10)) <= 32000)
  {
    value += STEP/10;
  }
  return value;
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

uint8_t checkSetting(uint8_t setting, uint8_t lowLim, uint8_t upLim, boolean &changed)
{
  if((code &  inputChecker.ROLL_LEFT) && setting > lowLim)
  {
    changed = true;
    setting--;
  }
  if(code &  inputChecker.ROLL_RIGHT && setting < upLim)
  {
    changed = true;
    setting++;
  }
  return setting;
}

void* ThreeItemPlusBackMenu(uint8_t &active3MenuItem, int16_t &item1, int16_t &item2, int16_t &item3, int16_t item1_step, int16_t item2_step, int16_t item3_step, char* title, void* prevPage, void* thisPage, _FLASH_STRING *itemDescription1 = 0, _FLASH_STRING *itemDescription2 = 0, _FLASH_STRING *itemDescription3 = 0)
{
  if(active3MenuItem < 3 && ((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT)))
  {
    fcSettingChanged = true;
  }
  if(code > 0)
  {
    switch(active3MenuItem)
    {
      case 0:
        item1 = checkCode(item1, item1_step);
      break;
      case 1:
        item2 = checkCode(item2, item2_step);        
      break;
      case 2:
        item3 = checkCode(item3, item3_step);        
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
  }
  
  static const uint8_t MENU_ITEMS = 5;
  
  active3MenuItem = checkMenuItem(active3MenuItem, MENU_ITEMS);
  
  uint8_t startRow = 1;
  uint8_t startCol = COLS/2 - ((*itemDescription1).length()+6)/2;
  OSD.setCursor( COLS/2 - strlen(title)/2, ++startRow );
  OSD.print( fixStr(title) );
  
  OSD.printIntArrow( startCol, ++startRow, itemDescription1, item1, 3, 1, active3MenuItem);
  OSD.printIntArrow( startCol, ++startRow, itemDescription2, item2, 3, 1, active3MenuItem);
  OSD.printIntArrow( startCol, ++startRow, itemDescription3, item3, 3, 1, active3MenuItem);
  OSD.printFS( startCol, ++startRow, &BACK_STR, active3MenuItem);
  OSD.printFS( startCol, ++startRow, &SAVE_EXIT_STR, active3MenuItem);
  
  return thisPage;
}

FLASH_STRING(RATES_DESC_STR1, "rc rate  : "); 
FLASH_STRING(RATES_DESC_STR2, "rate     : "); 
FLASH_STRING(RATES_DESC_STR3, "rc curve : ");

void* RatesRollMenu()
{
  return ThreeItemPlusBackMenu(activeRatesRollMenuItem, rcrate_roll, rate_roll, rccurve_roll, RATE_STEP, RATE_STEP, RATE_STEP, "roll", (void*) RatesMenu, (void*) RatesRollMenu, &RATES_DESC_STR1, &RATES_DESC_STR2, &RATES_DESC_STR3);
}

void* RatesPitchMenu()
{
  return ThreeItemPlusBackMenu(activeRatesPitchMenuItem, rcrate_pitch, rate_pitch, rccurve_pitch, RATE_STEP, RATE_STEP, RATE_STEP, "pitch", (void*)RatesMenu, (void*) RatesPitchMenu, &RATES_DESC_STR1, &RATES_DESC_STR2, &RATES_DESC_STR3);
}

void* RatesYawMenu()
{
  return ThreeItemPlusBackMenu(activeRatesPitchMenuItem, rcrate_yaw, rate_yaw, rccurve_yaw, RATE_STEP, RATE_STEP, RATE_STEP, "yaw", (void*)RatesMenu, (void*) RatesYawMenu, &RATES_DESC_STR1, &RATES_DESC_STR2, &RATES_DESC_STR3);
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
  
//FLASH_STRING(ROLL_STR,  "roll  ");
//FLASH_STRING(PITCH_STR, "pitch ");
//FLASH_STRING(YAW_STR,   "yaw   ");
//FLASH_STRING(BACK_STR,  "back");
  
  uint8_t startRow = 1;
  uint8_t startCol = COLS/2 - ROLL_STR.length()/2;
  FLASH_STRING(RATES_TITLE_STR, "rates menu");
  OSD.printFS(COLS/2 - RATES_TITLE_STR.length()/2, ++startRow, &RATES_TITLE_STR);
  
  OSD.printFS( startCol, ++startRow, &ROLL_STR, activeRatesMenuItem );
  OSD.printFS( startCol, ++startRow, &PITCH_STR, activeRatesMenuItem );
  OSD.printFS( startCol, ++startRow, &YAW_STR, activeRatesMenuItem );
  OSD.printFS( startCol, ++startRow, &BACK_STR, activeRatesMenuItem );

  return (void*)RatesMenu;
}

extern void* PIDMenu();

FLASH_STRING(PID_DESC_STR1, "p : "); 
FLASH_STRING(PID_DESC_STR2, "i : "); 
FLASH_STRING(PID_DESC_STR3, "d : ");

void* PIDRollMenu()
{
  return ThreeItemPlusBackMenu(activePIDRollMenuItem,  p_roll, i_roll, d_roll, P_STEP, I_STEP, D_STEP, "roll", (void*)PIDMenu, (void*) PIDRollMenu, &PID_DESC_STR1, &PID_DESC_STR2, &PID_DESC_STR3);
}

void* PIDPitchMenu()
{
  return ThreeItemPlusBackMenu(activePIDPitchMenuItem,  p_pitch, i_pitch, d_pitch, P_STEP, I_STEP, D_STEP, "pitch", (void*)PIDMenu, (void*) PIDPitchMenu, &PID_DESC_STR1, &PID_DESC_STR2, &PID_DESC_STR3);
}

void* PIDYawMenu()
{
  return ThreeItemPlusBackMenu(activePIDYawMenuItem,  p_yaw, i_yaw, d_yaw, P_STEP, I_STEP, D_STEP, "yaw", (void*)PIDMenu, (void*) PIDYawMenu, &PID_DESC_STR1, &PID_DESC_STR2, &PID_DESC_STR3);
}

void* TPAMenu()
{
  FLASH_STRING(TPA_DESC_STR1, "tpa p : "); 
  FLASH_STRING(TPA_DESC_STR2, "tpa i : "); 
  FLASH_STRING(TPA_DESC_STR3, "tpa d : ");
  return ThreeItemPlusBackMenu(activeTPAMenuItem,  p_tpa, i_tpa, d_tpa, TPA_STEP, TPA_STEP, TPA_STEP, "tpa menu", (void*)PIDMenu, (void*) TPAMenu, &TPA_DESC_STR1, &TPA_DESC_STR2, &TPA_DESC_STR3);
}

void* PIDMenu()
{
  if(code &  inputChecker.ROLL_RIGHT || code &  inputChecker.ROLL_LEFT)
  {
    switch(activePIDMenuItem)
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
      case 4:
        lpf_frq = checkSetting(lpf_frq, 0, 6, fcSettingChanged);
      break;
      case 5:
        cleanScreen();
        activePIDMenuItem = 0;
        return (void*)MainMenu;
      break;
    }
  }

  static const uint8_t PID_MENU_ITEMS = 6;
  activePIDMenuItem = checkMenuItem(activePIDMenuItem, PID_MENU_ITEMS);
  
//FLASH_STRING(ROLL_STR,  "roll  ");
//FLASH_STRING(PITCH_STR, "pitch ");
//FLASH_STRING(YAW_STR,   "yaw   ");
  FLASH_STRING(TPA_STR,   "tpa   ");
  FLASH_STRING(LPF_STR,   "lpf  : ");
//FLASH_STRING(BACK_STR,  "back");
  
  FLASH_STRING(LPF1_STR, "off ");
  FLASH_STRING(LPF2_STR, "high    ");
  FLASH_STRING(LPF3_STR, "med high");
  FLASH_STRING(LPF4_STR, "medium  ");
  FLASH_STRING(LPF5_STR, "med low ");
  FLASH_STRING(LPF6_STR, "low     ");
  FLASH_STRING(LPF7_STR, "very low");
  static _FLASH_STRING LPF_FRQ_STR[] = { LPF1_STR, LPF2_STR, LPF3_STR, LPF4_STR, LPF5_STR, LPF6_STR, LPF7_STR };
  
  uint8_t startRow = 1;
  uint8_t startCol = COLS/2 - (LPF_STR.length()+LPF7_STR.length())/2;
  FLASH_STRING(PID_MENU_TITLE_STR, "pid menu");
  OSD.printFS(COLS/2 - PID_MENU_TITLE_STR.length()/2, ++startRow, &PID_MENU_TITLE_STR);
  
  OSD.printFS( startCol, ++startRow, &ROLL_STR, activePIDMenuItem);
  OSD.printFS( startCol, ++startRow, &PITCH_STR, activePIDMenuItem);
  OSD.printFS( startCol, ++startRow, &YAW_STR, activePIDMenuItem);
  OSD.printFS( startCol, ++startRow, &TPA_STR, activePIDMenuItem);
  OSD.printFS( startCol, ++startRow, &LPF_STR, activePIDMenuItem);
  OSD.print( fixFlashStr(&LPF_FRQ_STR[lpf_frq]) );
  OSD.printFS( startCol, ++startRow, &BACK_STR, activePIDMenuItem);
  
  return (void*)PIDMenu;
}



void* BatteryMenu()
{
  boolean changed = false;
  if((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT))
  {
    switch(activeBatteryMenuItem)
    {
      case 0:
        batterySelect = true;
      break;
      case 1:
        settings.m_batWarning = checkSetting(settings.m_batWarning, 0, 1, settingChanged);
      break;
      case 2:
        settings.m_batWarningPercent = checkSetting(settings.m_batWarningPercent, 0, 100, changed);
        settingChanged |= changed;
        if(changed)
        {
          settings.FixBatWarning();
        }
      break;
      case 3:
        menuActive = false;
        menuWasActive = true;
      break;
      case 4:
        cleanScreen();
        activeBatteryMenuItem = 0;
        return (void*)MainMenu;
      break;
    }
  }
  static const uint8_t BATTERY_MENU_ITEMS = 5;
  activeBatteryMenuItem = checkMenuItem(activeBatteryMenuItem, BATTERY_MENU_ITEMS);
  
  FLASH_STRING(SELECT_BATTERY_STR,  "select battery ");
  FLASH_STRING(BATTERY_WARNING_STR, "batt. warning: ");
  FLASH_STRING(BATTERY_PERCENT_STR, "batt. % alarm: ");
//FLASH_STRING(SAVE_EXIT_STR,       "save+exit");
//FLASH_STRING(BACK_STR,            "back");
  
  uint8_t startRow = 1;
  uint8_t startCol = COLS/2 - (BATTERY_WARNING_STR.length()+4)/2;
  FLASH_STRING(BATTERY_TITLE_STR, "battery menu");
  OSD.printFS(COLS/2 - BATTERY_TITLE_STR.length()/2, ++startRow, &BATTERY_TITLE_STR);
  
  OSD.printFS( startCol, ++startRow, &SELECT_BATTERY_STR, activeBatteryMenuItem );
  OSD.printFS( startCol, ++startRow, &BATTERY_WARNING_STR, activeBatteryMenuItem );
  OSD.print( fixStr(ON_OFF_STR[settings.m_batWarning]) );
  
  OSD.printIntArrow( startCol, ++startRow, &BATTERY_PERCENT_STR, settings.m_batWarningPercent, 0, 1, activeBatteryMenuItem, "%", true );  
  
  OSD.printFS( startCol, ++startRow, &SAVE_EXIT_STR, activeBatteryMenuItem );
  OSD.printFS( startCol, ++startRow, &BACK_STR, activeBatteryMenuItem );
  
  return (void*)BatteryMenu;
}



void* DisplayMenu()
{
  if((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT))
  {
    switch(activeDisplayMenuItem)
    {
      case 0:
        settings.m_DVchannel = checkSetting(settings.m_DVchannel, 0, 3, settingChanged);
      break;
      case 1:
        settings.m_tempUnit = checkSetting(settings.m_tempUnit, 0, 1, settingChanged);
      break;
      case 2:
        settings.m_fontSize = checkSetting(settings.m_fontSize, 0, 1, settingChanged);
      break;
      case 3:
        settings.m_displaySymbols = checkSetting(settings.m_displaySymbols, 0, 1, settingChanged);
      break;
      case 4:
        menuActive = false;
        menuWasActive = true;
      break;
      case 5:
        activeDisplayMenuItem = 0;
        cleanScreen();
        return (void*)MainMenu;
      break;
    }
  }
  static const uint8_t DISPLAY_MENU_ITEMS = 6;
  activeDisplayMenuItem = checkMenuItem(activeDisplayMenuItem, DISPLAY_MENU_ITEMS);
  
  FLASH_STRING(DV_CHANNEL_STR,      "dv channel   : ");
  FLASH_STRING(TEMP_UNIT_STR,       "temp. unit   : ");
  FLASH_STRING(FONT_SIZE_STR,       "font size    : ");
  FLASH_STRING(SYMBOLS_SIZE_STR,    "symbols      : ");
//FLASH_STRING(SAVE_EXIT_STR,       "save+exit");
//FLASH_STRING(BACK_STR,            "back");
  
  uint8_t startRow = 1;
  uint8_t startCol = COLS/2 - (DV_CHANNEL_STR.length()+6)/2;
  FLASH_STRING(DISPLAY_TITLE_STR, "display menu");
  OSD.printFS( COLS/2 - DISPLAY_TITLE_STR.length()/2, ++startRow, &DISPLAY_TITLE_STR );
  
  OSD.printFS( startCol, ++startRow, &DV_CHANNEL_STR, activeDisplayMenuItem );
  OSD.print( fixStr("aux") );
  OSD.printInt16( startCol + DV_CHANNEL_STR.length() + 4, startRow, settings.m_DVchannel+1, 0, 1 );
  
  OSD.printFS( startCol, ++startRow, &TEMP_UNIT_STR, activeDisplayMenuItem);
  static const char tempSymbols[][2] = { {0xB0,0x00} , {0xB1, 0x00}};
  OSD.print( fixStr(tempSymbols[settings.m_tempUnit]) );
  
  FLASH_STRING(NORMAL_FONT_STR, "normal");
  FLASH_STRING(LARGE_FONT_STR,  "large ");
  static _FLASH_STRING FONT_SIZES_STR[] = { NORMAL_FONT_STR, LARGE_FONT_STR };
  OSD.printFS( startCol, ++startRow, &FONT_SIZE_STR, activeDisplayMenuItem );
  OSD.print( fixFlashStr(&FONT_SIZES_STR[settings.m_fontSize]) );
  
  OSD.printFS( startCol, ++startRow, &SYMBOLS_SIZE_STR, activeDisplayMenuItem );
  OSD.print( fixStr(ON_OFF_STR[settings.m_displaySymbols]) );
  
  OSD.printFS( startCol, ++startRow, &SAVE_EXIT_STR, activeDisplayMenuItem );
  OSD.printFS( startCol, ++startRow, &BACK_STR, activeDisplayMenuItem );
  
  return (void*)DisplayMenu;
}



void* MainMenu()
{
  if(code &  inputChecker.ROLL_RIGHT)
  {
    switch(activeMenuItem)
    {
      case 0:
        if(fcSettingsReceived)
        {
          cleanScreen();
          return (void*)PIDMenu;
        }
      break;
      case 1:
        if(fcSettingsReceived)
        {
          cleanScreen();
          return (void*)RatesMenu;
        }
      break;
      case 2:
        cleanScreen();
        return (void*)DisplayMenu;
      break;
      case 3:
        cleanScreen();
        return (void*)BatteryMenu;
      break;
      case 4:
        menuActive = false;
        menuWasActive = true;
      break;
      case 5:
        menuActive = false;
        menuWasActive = true;
        settingChanged = false;
        fcSettingChanged = false;
        settings.ReadSettings();
        fcSettingsReceived = false;
      break;
    }
  }
  static const uint8_t MAIN_MENU_ITEMS = 6;
  activeMenuItem = checkMenuItem(activeMenuItem, MAIN_MENU_ITEMS);
  
  FLASH_STRING(PID_STR,             "tune");
  FLASH_STRING(RATES_STR,           "rates");
  FLASH_STRING(DISPLAY_PAGE_STR,    "display");
  FLASH_STRING(BATTERY_PAGE_STR,    "battery");
//FLASH_STRING(SAVE_EXIT_STR,       "save+exit");
  FLASH_STRING(CANCEL_STR,          "cancel");
  
  uint8_t startRow = 0;
  uint8_t startCol = COLS/2 - SAVE_EXIT_STR.length()/2;
  OSD.setCursor( COLS/2 - strlen(KISS_OSD_VER)/2, ++startRow );
  OSD.print( fixStr(KISS_OSD_VER) );
  FLASH_STRING(MAIN_TITLE_STR, "main menu");
  OSD.printFS( COLS/2 - MAIN_TITLE_STR.length()/2, ++startRow, &MAIN_TITLE_STR );
  
  OSD.printFS( startCol, ++startRow, &PID_STR, activeMenuItem );
  OSD.printFS( startCol, ++startRow, &RATES_STR, activeMenuItem );
  OSD.printFS( startCol, ++startRow, &DISPLAY_PAGE_STR, activeMenuItem );
  OSD.printFS( startCol, ++startRow, &BATTERY_PAGE_STR, activeMenuItem );
  OSD.printFS( startCol, ++startRow, &SAVE_EXIT_STR, activeMenuItem );
  OSD.printFS( startCol, ++startRow, &CANCEL_STR, activeMenuItem );
  
  return (void*)MainMenu;
}
