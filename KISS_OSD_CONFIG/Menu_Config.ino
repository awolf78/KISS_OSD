static uint8_t activeBatteryMenuItem = 0;
static uint8_t activeDisplayMenuItem = 0;
static uint8_t activeVTXMenuItem = 0;
static uint8_t activeOrderMenuItem = 0;
static uint8_t activeResetMenuItem = 0;
static bool selectedOrder = false;
static uint8_t activeOrderMenuSelectedItem = 199;
static uint8_t confirmIndex = 0;


FLASH_STRING(BACK_STR,      "back");
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
  
  FLASH_STRING(TIMER_ORDER_STR,         "timer      ");
  FLASH_STRING(VOLTAGE_ORDER_STR,       "voltage    ");
  FLASH_STRING(THROTTLE_ORDER_STR,      "throttle   ");
  FLASH_STRING(MAH_ORDER_STR,           "mah        ");
  FLASH_STRING(NICKNAME_ORDER_STR,      "nickname   ");
  FLASH_STRING(AMPS_ORDER_STR,          "amps       ");
  FLASH_STRING(ESC_TEMP_ORDER_STR,      "esc temp   ");
  FLASH_STRING(ESC_VOLTAGE_ORDER_STR,   "esc voltage");
  FLASH_STRING(ESC_RPM_ORDER_STR,       "esc rpm    ");
//FLASH_STRING(BACK_STR,                "back");

  _FLASH_STRING *orderItems[CSettings::DISPLAY_DV_SIZE];
  orderItems[settings.m_DISPLAY_DV[DISPLAY_NICKNAME]] = &NICKNAME_ORDER_STR;
  orderItems[settings.m_DISPLAY_DV[DISPLAY_TIMER]] = &TIMER_ORDER_STR;
  orderItems[settings.m_DISPLAY_DV[DISPLAY_RC_THROTTLE]] = &THROTTLE_ORDER_STR;
  orderItems[settings.m_DISPLAY_DV[DISPLAY_COMB_CURRENT]] = &AMPS_ORDER_STR;
  orderItems[settings.m_DISPLAY_DV[DISPLAY_LIPO_VOLTAGE]] = &VOLTAGE_ORDER_STR;
  orderItems[settings.m_DISPLAY_DV[DISPLAY_MA_CONSUMPTION]] = &MAH_ORDER_STR;
  orderItems[settings.m_DISPLAY_DV[DISPLAY_ESC_KRPM]] = &ESC_RPM_ORDER_STR;
  orderItems[settings.m_DISPLAY_DV[DISPLAY_ESC_CURRENT]] = &ESC_VOLTAGE_ORDER_STR;
  orderItems[settings.m_DISPLAY_DV[DISPLAY_ESC_TEMPERATURE]] = &ESC_TEMP_ORDER_STR;
  
  uint8_t startRow = 1;
  uint8_t startCol = COLS/2 - ESC_VOLTAGE_ORDER_STR.length()/2;
  FLASH_STRING(ORDER_TITLE_STR, "order menu");
  OSD.printFS(COLS/2 - ORDER_TITLE_STR.length()/2, ++startRow, &ORDER_TITLE_STR);
  for(i=0; i<CSettings::DISPLAY_DV_SIZE; i++)
  {
    if(i == activeOrderMenuSelectedItem) OSD.blink1sec(); 
    OSD.printFS( startCol, ++startRow, orderItems[i], activeOrderMenuItem );
    OSD.noBlink();
  }  
  OSD.printFS( startCol, ++startRow, &BACK_STR, activeOrderMenuItem );
  
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
      if(code &  inputChecker.ROLL_RIGHT)
      {
        cleanScreen();
        activeBatteryMenuItem = 0;
        return (void*)MainMenu;
      }
    break;
  }
  static const uint8_t BATTERY_MENU_ITEMS = 4;
  activeBatteryMenuItem = checkMenuItem(activeBatteryMenuItem, BATTERY_MENU_ITEMS);
  
  FLASH_STRING(SELECT_BATTERY_STR,  "select battery ");
  FLASH_STRING(BATTERY_WARNING_STR, "batt. warning: ");
  FLASH_STRING(BATTERY_PERCENT_STR, "batt. % alarm: ");
//FLASH_STRING(BACK_STR,            "back");
  
  uint8_t startRow = 1;
  uint8_t startCol = COLS/2 - (BATTERY_WARNING_STR.length()+4)/2;
  FLASH_STRING(BATTERY_TITLE_STR, "battery menu");
  OSD.printFS(COLS/2 - BATTERY_TITLE_STR.length()/2, ++startRow, &BATTERY_TITLE_STR);
  
  OSD.printFS( startCol, ++startRow, &SELECT_BATTERY_STR, activeBatteryMenuItem );
  OSD.printFS( startCol, ++startRow, &BATTERY_WARNING_STR, activeBatteryMenuItem );
  OSD.print( fixStr(ON_OFF_STR[settings.m_batWarning]) );
  
  OSD.printIntArrow( startCol, ++startRow, &BATTERY_PERCENT_STR, settings.m_batWarningPercent, 0, 1, activeBatteryMenuItem, "%", true );  
  
  OSD.printFS( startCol, ++startRow, &BACK_STR, activeBatteryMenuItem );
  
  return (void*)BatteryMenu;
}



void* DisplayMenu()
{
  if((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT))
  {
    bool gogglechanged;
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
        settingChanged |= checkCode(settings.m_displaySymbols, 1, 0, 1);
      break;
      case 4:
        gogglechanged = checkCode(settings.m_goggle, 1, 0, 1);
        if(gogglechanged) correctItemsOnce = false;
        settingChanged |= gogglechanged;
      break;
      case 5:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          cleanScreen();
          menuActive = false;
          shiftOSDactive = true;
          return (void*)DisplayMenu;
        }
      break;
      case 6:
        if(code &  inputChecker.ROLL_RIGHT)
        {
          activeDisplayMenuItem = 0;
          cleanScreen();
          return (void*)MainMenu;
        }
      break;
    }
  }
  static const uint8_t DISPLAY_MENU_ITEMS = 7;
  activeDisplayMenuItem = checkMenuItem(activeDisplayMenuItem, DISPLAY_MENU_ITEMS);
  
  FLASH_STRING(DV_CHANNEL_STR,      "dv channel : ");
  FLASH_STRING(TEMP_UNIT_STR,       "temp. unit : ");
  FLASH_STRING(FONT_SIZE_STR,       "font size  : ");
  FLASH_STRING(SYMBOLS_SIZE_STR,    "symbols    : ");
  FLASH_STRING(GOGGLE_STR,          "goggle     : ");
  FLASH_STRING(CENTER_OSD_STR,      "center osd");
//FLASH_STRING(BACK_STR,            "back");
  
  uint8_t startRow = 1;
  uint8_t startCol = COLS/2 - (DV_CHANNEL_STR.length()+6)/2;
  FLASH_STRING(DISPLAY_TITLE_STR, "display menu");
  OSD.printFS( COLS/2 - DISPLAY_TITLE_STR.length()/2, ++startRow, &DISPLAY_TITLE_STR );
  
  OSD.printFS( startCol, ++startRow, &DV_CHANNEL_STR, activeDisplayMenuItem );
  OSD.print( fixStr("aux") );
  uint8_t tempCol = startCol + DV_CHANNEL_STR.length() + 4;
  OSD.printInt16(tempCol, startRow, settings.m_DVchannel+1, 0, 1 );
  
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

  FLASH_STRING(FATSHARK_STR,  "fshark");
  FLASH_STRING(HEADPLAY_STR,  "hplay ");
  static _FLASH_STRING GOGGLES_STR[] = { FATSHARK_STR, HEADPLAY_STR };
  OSD.printFS( startCol, ++startRow, &GOGGLE_STR, activeDisplayMenuItem );
  OSD.print( fixFlashStr(&GOGGLES_STR[settings.m_goggle]) );

  OSD.printFS( startCol, ++startRow, &CENTER_OSD_STR, activeDisplayMenuItem );
  OSD.printFS( startCol, ++startRow, &BACK_STR, activeDisplayMenuItem );
  
  return (void*)DisplayMenu;
}

static bool vTxSettingChanged = false;

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
        vTxPower = settings.m_vTxPower;
        vTxBand = settings.m_vTxBand;
        vTxChannel = settings.m_vTxChannel;
        settingChanged |= vTxSettingChanged;
        menuWasActive = true;
        cleanScreen();
        vtx_set_frequency(vTxBand, vTxChannel);
        return (void*)MainMenu;
      break;
    }
  }
  static const uint8_t VTX_MENU_ITEMS = 4;
  activeVTXMenuItem = checkMenuItem(activeVTXMenuItem, VTX_MENU_ITEMS);
  
  FLASH_STRING(VTX_POWER_STR,      "power   : ");
  FLASH_STRING(VTX_BAND_STR,       "band    : ");
  FLASH_STRING(VTX_CHANNEL_STR,    "channel : ");
  FLASH_STRING(SET_AND_BACK_STR,   "set+back");
  
  uint8_t startRow = 1;
  uint8_t startCol = COLS/2 - (VTX_POWER_STR.length()+6)/2;
  FLASH_STRING(VTX_TITLE_STR, "vtx menu");
  OSD.printFS( COLS/2 - VTX_TITLE_STR.length()/2, ++startRow, &VTX_TITLE_STR );

  FLASH_STRING(_25MW_STR,   "25mw ");
  FLASH_STRING(_200MW_STR,  "200mw");
  FLASH_STRING(_500MW_STR,  "500mw");
  static _FLASH_STRING VTX_POWERS_STR[] = { _25MW_STR, _200MW_STR, _500MW_STR };
  OSD.printFS( startCol, ++startRow, &VTX_POWER_STR, activeVTXMenuItem );
  OSD.print( fixFlashStr(&VTX_POWERS_STR[settings.m_vTxPower]) );

  OSD.printFS( startCol, ++startRow, &VTX_BAND_STR, activeVTXMenuItem );
#ifdef IMPULSERC_VTX
  OSD.print( fixStr(bandSymbols[settings.m_vTxBand]) );
#endif
  
  OSD.printIntArrow( startCol, ++startRow, &VTX_CHANNEL_STR, settings.m_vTxChannel+1, 0, 1, activeVTXMenuItem, "=" );
#ifdef IMPULSERC_VTX
  uint8_t tempCol = startCol + VTX_CHANNEL_STR.length() + 3;
  OSD.printInt16(tempCol, startRow, (int16_t)pgm_read_word(&vtx_frequencies[settings.m_vTxBand][settings.m_vTxChannel]), 0, 1, "mhz" );
#endif
  
  OSD.printFS( startCol, ++startRow, &SET_AND_BACK_STR, activeVTXMenuItem );
  
  return (void*)vTxMenu;
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
  
  FLASH_STRING(RESTORE_SAVE_STR,   "restore last save");
  FLASH_STRING(RESTORE_DEF_STR,    "restore defaults ");
  FLASH_STRING(CONFIRM_RESET_STR,  "confirm          ");
//FLASH_STRING(BACK_STR,           "back");
  
  uint8_t startRow = 1;
  uint8_t startCol = COLS/2 - (RESTORE_SAVE_STR.length()+6)/2;
  FLASH_STRING(RESET_TITLE_STR, "reset menu");
  OSD.printFS( COLS/2 - RESET_TITLE_STR.length()/2, ++startRow, &RESET_TITLE_STR );

  if(activeResetMenuItem == 0 && confirmIndex == 1)
  {
    OSD.printFS( startCol, ++startRow, &CONFIRM_RESET_STR, activeResetMenuItem );    
  }
  else
  {
    OSD.printFS( startCol, ++startRow, &RESTORE_SAVE_STR, activeResetMenuItem );
  }

  if(activeResetMenuItem == 1 && confirmIndex == 1)
  {
    OSD.printFS( startCol, ++startRow, &CONFIRM_RESET_STR, activeResetMenuItem );    
  }
  else
  {
    OSD.printFS( startCol, ++startRow, &RESTORE_DEF_STR, activeResetMenuItem );
  }
  
  OSD.printFS( startCol, ++startRow, &BACK_STR, activeResetMenuItem );
  
  return (void*)ResetMenu;
}

void* MainMenu()
{
  FLASH_STRING(FONT_UPDATE_STR,"font will be updated");
  FLASH_STRING(POWER_WARNING_STR,"do not power off!!!");
  if(code &  inputChecker.ROLL_RIGHT)
  {
    switch(activeMenuItem)
    {
      case 0:
        cleanScreen();
        OSD.printFS(COLS/2 - FONT_UPDATE_STR.length()/2, ROWS/2-1, &FONT_UPDATE_STR);
        OSD.printFS(COLS/2 - POWER_WARNING_STR.length()/2, ROWS/2, &POWER_WARNING_STR);
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
  
  FLASH_STRING(UPDATE_FONT_STR,     "update font");
  FLASH_STRING(DISPLAY_PAGE_STR,    "display");
  FLASH_STRING(MOVE_ITEMS_STR,      "move items");
  FLASH_STRING(CENTER_OSD_STR,      "center osd");
  FLASH_STRING(CHANGE_ORDER_STR,    "change order");
  FLASH_STRING(NICKNAME_STR,        "nickname");
  FLASH_STRING(BATTERY_PAGE_STR,    "battery");
  FLASH_STRING(VTX_PAGE_STR,        "vtx");
  FLASH_STRING(SAVE_STR,            "save");
  FLASH_STRING(RESET_STR,           "reset");
  
  uint8_t startRow = 0;
  uint8_t startCol = COLS/2 - CHANGE_ORDER_STR.length()/2;
  OSD.setCursor( COLS/2 - strlen(KISS_OSD_VER)/2, ++startRow );
  OSD.print( fixStr(KISS_OSD_VER) );
  FLASH_STRING(MAIN_TITLE_STR, "main menu");
  OSD.printFS( COLS/2 - MAIN_TITLE_STR.length()/2, ++startRow, &MAIN_TITLE_STR );
  
  OSD.printFS( startCol, ++startRow, &UPDATE_FONT_STR, activeMenuItem );
  OSD.printFS( startCol, ++startRow, &DISPLAY_PAGE_STR, activeMenuItem );
  OSD.printFS( startCol, ++startRow, &MOVE_ITEMS_STR, activeMenuItem );
  OSD.printFS( startCol, ++startRow, &CENTER_OSD_STR, activeMenuItem );
  OSD.printFS( startCol, ++startRow, &CHANGE_ORDER_STR, activeMenuItem );
  OSD.printFS( startCol, ++startRow, &NICKNAME_STR, activeMenuItem );
  OSD.printFS( startCol, ++startRow, &BATTERY_PAGE_STR, activeMenuItem );
  OSD.printFS( startCol, ++startRow, &VTX_PAGE_STR, activeMenuItem );
  OSD.printFS( startCol, ++startRow, &SAVE_STR, activeMenuItem );
  OSD.printFS( startCol, ++startRow, &RESET_STR, activeMenuItem );
  
  return (void*)MainMenu;
}
