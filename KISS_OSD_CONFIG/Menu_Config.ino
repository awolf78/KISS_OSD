static uint8_t activeBatteryMenuItem = 0;
static uint8_t activeDisplayMenuItem = 0;
static uint8_t activeVTXMenuItem = 0;
static uint8_t activeOrderMenuItem = 0;
static uint8_t activeResetMenuItem = 0;
static uint8_t activeIconsMenuItem = 0;
static uint8_t activeOSDItemsMenuItem = 0;
static uint8_t activeRSSIMenuItem = 0;
static uint8_t activeMiscMenuItem = 0;
static bool selectedOrder = false;
static uint8_t activeOrderMenuSelectedItem = 199;
static uint8_t confirmIndex = 0;

static const char SAVE_EXIT_STR[] PROGMEM = "save+exit";
static const char AUX_STR[] PROGMEM =       "aux";
static const char BACK_STR[] PROGMEM =      "back";
static const char ON_OFF_STR[][4] PROGMEM = { "off", "on " };

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

boolean checkCode(uint16_t &value, int16_t STEP, int16_t minVal = 0, int16_t maxVal = 32000)
{
  int16_t value2 = (int16_t)value;
  boolean changed = checkCode(value2, STEP, minVal, maxVal);
  value = (uint16_t)value2;
  return changed;
}

boolean checkCode(uint8_t &value, int16_t STEP, int16_t minVal = 0, int16_t maxVal = 32000)
{
  int16_t tempValue = (int16_t)value;
  boolean changed = checkCode(tempValue, STEP, minVal, maxVal);
  value = (uint8_t) tempValue;
  return changed;
}

boolean checkCode(int8_t &value, int16_t STEP, int16_t minVal = 0, int16_t maxVal = 32000)
{
  int16_t tempValue = (int16_t)value;
  boolean changed = checkCode(tempValue, STEP, minVal, maxVal);
  value = (int8_t) tempValue;
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

static const char DISPLAY_OSD_ITEMS_STR[][12] PROGMEM = { "callsign   ",
                                                        "timer      ",
                                                        "throttle   ",
                                                        "amps       ",
                                                        "voltage    ",
                                                        "mah        ",
                                                        "esc rpm    ",
                                                        "esc amps   ",
                                                        "esc temp   ",
                                                        "rssi       " };
static const char WATT_ORDER_STR[] PROGMEM =            "wattmeter  ";

void* ChangeOrder()
{
  OSD.topOffset = 2;
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
    OSD.topOffset = 3;
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
  
  uint8_t startRow = 0;
  uint8_t startCol = settings.COLS/2 - strlen_P(DISPLAY_OSD_ITEMS_STR[0])/2;
  static const char ORDER_TITLE_STR[] PROGMEM = "order menu";
  OSD.printP(settings.COLS/2 - strlen_P(ORDER_TITLE_STR)/2, ++startRow, ORDER_TITLE_STR);
  for(i=0; i<CSettings::DISPLAY_DV_SIZE; i++)
  {
    if(i == activeOrderMenuSelectedItem) OSD.blink1sec();
    if(settings.s.m_wattMeter && reverseLUT[i] == DISPLAY_COMB_CURRENT) OSD.printP( startCol, ++startRow, WATT_ORDER_STR, activeOrderMenuItem );
    else OSD.printP( startCol, ++startRow, DISPLAY_OSD_ITEMS_STR[reverseLUT[i]], activeOrderMenuItem );
  }  
  OSD.printP( startCol, ++startRow, BACK_STR, activeOrderMenuItem );
  
  return (void*)ChangeOrder;
}

void* SetOSDItems()
{
  OSD.topOffset = 2;
  if((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT) && activeOSDItemsMenuItem < CSettings::DISPLAY_DV_SIZE)
  {
    settingChanged |= checkCode(settings.m_DISPLAY_DV[activeOSDItemsMenuItem], 1, 0, 1);        
  }

  if((code &  inputChecker.ROLL_RIGHT) && activeOSDItemsMenuItem == CSettings::DISPLAY_DV_SIZE)
  {
    activeOSDItemsMenuItem = 0;
    cleanScreen();
    OSD.topOffset = 3;
    settings.SetupPPMs(DV_PPMs);
    return (void*)MainMenu;
  }

  static const uint8_t SET_OSD_MENU_ITEMS = CSettings::DISPLAY_DV_SIZE+1;
  activeOSDItemsMenuItem = checkMenuItem(activeOSDItemsMenuItem, SET_OSD_MENU_ITEMS);
  
  uint8_t startRow = 0;
  uint8_t startCol = settings.COLS/2 - strlen_P(DISPLAY_OSD_ITEMS_STR[0])/2;
  static const char SET_OSD_ITEMS_TITLE_STR[] PROGMEM = "set items menu";
  OSD.printP(settings.COLS/2 - strlen_P(SET_OSD_ITEMS_TITLE_STR)/2, ++startRow, SET_OSD_ITEMS_TITLE_STR);
  for(uint8_t i=0; i<CSettings::DISPLAY_DV_SIZE; i++)
  {
    OSD.printP( startCol, ++startRow, DISPLAY_OSD_ITEMS_STR[i], activeOSDItemsMenuItem );
    OSD.print(fixStr(": "));
    OSD.print( fixPStr(ON_OFF_STR[settings.m_DISPLAY_DV[i]]) );
  }  
  OSD.printP( startCol, ++startRow, BACK_STR, activeOSDItemsMenuItem );
  
  return (void*)SetOSDItems;
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
      settingChanged |= checkCode(settings.s.m_voltCorrect, 1, -10, 10);
    break;
    case 6:
      checkCode(settings.s.m_maxWatts, (int16_t)1000, (int16_t)1000, (int16_t)30000);
    break;
    case 7:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        activeBatteryMenuItem = 0;
        settings.UpdateMaxWatt(settings.s.m_maxWatts);
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
  OSD.print( fixPStr(ON_OFF_STR[settings.s.m_batWarning]) );
  
  OSD.printIntArrow( startCol, ++startRow, BATTERY_PERCENT_STR, settings.s.m_batWarningPercent, 0, 1, activeBatteryMenuItem, "%", true );

  OSD.printP( startCol, ++startRow, VOLTAGE_WARN_STR, activeBatteryMenuItem );  
  OSD.print( fixPStr(ON_OFF_STR[settings.s.m_voltWarning]) );

  OSD.printIntArrow( startCol, ++startRow, MIN_VOLT_STR, settings.s.m_minVolts, 1, 1, activeBatteryMenuItem, "v", 1 );

  OSD.printIntArrow( startCol, ++startRow, VOLT_CORRECT_STR, settings.s.m_voltCorrect, 1, 1, activeBatteryMenuItem, "v", 1 );

  OSD.printIntArrow( startCol, ++startRow, MAX_BEER_WATT_STR, settings.s.m_maxWatts/10, 0, 1, activeBatteryMenuItem, "w", 1 );
  
  OSD.printP( startCol, ++startRow, BACK_STR, activeBatteryMenuItem );
  
  return (void*)BatteryMenu;
}



void* IconsMenu()
{
  if((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT))
  {
    uint8_t oldVal;
    uint8_t temp1;
    switch(activeIconsMenuItem)
    {
      case 0:
        settings.m_oldDisplaySymbols = settings.s.m_displaySymbols;
        settingChanged |= checkCode(settings.s.m_displaySymbols, 1, 0, 1);
        settings.fixColBorders();        
      break;
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
        oldVal = settings.m_IconSettings[activeIconsMenuItem-1];
        settingChanged |= checkCode(settings.m_IconSettings[activeIconsMenuItem-1], 1, 0, 1);
        if((activeIconsMenuItem-1) == ESC_ICON && oldVal != settings.m_IconSettings[ESC_ICON] && settings.s.m_displaySymbols == 1) 
        {
          temp1 = settings.m_IconSettings[PROPS_ICON];
          settings.m_IconSettings[PROPS_ICON] = 0;
          settings.m_oldDisplaySymbols = 2;
          settings.s.m_displaySymbols = settings.m_IconSettings[ESC_ICON];
          settings.m_IconSettings[ESC_ICON] = 1;
          settings.fixColBorders();
          settings.m_IconSettings[ESC_ICON] = settings.s.m_displaySymbols;
          settings.m_IconSettings[PROPS_ICON] = temp1;
          settings.s.m_displaySymbols = 1;
          settings.m_oldDisplaySymbols = 1;
          
        }
        if((activeIconsMenuItem-1) == (uint8_t)PROPS_ICON && oldVal != settings.m_IconSettings[PROPS_ICON] && settings.s.m_displaySymbols == 1) 
        {
          temp1 = settings.m_IconSettings[ESC_ICON];
          settings.m_IconSettings[ESC_ICON] = 0;
          settings.m_oldDisplaySymbols = 2;
          settings.s.m_displaySymbols = settings.m_IconSettings[PROPS_ICON];
          settings.m_IconSettings[PROPS_ICON] = 1;
          settings.fixColBorders();
          settings.m_IconSettings[PROPS_ICON] = settings.s.m_displaySymbols;
          settings.m_IconSettings[ESC_ICON] = temp1;
          settings.s.m_displaySymbols = 1;
          settings.m_oldDisplaySymbols = 1;
        }
      break;      
      case 8:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          activeIconsMenuItem = 0;
          cleanScreen();
          return (void*)DisplayMenu;
        }
      break;
    }    
  }
  static const uint8_t ICONS_MENU_ITEMS = 9;
  activeIconsMenuItem = checkMenuItem(activeIconsMenuItem, ICONS_MENU_ITEMS);

  static const char SYMBOLS_SIZE_STR[] PROGMEM =    "icons      : ";
  static const char ALL_ICONS_STR[][14] PROGMEM = { "props icon : ",
                                                    "esc icon   : ",
                                                    "watt icon  : ",
                                                    "mah icon   : ",
                                                    "rssi icon  : ",
                                                    "timer icon : ",
                                                    "kiss logo  : " };
//static const char BACK_STR[] PROGMEM =            "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(SYMBOLS_SIZE_STR)+3)/2;
  static const char ICONS_TITLE_STR[] PROGMEM = "icons menu";
  OSD.printP( settings.COLS/2 - strlen_P(ICONS_TITLE_STR)/2, ++startRow, ICONS_TITLE_STR);

  OSD.printP( startCol, ++startRow, SYMBOLS_SIZE_STR, activeIconsMenuItem );
  OSD.print( fixPStr(ON_OFF_STR[settings.s.m_displaySymbols]) );
  
  uint8_t i;
  for(i=0; i<settings.ICON_SETTINGS_SIZE; i++)
  {
    OSD.printP( startCol, ++startRow, ALL_ICONS_STR[i], activeIconsMenuItem );
    OSD.print( fixPStr(ON_OFF_STR[settings.m_IconSettings[i]]) );
  }
  
  OSD.printP( startCol, ++startRow, BACK_STR, activeIconsMenuItem );
  
  return (void*)IconsMenu;
}


void* RSSIMenu()
{
  #ifdef BF32_MODE
  uint8_t maxRSSIChannel = 4;
  #else
  uint8_t maxRSSIChannel = 3;
  #endif
  switch(activeRSSIMenuItem)
  {
    case 0:
      settingChanged |= checkCode(settings.s.m_RSSIchannel, 1, -1, maxRSSIChannel);
    break;
    case 1:
      settingChanged |= checkCode(settings.s.m_RSSImin, 10, 0, 2000);
    break;
    case 2:
      settingChanged |= checkCode(settings.s.m_RSSImax, 10, 0, 2000);
    break;
    case 3:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        activeRSSIMenuItem = 0;          
        cleanScreen();
        return (void*)DisplayMenu;
      }
    break;
  }
  static const uint8_t RSSI_MENU_ITEMS = 4;
  activeRSSIMenuItem = checkMenuItem(activeRSSIMenuItem, RSSI_MENU_ITEMS);
  
  static const char RSSI_CHANNEL_STR[] PROGMEM =    "rssi channel: ";
  static const char RSSI_MIN_PPM_STR[] PROGMEM =    "rssi min ppm: ";
  static const char RSSI_MAX_PPM_STR[] PROGMEM =    "rssi max ppm: ";
//static const char BACK_STR[] PROGMEM =            "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(RSSI_CHANNEL_STR)+9)/2;
  static const char RSSI_TITLE_STR[] PROGMEM = "rssi config menu";
  OSD.printP( settings.COLS/2 - strlen_P(RSSI_TITLE_STR)/2, ++startRow, RSSI_TITLE_STR );
  
  OSD.printP( startCol, ++startRow, RSSI_CHANNEL_STR, activeRSSIMenuItem );
  if(settings.s.m_RSSIchannel < 0)
  {
    OSD.print( fixPStr(ON_OFF_STR[0]) );
  }
  else
  {
    if(settings.s.m_RSSIchannel < 4)
    {
      OSD.print( fixPStr(AUX_STR) );
      uint8_t tempCol = startCol + strlen_P(RSSI_CHANNEL_STR) + 4;
      OSD.printInt16(tempCol, startRow, settings.s.m_RSSIchannel+1, 0, 1, "", 5);
    }
    else
    {
      static const char TELEMETRY_STR[] PROGMEM = "telemetry";
      OSD.print( fixPStr(TELEMETRY_STR) );
    }
  }

  OSD.printIntArrow( startCol, ++startRow, RSSI_MIN_PPM_STR, settings.s.m_RSSImin, 0, 0, activeRSSIMenuItem, "", 1);
  
  OSD.printIntArrow( startCol, ++startRow, RSSI_MAX_PPM_STR, settings.s.m_RSSImax, 0, 0, activeRSSIMenuItem, "", 1);
  
  OSD.printP( startCol, ++startRow, BACK_STR, activeRSSIMenuItem );
  
  return (void*)RSSIMenu;
}


static uint8_t oldDVOrderPos[CSettings::DISPLAY_DV_SIZE] =  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
static uint8_t oldOSDItemsSel[CSettings::DISPLAY_DV_SIZE] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

void* DisplayMenu()
{
  if((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT))
  {
    bool gogglechanged, symbolChanged;
    uint8_t oldDVChannel = settings.s.m_DVchannel;
    switch(activeDisplayMenuItem)
    {
      case 0:
        settingChanged |= checkCode(settings.s.m_DVchannel, 1, 0, 4);
        if(settings.s.m_DVchannel != oldDVChannel)
        {
          uint8_t i;
          if(oldDVChannel == 4)
          {
            for(i=0; i<CSettings::DISPLAY_DV_SIZE; i++)
            {
              oldOSDItemsSel[i] = settings.m_DISPLAY_DV[i];
              settings.m_DISPLAY_DV[i] = oldDVOrderPos[i];             
            }            
          }
          if(oldDVChannel == 3 && settings.s.m_DVchannel == 4)
          {
            for(i=0; i<CSettings::DISPLAY_DV_SIZE; i++)
            {
              oldDVOrderPos[i] = settings.m_DISPLAY_DV[i];
              settings.m_DISPLAY_DV[i] = oldOSDItemsSel[i];             
            }            
          }
          settings.SetupPPMs(DV_PPMs);
        }
      break;
      case 1:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          cleanScreen();
          return (void*)RSSIMenu;          
        }
      break;
      case 2:
        settingChanged |= checkCode(settings.s.m_tempUnit, 1, 0, 1);
      break;
      case 3:
        settingChanged |= checkCode(settings.s.m_fontSize, 1, 0, 1);
      break;
      case 4:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          cleanScreen();
          return (void*)IconsMenu;          
        }
      break;
      case 5:
        gogglechanged = checkCode(settings.s.m_goggle, 1, 0, 1);
        if(gogglechanged) correctItemsOnce = false;
        settingChanged |= gogglechanged;
      break;
      case 6:
        settingChanged |= checkCode(settings.s.m_wattMeter, 1, 0, 2);
      break;
      case 7:
        settingChanged |= checkCode(settings.s.m_crossHair, 1, 0, 8);
      break;
      case 8:
        settingChanged |= checkCode(settings.s.m_stats, 1, 0, 2);
      break;
      case 9:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          activeDisplayMenuItem = 0;
          cleanScreen();
          return (void*)MainMenu;
        }
      break;
    }
  }
  static const uint8_t DISPLAY_MENU_ITEMS = 10;
  activeDisplayMenuItem = checkMenuItem(activeDisplayMenuItem, DISPLAY_MENU_ITEMS);
  
  static const char DV_CHANNEL_STR[] PROGMEM =      "reduce items :";
  static const char RSSI_MENU_STR[] PROGMEM =       "rssi";
  static const char TEMP_UNIT_STR[] PROGMEM =       "temp. unit   :";
  static const char FONT_SIZE_STR[] PROGMEM =       "font size    :";
  static const char SYMBOLS_SIZE_STR[] PROGMEM =    "icons";
  static const char GOGGLE_STR[] PROGMEM =          "goggle       :";
  static const char BEERMUG_STR[] PROGMEM =         "watt meter   :";
  static const char CROSSHAIR_STR[] PROGMEM =       "crosshair    :";
  static const char STATISTICS_STR[] PROGMEM =      "statistics   :";
//static const char BACK_STR[] PROGMEM =            "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(DV_CHANNEL_STR)+6)/2;
  static const char DISPLAY_TITLE_STR[] PROGMEM = "display menu";
  OSD.printP( settings.COLS/2 - strlen_P(DISPLAY_TITLE_STR)/2, ++startRow, DISPLAY_TITLE_STR );
  
  OSD.printP( startCol, ++startRow, DV_CHANNEL_STR, activeDisplayMenuItem );
  static const char AUX_STR[] PROGMEM = "aux";
  static const char FIXED_STR[] PROGMEM = "fixed";
  if(settings.s.m_DVchannel == 4)
  {
    OSD.print( fixPStr(FIXED_STR) );
  }
  else
  {
    OSD.print( fixPStr(AUX_STR) );
    uint8_t tempCol = startCol + strlen_P(DV_CHANNEL_STR) + 4;
    OSD.printInt16(tempCol, startRow, settings.s.m_DVchannel+1, 0, 1, " " );
  }

  OSD.printP( startCol, ++startRow, RSSI_MENU_STR, activeDisplayMenuItem );
    
  OSD.printP( startCol, ++startRow, TEMP_UNIT_STR, activeDisplayMenuItem);
  static const char tempSymbols[][2] PROGMEM = { {0xB0,0x00} , {0xB1, 0x00}};
  OSD.print( fixPStr(tempSymbols[settings.s.m_tempUnit]) );
  
  static const char NORMAL_FONT_STR[] PROGMEM = "normal";
  static const char LARGE_FONT_STR[] PROGMEM =  "large ";
  static const char* FONT_SIZES_STR[] = { NORMAL_FONT_STR, LARGE_FONT_STR };
  OSD.printP( startCol, ++startRow, FONT_SIZE_STR, activeDisplayMenuItem );
  OSD.print( fixPStr(FONT_SIZES_STR[settings.s.m_fontSize]) );
  
  OSD.printP( startCol, ++startRow, SYMBOLS_SIZE_STR, activeDisplayMenuItem );

  static const char FATSHARK_STR[] PROGMEM =   "fshark";
  static const char HEADPLAY_STR[] PROGMEM =   "hplay ";
  static const char* GOGGLES_STR[] = { FATSHARK_STR, HEADPLAY_STR };
  OSD.printP( startCol, ++startRow, GOGGLE_STR, activeDisplayMenuItem );
  OSD.print( fixPStr(GOGGLES_STR[settings.s.m_goggle]) );

  OSD.printP( startCol, ++startRow, BEERMUG_STR, activeDisplayMenuItem );
  static const char ON_OFF_BEER_STR[][8] PROGMEM = { "off    ", "on     ", "beermug" };
  OSD.print( fixPStr(ON_OFF_BEER_STR[settings.s.m_wattMeter]) );

  OSD.printP( startCol, ++startRow, CROSSHAIR_STR, activeDisplayMenuItem );
  static const char ON_OFF_STR_CROSS[][4] PROGMEM = { "off", "on ", "-3 ", "-2 ", "-1 ", "0  ", "+1 ", "+2 ", "+3 " };
  OSD.print( fixPStr(ON_OFF_STR_CROSS[settings.s.m_crossHair]) );

  OSD.printP( startCol, ++startRow, STATISTICS_STR, activeDisplayMenuItem );
  static const char ON_OFF_STAT_STR[][7] PROGMEM = { "off   ", "on    ", "hidden" };
  OSD.print( fixPStr(ON_OFF_STAT_STR[settings.s.m_stats]) );
  
  OSD.printP( startCol, ++startRow, BACK_STR, activeDisplayMenuItem );
  
  return (void*)DisplayMenu;
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
  if((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT))
  {
    switch(activeVTXMenuItem)
    {
      case 0:
        vTxSettingChanged |= checkCode(settings.s.m_vTxMinPower, 1, 0, maxPower);
      break;
      case 1:
        vTxSettingChanged |= checkCode(settings.s.m_vTxPower, 1, 0, maxPower);
      break;
      case 2:
        vTxSettingChanged |= checkCode(settings.s.m_vTxBand, 1, 0, 4);
        #ifdef AUSSIE_CHANNELS
        if(settings.s.m_vTxBand == 2 && settings.s.m_AussieChannels == 0)
        {
          settings.s.m_vTxBand += (int8_t)settings.s.m_vTxBand - (int8_t)_oldvTxBand;
        }
        #endif
      break;
      case 3:
        vTxSettingChanged |= checkCode(settings.s.m_vTxChannel, 1, 0, 7);
      break;
      case 4:
        settingChanged |= checkCode(settings.s.m_AussieChannels, 1, 0, 1);
      break;
      case 5:
        if(code &  inputChecker.ROLL_RIGHT)
        {
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
      case 6:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          activeVTXMenuItem = 0;
          if(!vTxSettingChanged)
          {
            settings.s.m_vTxPower = vTxPower;
            settings.s.m_vTxBand = vTxBand;
            settings.s.m_vTxChannel = vTxChannel;
          }
          vTxSettingChanged = false;
          cleanScreen();
          return (void*)MiscMenu;
        }
      break;
    }
  }
  static const uint8_t VTX_MENU_ITEMS = 7;
  activeVTXMenuItem = checkMenuItem(activeVTXMenuItem, VTX_MENU_ITEMS);

  static const char VTX_MIN_POWER_STR[] PROGMEM =  "min power :";
  static const char VTX_POWER_STR[] PROGMEM =      "power     :";
  static const char VTX_BAND_STR[] PROGMEM =       "band      :";
  static const char VTX_CHANNEL_STR[] PROGMEM =    "channel   :";
  static const char VTX_UNLOCK_STR[] PROGMEM =     "unlock vtx:";
//static const char SAVE_EXIT_STR[] PROGMEM =      "save+exit";
//static const char BACK_STR[] PROGMEM =           "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(VTX_POWER_STR)+6)/2;
  static const char VTX_TITLE_STR[] PROGMEM = "vtx menu";
  OSD.printP( settings.COLS/2 - strlen_P(VTX_TITLE_STR)/2, ++startRow, VTX_TITLE_STR );

  static const char VTX_POWERS_STR[][6] PROGMEM = { {"25mw "}, {"200mw"}, {"500mw"} };
  OSD.printP( startCol, ++startRow, VTX_MIN_POWER_STR, activeVTXMenuItem );
  OSD.print( fixPStr(VTX_POWERS_STR[settings.s.m_vTxMinPower]) );
  
  OSD.printP( startCol, ++startRow, VTX_POWER_STR, activeVTXMenuItem );
  OSD.print( fixPStr(VTX_POWERS_STR[settings.s.m_vTxPower]) );

  OSD.printP( startCol, ++startRow, VTX_BAND_STR, activeVTXMenuItem );
  OSD.print( fixPStr(bandSymbols[settings.s.m_vTxBand]) );

  OSD.printIntArrow( startCol, ++startRow, VTX_CHANNEL_STR, settings.s.m_vTxChannel+1, 0, 1, activeVTXMenuItem, "=" );
  OSD.printInt16( startCol + strlen_P(VTX_CHANNEL_STR) + 3, startRow, (int16_t)pgm_read_word(&vtx_frequencies[settings.s.m_vTxBand][settings.s.m_vTxChannel]), 0, 1, "mhz" );

  OSD.printP( startCol, ++startRow, VTX_UNLOCK_STR, activeVTXMenuItem );
  OSD.print( fixPStr(ON_OFF_STR[settings.s.m_AussieChannels]) );
  
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
      settingChanged |= checkCode(settings.s.m_vTxMaxPower, 50);
    break;
    case 1:
      if(code &  inputChecker.ROLL_RIGHT)
      {
        activeVTXMenuItem = 0;          
        cleanScreen();
        return (void*)MiscMenu;
      }
    break;
  }
  
  static const uint8_t VTX_MENU_ITEMS = 2;
  activeVTXMenuItem = checkMenuItem(activeVTXMenuItem, VTX_MENU_ITEMS);
  
  static const char VTX_MAX_POWER_STR[] PROGMEM =      "max vtx power: ";
//static const char BACK_STR[] PROGMEM =               "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(VTX_MAX_POWER_STR)+6)/2;
  static const char VTX_TITLE_STR[] PROGMEM = "vtx config menu";
  OSD.printP( settings.COLS/2 - strlen_P(VTX_TITLE_STR)/2, ++startRow, VTX_TITLE_STR );
  
  OSD.printIntArrow( startCol, ++startRow, VTX_MAX_POWER_STR, settings.s.m_vTxMaxPower, 0, 0, activeVTXMenuItem, "mw", 1);
  
  OSD.printP( startCol, ++startRow, BACK_STR, activeVTXMenuItem );
  
  return (void*)vTxMenu;
}
#endif

void* MiscMenu()
{
  if((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT))
  {
    switch(activeMiscMenuItem)
    {
      case 0:
        cleanScreen();
        return (void*)vTxMenu;
      break;
      case 1:
        settingChanged |= checkCode(settings.s.m_RCSplitControl, 1, 0, 1);
      break;
      case 2:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          activeMiscMenuItem = 0;          
          cleanScreen();
          return (void*)MainMenu;
        }
      break;
    }
  }
  static const uint8_t MISC_MENU_ITEMS = 3;
  activeMiscMenuItem = checkMenuItem(activeMiscMenuItem, MISC_MENU_ITEMS);
  
  static const char VTX_MENU_STR[] PROGMEM =      "vtx";
  static const char RC_SPLIT_STR[] PROGMEM =      "rc split:";
//static const char BACK_STR[] PROGMEM =          "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(RC_SPLIT_STR)+3)/2;
  static const char MISC_TITLE_STR[] PROGMEM = "misc menu";
  OSD.printP( settings.COLS/2 - strlen_P(MISC_TITLE_STR)/2, ++startRow, MISC_TITLE_STR );
  
  OSD.printP( startCol, ++startRow, VTX_MENU_STR, activeMiscMenuItem );
  
  OSD.printP( startCol, ++startRow, RC_SPLIT_STR, activeMiscMenuItem );
  OSD.print( fixPStr(ON_OFF_STR[settings.s.m_RCSplitControl]) );
  
  OSD.printP( startCol, ++startRow, BACK_STR, activeMiscMenuItem );
  
  return (void*)MiscMenu;
}

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
        setupNickname = true;
        charSelected = 0;
        charIndex = findCharPos(settings.s.m_nickname[charSelected]);
        return (void*)MainMenu; 
      break;
      case 3:
        cleanScreen();
        moveItems = true;
        settings.SetupPPMs(DV_PPMs, true);
        menuActive = false;
        moveSelected = 0;
        OSD_ITEM_BLINK[moveSelected] = true;
        return (void*)MainMenu; 
      break;
      case 4:
        cleanScreen();
        menuActive = false;
        shiftOSDactive = true;
        return (void*)MainMenu;
      break;
      case 5:
        cleanScreen();
        if(settings.s.m_DVchannel == 4) return (void*)SetOSDItems;
        else return (void*)ChangeOrder;
      break;      
      case 6:
        cleanScreen();
        return (void*)BatteryMenu;
      break;
      case 7:
        cleanScreen();
        return (void*)MiscMenu;
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
  static const char NICKNAME_STR[] PROGMEM =        "callsign";
  static const char MOVE_ITEMS_STR[] PROGMEM =      "move items";
  static const char CENTER_OSD_STR[] PROGMEM =      "center osd";
  static const char CHANGE_ORDER_STR[] PROGMEM =    "change order";  static const char SET_OSD_ITEMS_STR[] PROGMEM = "set items";
  static const char BATTERY_PAGE_STR[] PROGMEM =    "battery";
  static const char MISC_PAGE_STR[] PROGMEM =       "misc";
  static const char SAVE_STR[] PROGMEM =            "save";
  static const char RESET_STR[] PROGMEM =           "reset";
  
  uint8_t startRow = 0;
  uint8_t startCol = settings.COLS/2 - strlen_P(CHANGE_ORDER_STR)/2;
  OSD.setCursor( settings.COLS/2 - strlen_P(KISS_OSD_VER)/2, ++startRow );
  OSD.print( fixPStr(KISS_OSD_VER) );
  static const char MAIN_TITLE_STR[] PROGMEM = "main menu";
  OSD.printP( settings.COLS/2 - strlen_P(MAIN_TITLE_STR)/2, ++startRow, MAIN_TITLE_STR );
  
  OSD.printP( startCol, ++startRow, UPDATE_FONT_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, DISPLAY_PAGE_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, NICKNAME_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, MOVE_ITEMS_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, CENTER_OSD_STR, activeMenuItem );
  if(settings.s.m_DVchannel == 4) OSD.printP( startCol, ++startRow, SET_OSD_ITEMS_STR, activeMenuItem );  
  else OSD.printP( startCol, ++startRow, CHANGE_ORDER_STR, activeMenuItem );  
  OSD.printP( startCol, ++startRow, BATTERY_PAGE_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, MISC_PAGE_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, SAVE_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, RESET_STR, activeMenuItem );
  
  return (void*)MainMenu;
}
