static uint8_t activeBatteryMenuItem = 0;
static uint8_t activeDisplayMenuItem = 0;
static uint8_t activeVTXMenuItem = 0;
static uint8_t activeOrderMenuItem = 0;
static uint8_t activeResetMenuItem = 0;
static bool selectedOrder = false;
static uint8_t activeOrderMenuSelectedItem = 199;
static uint8_t confirmIndex = 0;

static const char SAVE_EXIT_STR[] PROGMEM = "save+exit";
static const char BACK_STR[] PROGMEM =      "back";
static char ON_OFF_STR[][4] = { "off", "on " };

extern void* MainMenu();

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

void* ChangeOrder()
{
  uint8_t i;
  uint8_t reverseLUT[CSettings::DISPLAY_DV_SIZE];
  for(i=0; i<CSettings::DISPLAY_DV_SIZE; i++)
  {
    reverseLUT[settings.m_DISPLAY_DV[i]] = i;
  }

  if(activeOrderMenuItem == CSettings::DISPLAY_DV_SIZE && code &  inputChecker.ROLL_RIGHT)
  {
    cleanScreen();
    activeOrderMenuItem = 0;
    return (void*)MainMenu;
  }
  else
  {
    if(code &  inputChecker.ROLL_RIGHT && !selectedOrder)
    {
      selectedOrder = true;
      activeOrderMenuSelectedItem = activeOrderMenuItem;
      activeOrderMenuItem = 199;
    }
    if(code & inputChecker.ROLL_LEFT && selectedOrder)
    {
      selectedOrder = false;
      activeOrderMenuItem = activeOrderMenuSelectedItem;
      activeOrderMenuSelectedItem = 199;
    }
    if(code & inputChecker.PITCH_UP && selectedOrder)
    {
      if(settings.m_DISPLAY_DV[reverseLUT[activeOrderMenuSelectedItem]] > 0)
      {
        settings.m_DISPLAY_DV[reverseLUT[activeOrderMenuSelectedItem-1]]++;
        settings.m_DISPLAY_DV[reverseLUT[activeOrderMenuSelectedItem]]--;
        activeOrderMenuSelectedItem--;
        settings.SetupPPMs(DV_PPMs);
        settingChanged = true;
      }
    }
    if(code & inputChecker.PITCH_DOWN && selectedOrder)
    {
      if(settings.m_DISPLAY_DV[reverseLUT[activeOrderMenuSelectedItem]] < CSettings::DISPLAY_DV_SIZE)
      {
        settings.m_DISPLAY_DV[reverseLUT[activeOrderMenuSelectedItem+1]]--;
        settings.m_DISPLAY_DV[reverseLUT[activeOrderMenuSelectedItem]]++;
        activeOrderMenuSelectedItem++;
        settings.SetupPPMs(DV_PPMs);
        settingChanged = true;
      }
    }
  }

  static const uint8_t ORDER_MENU_ITEMS = CSettings::DISPLAY_DV_SIZE+1;
  activeOrderMenuItem = checkMenuItem(activeOrderMenuItem, ORDER_MENU_ITEMS);
  
  static const char TIMER_ORDER_STR[] PROGMEM =         "timer      ";
  static const char VOLTAGE_ORDER_STR[] PROGMEM =       "voltage    ";
  static const char THROTTLE_ORDER_STR[] PROGMEM =      "throttle   ";
  static const char MAH_ORDER_STR[] PROGMEM =           "mah        ";
  static const char NICKNAME_ORDER_STR[] PROGMEM =      "nickname   ";
  static const char AMPS_ORDER_STR[] PROGMEM =          "amps       ";
  static const char ESC_TEMP_ORDER_STR[] PROGMEM =      "esc temp   ";
  static const char ESC_VOLTAGE_ORDER_STR[] PROGMEM =   "esc voltage";
  static const char ESC_RPM_ORDER_STR[] PROGMEM =       "esc rpm    ";
//static const char BACK_STR[] PROGMEM =                "back";

  char *orderItems[CSettings::DISPLAY_DV_SIZE];
  orderItems[settings.m_DISPLAY_DV[DISPLAY_NICKNAME]] = NICKNAME_ORDER_STR;
  orderItems[settings.m_DISPLAY_DV[DISPLAY_TIMER]] = TIMER_ORDER_STR;
  orderItems[settings.m_DISPLAY_DV[DISPLAY_RC_THROTTLE]] = THROTTLE_ORDER_STR;
  orderItems[settings.m_DISPLAY_DV[DISPLAY_COMB_CURRENT]] = AMPS_ORDER_STR;
  orderItems[settings.m_DISPLAY_DV[DISPLAY_LIPO_VOLTAGE]] = VOLTAGE_ORDER_STR;
  orderItems[settings.m_DISPLAY_DV[DISPLAY_MA_CONSUMPTION]] = MAH_ORDER_STR;
  orderItems[settings.m_DISPLAY_DV[DISPLAY_ESC_KRPM]] = ESC_RPM_ORDER_STR;
  orderItems[settings.m_DISPLAY_DV[DISPLAY_ESC_CURRENT]] = ESC_VOLTAGE_ORDER_STR;
  orderItems[settings.m_DISPLAY_DV[DISPLAY_ESC_TEMPERATURE]] = ESC_TEMP_ORDER_STR;
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - strlen_P(ESC_VOLTAGE_ORDER_STR)/2;
  static const char ORDER_TITLE_STR[] PROGMEM = "order menu";
  OSD.printP(settings.COLS/2 - strlen_P(ORDER_TITLE_STR)/2, ++startRow, ORDER_TITLE_STR);
  for(i=0; i<CSettings::DISPLAY_DV_SIZE; i++)
  {
    if(i == activeOrderMenuSelectedItem) OSD.blink1sec(); 
    OSD.printP( startCol, ++startRow, orderItems[i], activeOrderMenuItem );
    OSD.noBlink();
  }  
  OSD.printP( startCol, ++startRow, BACK_STR, activeOrderMenuItem );
  
  return (void*)ChangeOrder;
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
        cleanScreen();
        activeBatteryMenuItem = 0;
        settings.UpdateMaxWatt(settings.m_maxWatts);
        return (void*)MainMenu;
      }
    break;
  }
  static const uint8_t BATTERY_MENU_ITEMS = 8;
  activeBatteryMenuItem = checkMenuItem(activeBatteryMenuItem, BATTERY_MENU_ITEMS);
  
  static const char SELECT_BATTERY_STR[]  PROGMEM = "select battery ";
  static const char BATTERY_WARNING_STR[] PROGMEM = "batt. warning: ";
  static const char BATTERY_PERCENT_STR[] PROGMEM = "batt. % alarm: ";
  static const char VOLTAGE_WARN_STR[] PROGMEM =    "volt. warning: ";
  static const char MIN_VOLT_STR[] PROGMEM =        "min voltage  : ";
  static const char VOLT_CORRECT_STR[] PROGMEM =    "voltage corr : ";
  static const char MAX_BEER_WATT_STR[] PROGMEM =   "wattmeter max: ";
//static const char BACK_STR[] PROGMEM =            "back");
  
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
  
  OSD.printP( startCol, ++startRow, BACK_STR, activeBatteryMenuItem );
  
  return (void*)BatteryMenu;
}



void* DisplayMenu()
{
  if((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT))
  {
    bool gogglechanged, symbolChanged;
    switch(activeDisplayMenuItem)
    {
      case 0:
        settingChanged |= checkCode(settings.m_DVchannel, 1, 0, 3);
      break;
      case 1:
        settingChanged |= checkCode(settings.m_tempUnit, 1, 0, 1);
      break;
      case 2:
        settingChanged |= checkCode(settings.m_fontSize, 1, 0, 1);
      break;
      case 3:
        symbolChanged = checkCode(settings.m_displaySymbols, 1, 0, 1);
        settingChanged |= symbolChanged;
        if(symbolChanged) 
        {
          symbolOnOffChanged = true;
        }
      break;
      case 4:
        gogglechanged = checkCode(settings.m_goggle, 1, 0, 1);
        if(gogglechanged) correctItemsOnce = false;
        settingChanged |= gogglechanged;
      break;
      case 5:
        settingChanged |= checkCode(settings.m_wattMeter, 1, 0, 2);
      break;
      case 6:
        settingChanged |= checkCode(settings.m_props, 1, 0, 1);
      break;
      /*case 6:
        settingChanged |= checkCode(settings.m_Moustache, 1, 0, 1);
      break;*/
      case 7:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          activeDisplayMenuItem = 0;
          cleanScreen();
          return (void*)MainMenu;
        }
      break;
    }
  }
  static const uint8_t DISPLAY_MENU_ITEMS = 8;
  activeDisplayMenuItem = checkMenuItem(activeDisplayMenuItem, DISPLAY_MENU_ITEMS);
  
  static const char DV_CHANNEL_STR[] PROGMEM =      "dv channel : ";
  static const char TEMP_UNIT_STR[] PROGMEM =       "temp. unit : ";
  static const char FONT_SIZE_STR[] PROGMEM =       "font size  : ";
  static const char SYMBOLS_SIZE_STR[] PROGMEM =    "symbols    : ";
  static const char GOGGLE_STR[] PROGMEM =          "goggle     : ";
  static const char BEERMUG_STR[] PROGMEM =         "watt meter : ";
  static const char PROPS_STR[] PROGMEM =           "props      : ";
//static const char MUSTACHE_STR[] PROGMEM =        "mustache   : ";
//static const char BACK_STR[] PROGMEM =            "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(DV_CHANNEL_STR)+6)/2;
  static const char DISPLAY_TITLE_STR[] PROGMEM = "display menu";
  OSD.printP( settings.COLS/2 - strlen_P(DISPLAY_TITLE_STR)/2, ++startRow, DISPLAY_TITLE_STR );
  
  OSD.printP( startCol, ++startRow, DV_CHANNEL_STR, activeDisplayMenuItem );
  OSD.print( fixStr("aux") );
  uint8_t tempCol = startCol + strlen_P(DV_CHANNEL_STR) + 4;
  OSD.printInt16(tempCol, startRow, settings.m_DVchannel+1, 0, 1 );
  
  OSD.printP( startCol, ++startRow, TEMP_UNIT_STR, activeDisplayMenuItem);
  static const char tempSymbols[][2] = { {0xB0,0x00} , {0xB1, 0x00}};
  OSD.print( fixStr(tempSymbols[settings.m_tempUnit]) );
  
  static const char NORMAL_FONT_STR[] PROGMEM = "normal";
  static const char LARGE_FONT_STR[] PROGMEM =  "large ";
  static const char* FONT_SIZES_STR[] = { NORMAL_FONT_STR, LARGE_FONT_STR };
  OSD.printP( startCol, ++startRow, FONT_SIZE_STR, activeDisplayMenuItem );
  OSD.print( fixPStr(FONT_SIZES_STR[settings.m_fontSize]) );
  
  OSD.printP( startCol, ++startRow, SYMBOLS_SIZE_STR, activeDisplayMenuItem );
  OSD.print( fixStr(ON_OFF_STR[settings.m_displaySymbols]) );

  static const char FATSHARK_STR[] PROGMEM =   "fshark";
  static const char HEADPLAY_STR[] PROGMEM =   "hplay ";
  static const char* GOGGLES_STR[] = { FATSHARK_STR, HEADPLAY_STR };
  OSD.printP( startCol, ++startRow, GOGGLE_STR, activeDisplayMenuItem );
  OSD.print( fixPStr(GOGGLES_STR[settings.m_goggle]) );

  OSD.printP( startCol, ++startRow, BEERMUG_STR, activeDisplayMenuItem );
  static char ON_OFF_BEER_STR[][8] = { "off    ", "on     ", "beermug" };
  OSD.print( fixStr(ON_OFF_BEER_STR[settings.m_wattMeter]) );

  OSD.printP( startCol, ++startRow, PROPS_STR, activeDisplayMenuItem );
  OSD.print( fixStr(ON_OFF_STR[settings.m_props]) );

  /*OSD.printP( startCol, ++startRow, MUSTACHE_STR, activeDisplayMenuItem );
  OSD.print( fixStr(ON_OFF_STR[settings.m_Moustache]) );*/
  
  OSD.printP( startCol, ++startRow, BACK_STR, activeDisplayMenuItem );
  
  return (void*)DisplayMenu;
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
#endif

void* ResetMenu()
{
  if(code &  inputChecker.ROLL_RIGHT)
  {
    switch(activeResetMenuItem)
    {
      case 0:
        confirmIndex++;
        if(confirmIndex > 1)
        {
          settings.ReadSettings();
          confirmIndex = 0;
        }
      break;
      case 1:
        confirmIndex++;
        if(confirmIndex > 1)
        {
          settings.LoadDefaults();
          confirmIndex = 0;
        }
      break;
      case 2:
        cleanScreen();
        return (void*)MainMenu;
      break;
    }
  }
  static const uint8_t RESET_MENU_ITEMS = 3;
  uint8_t oldItem = activeResetMenuItem;
  activeResetMenuItem = checkMenuItem(activeResetMenuItem, RESET_MENU_ITEMS);
  if(oldItem != activeResetMenuItem)
  {
    confirmIndex = 0; 
  }
  
  static const char RESTORE_SAVE_STR[] PROGMEM =   "restore last save";
  static const char RESTORE_DEF_STR[] PROGMEM =    "restore defaults ";
  static const char CONFIRM_RESET_STR[] PROGMEM =  "confirm          ";
//static const char BACK_STR[] PROGMEM =           "back");
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(RESTORE_SAVE_STR)+6)/2;
  static const char RESET_TITLE_STR[] PROGMEM = "reset menu";
  OSD.printP( settings.COLS/2 - strlen_P(RESET_TITLE_STR)/2, ++startRow, RESET_TITLE_STR );

  if(activeResetMenuItem == 0 && confirmIndex == 1)
  {
    OSD.printP( startCol, ++startRow, CONFIRM_RESET_STR, activeResetMenuItem );    
  }
  else
  {
    OSD.printP( startCol, ++startRow, RESTORE_SAVE_STR, activeResetMenuItem );
  }

  if(activeResetMenuItem == 1 && confirmIndex == 1)
  {
    OSD.printP( startCol, ++startRow, CONFIRM_RESET_STR, activeResetMenuItem );    
  }
  else
  {
    OSD.printP( startCol, ++startRow, RESTORE_DEF_STR, activeResetMenuItem );
  }
  
  OSD.printP( startCol, ++startRow, BACK_STR, activeResetMenuItem );
  
  return (void*)ResetMenu;
}

void* MainMenu()
{
  static const char FONT_UPDATE_STR[] PROGMEM =   "font will be updated";
  static const char POWER_WARNING_STR[] PROGMEM = "do not power off!!!";
  if(code &  inputChecker.ROLL_RIGHT)
  {
    switch(activeMenuItem)
    {
      case 0:
        cleanScreen();
        OSD.printP(settings.COLS/2 - strlen_P(FONT_UPDATE_STR)/2, settings.ROWS/2-1, FONT_UPDATE_STR);
        OSD.printP(settings.COLS/2 - strlen_P(POWER_WARNING_STR)/2, settings.ROWS/2, POWER_WARNING_STR);
        updateFont = true;
        return (void*)MainMenu;
      break;
      case 1:
        cleanScreen();
        return (void*)DisplayMenu;
      break;
      case 2:
        cleanScreen();
        moveItems = true;
        settings.SetupPPMs(DV_PPMs, true);
        menuActive = false;
        moveSelected = 0;
        OSD_ITEM_BLINK[moveSelected] = true;
        return (void*)MainMenu; 
      break;
      case 3:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        menuActive = false;
        shiftOSDactive = true;
        return (void*)MainMenu;
      }
      break;
      case 4:
        cleanScreen();
        return (void*)ChangeOrder;
      break;
      case 5:
        cleanScreen();
        setupNickname = true;
        charSelected = 0;
        charIndex = findCharPos(settings.m_nickname[charSelected]);
        return (void*)MainMenu; 
      break;
      case 6:
        cleanScreen();
        return (void*)BatteryMenu;
      break;
      case 7:
#ifdef IMPULSERC_VTX
        cleanScreen();
        return (void*)vTxMenu;
#endif
      break;
      case 8:
        cleanScreen();
        saveSettings = true;
        return (void*)MainMenu;        
      break;
      case 9:
        cleanScreen();
        return (void*)ResetMenu;
      break;
    }
  }
  static const uint8_t MAIN_MENU_ITEMS = 10;
  activeMenuItem = checkMenuItem(activeMenuItem, MAIN_MENU_ITEMS);
  
  static const char UPDATE_FONT_STR[] PROGMEM =     "update font";
  static const char DISPLAY_PAGE_STR[] PROGMEM =    "display";
  static const char MOVE_ITEMS_STR[] PROGMEM =      "move items";
  static const char CENTER_OSD_STR[] PROGMEM =      "center osd";
  static const char CHANGE_ORDER_STR[] PROGMEM =    "change order";
  static const char NICKNAME_STR[] PROGMEM =        "nickname";
  static const char BATTERY_PAGE_STR[] PROGMEM =    "battery";
  static const char VTX_PAGE_STR[] PROGMEM =        "vtx";
  static const char SAVE_STR[] PROGMEM =            "save";
  static const char RESET_STR[] PROGMEM =           "reset";
  
  uint8_t startRow = 0;
  uint8_t startCol = settings.COLS/2 - strlen_P(CHANGE_ORDER_STR)/2;
  OSD.setCursor( settings.COLS/2 - strlen(KISS_OSD_VER)/2, ++startRow );
  OSD.print( fixStr(KISS_OSD_VER) );
  static const char MAIN_TITLE_STR[] PROGMEM = "main menu";
  OSD.printP( settings.COLS/2 - strlen_P(MAIN_TITLE_STR)/2, ++startRow, MAIN_TITLE_STR );
  
  OSD.printP( startCol, ++startRow, UPDATE_FONT_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, DISPLAY_PAGE_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, MOVE_ITEMS_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, CENTER_OSD_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, CHANGE_ORDER_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, NICKNAME_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, BATTERY_PAGE_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, VTX_PAGE_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, SAVE_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, RESET_STR, activeMenuItem );
  
  return (void*)MainMenu;
}
