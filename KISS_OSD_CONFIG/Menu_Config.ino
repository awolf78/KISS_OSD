static uint8_t activeBatteryMenuItem = 0;
static uint8_t activeDisplayMenuItem = 0;
static uint8_t activeVTXMenuItem = 0;
static uint8_t activeOrderMenuItem = 0;
static uint8_t activeResetMenuItem = 0;
static uint8_t activeIconsMenuItem = 0;
static uint8_t activeOSDItemsMenuItem = 0;
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

boolean checkCode(volatile int8_t &value, int16_t STEP, int16_t minVal = 0, int16_t maxVal = 32000)
{
  int16_t tempValue = value;
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
    if(settings.m_wattMeter && reverseLUT[i] == DISPLAY_COMB_CURRENT) OSD.printP( startCol, ++startRow, WATT_ORDER_STR, activeOrderMenuItem );
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
    OSD.print( fixStr(ON_OFF_STR[settings.m_DISPLAY_DV[i]]) );
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



void* IconsMenu()
{
  if((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT))
  {
    uint8_t oldVal;
    uint8_t temp1;
    switch(activeIconsMenuItem)
    {
      case 0:
        settings.m_oldDisplaySymbols = settings.m_displaySymbols;
        settingChanged |= checkCode(settings.m_displaySymbols, 1, 0, 1);
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
        if((activeIconsMenuItem-1) == ESC_ICON && oldVal != settings.m_IconSettings[ESC_ICON] && settings.m_displaySymbols == 1) 
        {
          temp1 = settings.m_IconSettings[PROPS_ICON];
          settings.m_IconSettings[PROPS_ICON] = 0;
          settings.m_oldDisplaySymbols = 2;
          settings.m_displaySymbols = settings.m_IconSettings[ESC_ICON];
          settings.m_IconSettings[ESC_ICON] = 1;
          settings.fixColBorders();
          settings.m_IconSettings[ESC_ICON] = settings.m_displaySymbols;
          settings.m_IconSettings[PROPS_ICON] = temp1;
          settings.m_displaySymbols = 1;
          settings.m_oldDisplaySymbols = 1;
          
        }
        if((activeIconsMenuItem-1) == (uint8_t)PROPS_ICON && oldVal != settings.m_IconSettings[PROPS_ICON] && settings.m_displaySymbols == 1) 
        {
          temp1 = settings.m_IconSettings[ESC_ICON];
          settings.m_IconSettings[ESC_ICON] = 0;
          settings.m_oldDisplaySymbols = 2;
          settings.m_displaySymbols = settings.m_IconSettings[PROPS_ICON];
          settings.m_IconSettings[PROPS_ICON] = 1;
          settings.fixColBorders();
          settings.m_IconSettings[PROPS_ICON] = settings.m_displaySymbols;
          settings.m_IconSettings[ESC_ICON] = temp1;
          settings.m_displaySymbols = 1;
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
  OSD.print( fixStr(ON_OFF_STR[settings.m_displaySymbols]) );
  
  uint8_t i;
  for(i=0; i<settings.ICON_SETTINGS_SIZE; i++)
  {
    OSD.printP( startCol, ++startRow, ALL_ICONS_STR[i], activeIconsMenuItem );
    OSD.print( fixStr(ON_OFF_STR[settings.m_IconSettings[i]]) );
  }
  
  OSD.printP( startCol, ++startRow, BACK_STR, activeIconsMenuItem );
  
  return (void*)IconsMenu;
}

static uint8_t oldDVOrderPos[CSettings::DISPLAY_DV_SIZE] =  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
static uint8_t oldOSDItemsSel[CSettings::DISPLAY_DV_SIZE] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

void* DisplayMenu()
{
  if((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT))
  {
    bool gogglechanged, symbolChanged;
    uint8_t oldDVChannel = settings.m_DVchannel;
    switch(activeDisplayMenuItem)
    {
      case 0:
        settingChanged |= checkCode(settings.m_DVchannel, 1, 0, 4);
        if(settings.m_DVchannel != oldDVChannel && oldDVChannel > 2 && settings.m_DVchannel > 2)
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
          else
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
        settingChanged |= checkCode(settings.m_RSSIchannel, 1, -1, 3);
      break;
      case 2:
        settingChanged |= checkCode(settings.m_tempUnit, 1, 0, 1);
      break;
      case 3:
        settingChanged |= checkCode(settings.m_fontSize, 1, 0, 1);
      break;
      case 4:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          cleanScreen();
          return (void*)IconsMenu;          
        }
      break;
      case 5:
        gogglechanged = checkCode(settings.m_goggle, 1, 0, 1);
        if(gogglechanged) correctItemsOnce = false;
        settingChanged |= gogglechanged;
      break;
      case 6:
        settingChanged |= checkCode(settings.m_wattMeter, 1, 0, 2);
      break;
      case 7:
        settingChanged |= checkCode(settings.m_crossHair, 1, 0, 8);
      break;
      case 8:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          activeDisplayMenuItem = 0;
          cleanScreen();
          return (void*)MainMenu;
        }
      break;
    }
  }
  static const uint8_t DISPLAY_MENU_ITEMS = 9;
  activeDisplayMenuItem = checkMenuItem(activeDisplayMenuItem, DISPLAY_MENU_ITEMS);
  
  static const char DV_CHANNEL_STR[] PROGMEM =      "dv channel : ";
  static const char RSSI_CHANNEL_STR[] PROGMEM =    "rssi chan. : ";
  static const char TEMP_UNIT_STR[] PROGMEM =       "temp. unit : ";
  static const char FONT_SIZE_STR[] PROGMEM =       "font size  : ";
  static const char SYMBOLS_SIZE_STR[] PROGMEM =    "icons";
  static const char GOGGLE_STR[] PROGMEM =          "goggle     : ";
  static const char BEERMUG_STR[] PROGMEM =         "watt meter : ";
  static const char CROSSHAIR_STR[] PROGMEM =       "crosshair  : ";
//static const char BACK_STR[] PROGMEM =            "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(DV_CHANNEL_STR)+6)/2;
  static const char DISPLAY_TITLE_STR[] PROGMEM = "display menu";
  OSD.printP( settings.COLS/2 - strlen_P(DISPLAY_TITLE_STR)/2, ++startRow, DISPLAY_TITLE_STR );
  
  OSD.printP( startCol, ++startRow, DV_CHANNEL_STR, activeDisplayMenuItem );
  if(settings.m_DVchannel == 4)
  {
    OSD.print( fixStr("fixed") );
  }
  else
  {
    OSD.print( fixStr("aux") );
    uint8_t tempCol = startCol + strlen_P(DV_CHANNEL_STR) + 4;
    OSD.printInt16(tempCol, startRow, settings.m_DVchannel+1, 0, 1, " " );
  }

  OSD.printP( startCol, ++startRow, RSSI_CHANNEL_STR, activeDisplayMenuItem );
  if(settings.m_RSSIchannel < 0)
  {
    OSD.print( fixStr("off") );
  }
  else
  {
    OSD.print( fixStr("aux") );
    uint8_t tempCol = startCol + strlen_P(RSSI_CHANNEL_STR) + 4;
    OSD.printInt16(tempCol, startRow, settings.m_RSSIchannel+1, 0, 1 );
  }
    
  OSD.printP( startCol, ++startRow, TEMP_UNIT_STR, activeDisplayMenuItem);
  static const char tempSymbols[][2] = { {0xB0,0x00} , {0xB1, 0x00}};
  OSD.print( fixStr(tempSymbols[settings.m_tempUnit]) );
  
  static const char NORMAL_FONT_STR[] PROGMEM = "normal";
  static const char LARGE_FONT_STR[] PROGMEM =  "large ";
  static const char* FONT_SIZES_STR[] = { NORMAL_FONT_STR, LARGE_FONT_STR };
  OSD.printP( startCol, ++startRow, FONT_SIZE_STR, activeDisplayMenuItem );
  OSD.print( fixPStr(FONT_SIZES_STR[settings.m_fontSize]) );
  
  OSD.printP( startCol, ++startRow, SYMBOLS_SIZE_STR, activeDisplayMenuItem );

  static const char FATSHARK_STR[] PROGMEM =   "fshark";
  static const char HEADPLAY_STR[] PROGMEM =   "hplay ";
  static const char* GOGGLES_STR[] = { FATSHARK_STR, HEADPLAY_STR };
  OSD.printP( startCol, ++startRow, GOGGLE_STR, activeDisplayMenuItem );
  OSD.print( fixPStr(GOGGLES_STR[settings.m_goggle]) );

  OSD.printP( startCol, ++startRow, BEERMUG_STR, activeDisplayMenuItem );
  static char ON_OFF_BEER_STR[][8] = { "off    ", "on     ", "beermug" };
  OSD.print( fixStr(ON_OFF_BEER_STR[settings.m_wattMeter]) );

  OSD.printP( startCol, ++startRow, CROSSHAIR_STR, activeDisplayMenuItem );
  static const char ON_OFF_STR_CROSS[][4] = { "off", "on ", "-3 ", "-2 ", "-1 ", "0  ", "+1 ", "+2 ", "+3 " };
  OSD.print( fixStr(ON_OFF_STR_CROSS[settings.m_crossHair]) );
  
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
#else
void* vTxMenu()
{
  if((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT))
  {
    switch(activeVTXMenuItem)
    {
      case 0:
        settingChanged |= checkCode(settings.m_vTxMaxPower, 50);
      break;
      case 1:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          activeVTXMenuItem = 0;          
          cleanScreen();
          return (void*)MainMenu;
        }
      break;
    }
  }
  static const uint8_t VTX_MENU_ITEMS = 2;
  activeVTXMenuItem = checkMenuItem(activeVTXMenuItem, VTX_MENU_ITEMS);
  
  static const char VTX_MAX_POWER_STR[] PROGMEM =      "max vtx power: ";
//static const char BACK_STR[] PROGMEM =               "back";
  
  uint8_t startRow = 1;
  uint8_t startCol = settings.COLS/2 - (strlen_P(VTX_MAX_POWER_STR)+6)/2;
  static const char VTX_TITLE_STR[] PROGMEM = "vtx config menu";
  OSD.printP( settings.COLS/2 - strlen_P(VTX_TITLE_STR)/2, ++startRow, VTX_TITLE_STR );
  
  OSD.printIntArrow( startCol, ++startRow, VTX_MAX_POWER_STR, settings.m_vTxMaxPower, 0, 0, activeVTXMenuItem, "mw", 1);
  
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
        setupNickname = true;
        charSelected = 0;
        charIndex = findCharPos(settings.m_nickname[charSelected]);
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
        if(settings.m_DVchannel == 4) return (void*)SetOSDItems;
        else return (void*)ChangeOrder;
      break;      
      case 6:
        cleanScreen();
        return (void*)BatteryMenu;
      break;
      case 7:
        cleanScreen();
        return (void*)vTxMenu;
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
  OSD.printP( startCol, ++startRow, NICKNAME_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, MOVE_ITEMS_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, CENTER_OSD_STR, activeMenuItem );
  if(settings.m_DVchannel == 4) OSD.printP( startCol, ++startRow, SET_OSD_ITEMS_STR, activeMenuItem );  
  else OSD.printP( startCol, ++startRow, CHANGE_ORDER_STR, activeMenuItem );  
  OSD.printP( startCol, ++startRow, BATTERY_PAGE_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, VTX_PAGE_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, SAVE_STR, activeMenuItem );
  OSD.printP( startCol, ++startRow, RESET_STR, activeMenuItem );
  
  return (void*)MainMenu;
}
