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
static const int16_t P_STEP = 100;
static const int16_t I_STEP = 1;
static const int16_t D_STEP = 1000;
static const int16_t TPA_STEP = 50;
static const int16_t RATE_STEP = 50;

extern void* MainMenu();
extern void* RatesMenu();

void checkArrow(uint8_t currentRow, uint8_t menuItem)
{
  if(currentRow-3 == menuItem)
  {
    OSD.print(fixChar('>'));
  }
  else
  {
    OSD.print(fixChar(' '));
  }
}

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
          cleanScreen();
          return (void*) MainMenu;
        }
    }
  }
  
  static const uint8_t MENU_ITEMS = 5;
  
  active3MenuItem = checkMenuItem(active3MenuItem, MENU_ITEMS);
  
  FLASH_STRING(BACK5_STR,    "back");
  FLASH_STRING(MAIN_RETURN_STR, "main menu");
  
  uint8_t startRow = 1;
  uint8_t startCol = COLS/2 - ((*itemDescription1).length()+6)/2;
  OSD.setCursor( COLS/2 - strlen(title)/2, ++startRow );
  OSD.print( fixStr(title) );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, active3MenuItem);
  OSD.print( fixFlashStr(itemDescription1) );
  print_int16(item1, printBuf,3,1);
  OSD.print( printBuf );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, active3MenuItem);
  OSD.print( fixFlashStr(itemDescription2) );
  print_int16(item2, printBuf,3,1);
  OSD.print( printBuf );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, active3MenuItem);
  OSD.print( fixFlashStr(itemDescription3) );
  print_int16(item3, printBuf,3,1);
  OSD.print( printBuf );
 
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, active3MenuItem);
  OSD.print( fixFlashStr(&BACK5_STR) );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, active3MenuItem);
  OSD.print( fixFlashStr(&MAIN_RETURN_STR) );
  
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
  
  FLASH_STRING(ROLL_STR,  "roll  ");
  FLASH_STRING(PITCH_STR, "pitch ");
  FLASH_STRING(YAW_STR,   "yaw   ");
  FLASH_STRING(BACK2_STR, "back  ");
  
  uint8_t startRow = 1;
  uint8_t startCol = COLS/2 - ROLL_STR.length()/2;
  const char title[] = "rates menu";
  OSD.setCursor( COLS/2 - strlen(title)/2, ++startRow );   
  OSD.print( fixStr(title) );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeRatesMenuItem);
  OSD.print( fixFlashStr(&ROLL_STR) );
 
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeRatesMenuItem);
  OSD.print( fixFlashStr(&PITCH_STR) );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeRatesMenuItem);
  OSD.print( fixFlashStr(&YAW_STR) );
 
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeRatesMenuItem);
  OSD.print( fixFlashStr(&BACK2_STR) );
 
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
        if((code &  inputChecker.ROLL_LEFT) && lpf_frq > 0)
        {
          lpf_frq--;
          fcSettingChanged = true;
        }
        if((code &  inputChecker.ROLL_RIGHT) && lpf_frq < 6)
        {
          lpf_frq++;
          fcSettingChanged = true;
        }
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
  
  FLASH_STRING(ROLL_STR,  "roll  ");
  FLASH_STRING(PITCH_STR, "pitch ");
  FLASH_STRING(YAW_STR,   "yaw   ");
  FLASH_STRING(TPA_STR,   "tpa   ");
  FLASH_STRING(LPF_STR,   "lpf  : ");
  FLASH_STRING(BACK2_STR, "back  ");
  
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
  const char title[] = "pid menu";
  OSD.setCursor( COLS/2 - strlen(title)/2, ++startRow );
  OSD.print( fixStr(title) );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activePIDMenuItem);
  OSD.print( fixFlashStr(&ROLL_STR) );
 
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activePIDMenuItem);
  OSD.print( fixFlashStr(&PITCH_STR) );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activePIDMenuItem);
  OSD.print( fixFlashStr(&YAW_STR) );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activePIDMenuItem);
  OSD.print( fixFlashStr(&TPA_STR) );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activePIDMenuItem);
  OSD.print( fixFlashStr(&LPF_STR) );
  OSD.print( fixFlashStr(&LPF_FRQ_STR[lpf_frq]) );
 
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activePIDMenuItem);
  OSD.print( fixFlashStr(&BACK2_STR) );
 
  return (void*)PIDMenu;
}




void* MainMenu()
{
  if((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT))
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
        batterySelect = true;
      break;
      case 3:
        if((code &  inputChecker.ROLL_LEFT) && settings.m_batWarning == 1)
        {
          settings.m_batWarning = 0;
          settingChanged = true;
        }
        if(code &  inputChecker.ROLL_RIGHT && settings.m_batWarning == 0)
        {
          settings.m_batWarning = 1;
          settingChanged = true;
        }
      break;
      case 4:
        if((code &  inputChecker.ROLL_LEFT) && settings.m_batWarningPercent > 0)
        {
          settings.m_batWarningPercent--;
          settings.FixBatWarning();
          settingChanged = true;
        }
        if((code &  inputChecker.ROLL_RIGHT) && settings.m_batWarningPercent < 100)
        {
          settings.m_batWarningPercent++;
          settings.FixBatWarning();
          settingChanged = true;
        }
      break;
      case 5:
        if((code &  inputChecker.ROLL_LEFT) && settings.m_DVchannel > 0)
        {
          settings.m_DVchannel--;
          settingChanged = true;
        }
        if((code &  inputChecker.ROLL_RIGHT) && settings.m_DVchannel < 3)
        {
          settings.m_DVchannel++;
          settingChanged = true;
        }
      break;
      case 6:
        if((code &  inputChecker.ROLL_LEFT) && settings.m_tempUnit == 1)
        {
          settings.m_tempUnit = 0;
          settingChanged = true;
        }
        if((code &  inputChecker.ROLL_RIGHT) && settings.m_tempUnit == 0)
        {
          settings.m_tempUnit = 1;
          settingChanged = true;
        }
      break;
      case 7:
        menuActive = false;
        menuWasActive = true;
      break;
      case 8:
        menuActive = false;
        menuWasActive = true;
        settingChanged = false;
        fcSettingChanged = false;
        settings.ReadSettings();
        fcSettingsReceived = false;
      break;
    }
  }
  static const uint8_t MAIN_MENU_ITEMS = 9;
  activeMenuItem = checkMenuItem(activeMenuItem, MAIN_MENU_ITEMS);
  
  FLASH_STRING(PID_STR,             "tune");
  FLASH_STRING(RATES_STR,           "rates");
  FLASH_STRING(SELECT_BATTERY_STR,  "select battery ");
  FLASH_STRING(BATTERY_WARNING_STR, "batt. warning: ");
  FLASH_STRING(BATTERY_PERCENT_STR, "batt. % alarm: ");
  FLASH_STRING(DV_CHANNEL_STR,      "dv channel   : ");
  FLASH_STRING(TEMP_UNIT_STR,       "temp. unit   : ");
  FLASH_STRING(SAVE_EXIT_STR,       "save+exit");
  FLASH_STRING(CANCEL_STR,          "cancel");
  static char ON_OFF_STR[][4] = { "off", "on " };
  
  uint8_t startRow = 0;
  uint8_t startCol = COLS/2 - (BATTERY_WARNING_STR.length()+4)/2;
  OSD.setCursor( COLS/2 - strlen(KISS_OSD_VER)/2, ++startRow );
  OSD.print( fixStr(KISS_OSD_VER) );
  const char title[] = "main menu";
  OSD.setCursor( COLS/2 - strlen(title)/2, ++startRow );
  OSD.print( fixStr(title) );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeMenuItem);
  OSD.print( fixFlashStr(&PID_STR) );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeMenuItem);
  OSD.print( fixFlashStr(&RATES_STR) );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeMenuItem);
  OSD.print( fixFlashStr(&SELECT_BATTERY_STR) );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeMenuItem);
  OSD.print( fixFlashStr(&BATTERY_WARNING_STR) );
  OSD.print( fixStr(ON_OFF_STR[settings.m_batWarning]) );
 
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeMenuItem);
  OSD.print( fixFlashStr(&BATTERY_PERCENT_STR) );
  print_int16(settings.m_batWarningPercent, printBuf,0,1);
  OSD.print( printBuf );
  OSD.print( fixChar('%') );
  OSD.print( fixFlashStr(&TWO_BLANKS) );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeMenuItem);
  OSD.print( fixFlashStr(&DV_CHANNEL_STR) );
  OSD.print( fixStr("aux") );
  OSD.print( (char)(settings.m_DVchannel+1+0x06) );
 
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeMenuItem);
  OSD.print( fixFlashStr(&TEMP_UNIT_STR) );
  static const char tempSymbols[][2] = { {0x30,0x00} , {0x31, 0x00}};
  OSD.print( tempSymbols[settings.m_tempUnit] );
 
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeMenuItem);
  OSD.print( fixFlashStr(&SAVE_EXIT_STR) ); 
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeMenuItem);
  OSD.print( fixFlashStr(&CANCEL_STR) );
  
  return (void*)MainMenu;
}
