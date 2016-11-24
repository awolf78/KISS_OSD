/*
KISS FC OSD v1.0
By Felix Niessen (felix.niessen@googlemail.com)
for Flyduino.net

KISS FC OSD v2.0
by Alexander Wolf (awolf78@gmx.de)
for everyone who loves KISS

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/




// CONFIGURATION
//=========================================================================================================================
#define NICKNAME "airwolf"

// video system
//=============================
//#define PAL
#define NTSC

// MAX7456 Charset (change if you get sensless signs)
//=============================
#define USE_MAX7456_ASCII
//#define USE_MAX7456_MAXIM

// motors magnepole count (to display the right RPMs)
//=============================
#define MAGNETPOLECOUNT 14 // 2 for ERPMs

// Filter for ESC datas (higher value makes them less erratic) 0 = no filter, 20 = very strong filter
//=============================
#define ESC_FILTER 10

/*
Digital Volume configuration tool
---------------------------------
Use this feature to increase the amount of data displayed in reduced mode.
Only makes sense of you have a remote with a digital volume (DV) dial. 
Assign the DV ony our remote to your DV_CHAN. Anything you do not want to 
display set to -1.
Example:
You want the Lipo voltage and mAh consumption to be shown at all times and then be able
to show your nickname and after the the combination current with the DV dial:
- set DV_CHAN to the channel for the DV dial (and set it to the same channel on your radio)
- set DISPLAY_MA_CONSUMPTION_DV to 0
- set DISPLAY_LIPO_VOLTAGE_DV to 0
- set DISPLAY_NICKNAME_DV to 1
- set DISPLAY_COMB_CURRENT_DV to 2
- set the other DISPLAY_..._DV values to -1 
When you turn your radio on with DV all the way to minimum volume you will see only the Lipo voltage and
the the mAh consumption. Turning the DV dial a little higher your nickname will pop up (don't forget to arm 
first). Turning it even further will display the combination current.
=============================*/
static int16_t DISPLAY_NICKNAME_DV =           2; // 0-10, 0 = always on, -1 = never on, 1 = first, 2 = second, etc.  
static int16_t DISPLAY_TIMER_DV =              1; // 0-10, 0 = always on, -1 = never on, 1 = first, 2 = second, etc.
static int16_t DISPLAY_RC_THROTTLE_DV =        6; // 0-10, 0 = always on, -1 = never on, 1 = first, 2 = second, etc.
static int16_t DISPLAY_COMB_CURRENT_DV =       3; // 0-10, 0 = always on, -1 = never on, 1 = first, 2 = second, etc.
static int16_t DISPLAY_LIPO_VOLTAGE_DV =       0; // 0-10, 0 = always on, -1 = never on, 1 = first, 2 = second, etc.
static int16_t DISPLAY_MA_CONSUMPTION_DV =     0; // 0-10, 0 = always on, -1 = never on, 1 = first, 2 = second, etc.
static int16_t DISPLAY_ESC_KRPM_DV =           5; // 0-10, 0 = always on, -1 = never on, 1 = first, 2 = second, etc.
static int16_t DISPLAY_ESC_CURRENT_DV =        4; // 0-10, 0 = always on, -1 = never on, 1 = first, 2 = second, etc.
static int16_t DISPLAY_STATS_DV =              0; // 0-10, 0 = always on, -1 = never on, 1 = first, 2 = second, etc.
static int16_t DISPLAY_ESC_TEMPERATURE_DV =    7; // 0-10, 0 = always on, -1 = never on, 1 = first, 2 = second, etc.

/* Low battery warning config
-----------------------------
This feature will show a flashing "BATTERY LOW" warning just below the center of your 
display when bat_mAh_warning is exceeded - letting you know when it is time to land. 
I have found the mAh calculation of my KISS setup (KISS FC with 24RE ESCs using ESC telemetry) to 
be very accurate, so be careful when using other ESCs - mAh calculation might not be as accurate. 
While disarmed, yaw to the right. This will display the selected battery. Roll left or right to 
select a different battery. Pitch up or down to change the battery capacity. The warning will be 
displayed according to the percentage configured in the menu. Enter the menu with a yaw to the left 
for more than 3 seconds. Default is 23%, so if you have a 1300 mAh battery, it would start warning 
you at 1001 mAh used capacity.
=============================*/
static const int16_t BAT_MAH_INCREMENT = 50;

// END OF CONFIGURATION
//=========================================================================================================================


// internals
//=============================

//#define DEBUG
//#define PROTODEBUG

const char KISS_OSD_VER[] = "kiss osd v2.1";

#include "Flash.h"
#include <SPI.h>
#include <MAX7456.h>
#include "MyMAX7456.h"
#include <EEPROM.h>
#include "SerialPort.h"
#include "CSettings.h"
#include "CStickInput.h"
#include "fixFont.h"
#include "CMeanFilter.h"

const uint8_t osdChipSelect          =            6;
const byte masterOutSlaveIn          =            MOSI;
const byte masterInSlaveOut          =            MISO;
const byte slaveClock                =            SCK;
const byte osdReset                  =            2;

#ifdef PAL
static const uint8_t ROWS = MAX7456_ROWS_P1;
static const uint8_t COLS = MAX7456_COLS_P1;
#else
static const uint8_t ROWS = MAX7456_ROWS_N0;
static const uint8_t COLS = MAX7456_COLS_N1;
#endif

CMyMax7456 OSD( osdChipSelect );
SerialPort<0, 64, 0> NewSerial;
CStickInput inputChecker;
CSettings settings;

static const int16_t DV_PPM_INCREMENT = 100;

int16_t setupPPM(int16_t pos) 
{
  if(pos < 0) 
  {
    return 10000;
  }
  return -1000 + (pos * DV_PPM_INCREMENT);
}

void cleanScreen() 
{
  OSD.clear();
  while(!OSD.clearIsBusy());
}

static uint8_t lastTempUnit;

void setupMAX7456()
{
  #if defined(PAL)
    OSD.begin(COLS,ROWS,0);
    OSD.setTextOffset(-1,-6);
    OSD.setDefaultSystem(MAX7456_PAL);
  #endif
  #if defined(NTSC)
    OSD.begin(COLS,ROWS);
    OSD.setDefaultSystem(MAX7456_NTSC);
  #endif
  OSD.setSwitchingTime( 5 );   
  #if defined(USE_MAX7456_ASCII)
    OSD.setCharEncoding( MAX7456_ASCII );  
  #endif 
  #if defined(USE_MAX7456_MAXIM)
    OSD.setCharEncoding( MAX7456_MAXIM );  
  #endif
  OSD.display(); 
}

void setup()
{
  uint8_t i = 0;
  SPI.begin();
  SPI.setClockDivider( SPI_CLOCK_DIV2 ); 
  setupMAX7456();
  
  //clean used area
  while (!OSD.notInVSync());
  cleanScreen();
  settings.ReadSettings();
  lastTempUnit = settings.m_tempUnit;
  DISPLAY_NICKNAME_DV = setupPPM(DISPLAY_NICKNAME_DV);
  DISPLAY_TIMER_DV = setupPPM(DISPLAY_TIMER_DV);
  DISPLAY_RC_THROTTLE_DV = setupPPM(DISPLAY_RC_THROTTLE_DV);
  DISPLAY_COMB_CURRENT_DV = setupPPM(DISPLAY_COMB_CURRENT_DV);
  DISPLAY_LIPO_VOLTAGE_DV = setupPPM(DISPLAY_LIPO_VOLTAGE_DV);
  DISPLAY_MA_CONSUMPTION_DV = setupPPM(DISPLAY_MA_CONSUMPTION_DV);
  DISPLAY_ESC_KRPM_DV = setupPPM(DISPLAY_ESC_KRPM_DV);
  DISPLAY_ESC_CURRENT_DV = setupPPM(DISPLAY_ESC_CURRENT_DV);
  DISPLAY_STATS_DV = setupPPM(DISPLAY_STATS_DV);
  DISPLAY_ESC_TEMPERATURE_DV = setupPPM(DISPLAY_ESC_TEMPERATURE_DV);
  NewSerial.begin(115200);
}

static int16_t  throttle = 0;
static int16_t  roll = 0;
static int16_t  pitch = 0;
static int16_t  yaw = 0;
static uint16_t current = 0;
static int8_t armed = 0;
static int8_t idleTime = 0;
static int16_t LipoVoltage = 0;
static int16_t LipoMAH = 0;
static int16_t  previousMAH = 0;
static int16_t totalMAH = 0;
static int16_t MaxAmps = 0;
static int16_t MaxC    = 0;
static int16_t MaxRPMs = 0;
static int16_t MaxWatt = 0;
static int16_t MaxTemp = 0;
static int16_t MinBat = 0;
static int16_t motorKERPM[4] = {0,0,0,0};
static int16_t maxKERPM[4] = {0,0,0,0};
static int16_t motorCurrent[4] = {0,0,0,0};
static int16_t maxCurrent[4] = {0,0,0,0};
static int16_t ESCTemps[4] = {0,0,0,0};
static int16_t maxTemps[4] = {0,0,0,0};
static int16_t ESCVoltage[4] = {0,0,0,0};
static int16_t minVoltage[4] = {10000,10000,10000,10000};
static int16_t ESCmAh[4] = {0,0,0,0};
static int16_t  AuxChanVals[4] = {0,0,0,0};
static unsigned long start_time = 0;
static unsigned long time = 0;
static unsigned long old_time = 0;
static unsigned long total_time = 0;
static unsigned long DV_change_time = 0;
static int16_t last_Aux_Val = -10000;
static boolean armedOnce = false;
static boolean showBat = false;
static boolean settingChanged = false;
static uint8_t statPage = 0;


uint32_t ESC_filter(uint32_t oldVal, uint32_t newVal){
  return (uint32_t)((uint32_t)((uint32_t)((uint32_t)oldVal*ESC_FILTER)+(uint32_t)newVal))/(ESC_FILTER+1);
}

int16_t findMax4(int16_t maxV, int16_t *values, int16_t length) 
{
  for(uint8_t i = 0; i < length; i++) 
  {
    if(values[i] > maxV) 
    {
      maxV = values[i];
    }
  }
  return maxV;
}


int16_t findMax(int16_t maxV, int16_t newVal) 
{
  if(newVal > maxV) {
    return newVal;
  }
  return maxV;
}

int16_t findMin(int16_t minV, int16_t newVal) 
{
  if(newVal < minV) {
    return newVal;
  }
  return minV;
}

void ReviveOSD()
{
  while(OSD.resetIsBusy()) delay(100);
  OSD.reset();
  while(OSD.resetIsBusy()) delay(100);
  setupMAX7456();
}

static int16_t p_roll = 0;
static int16_t p_pitch, p_yaw, p_tpa, i_roll, i_pitch, i_yaw, i_tpa, d_roll, d_pitch, d_yaw, d_tpa;
static int16_t rcrate_roll, rate_roll, rccurve_roll, rcrate_pitch, rate_pitch, rccurve_pitch, rcrate_yaw, rate_yaw, rccurve_yaw;
static int16_t lpf_frq;
static uint8_t notchFilterEnabled = 2;
static int16_t notchFilterCenter, notchFilterCut;
static int16_t yawFilterCut;
static boolean fcSettingsReceived = false;
static boolean armOnYaw = true;
static uint32_t LastLoopTime = 0;
static int16_t lastAuxVal = 0;
static boolean dShotEnabled = false;
static boolean logoDone = false;
extern void ReadFCSettings(boolean skipValues);

typedef void* (*fptr)();
static char tempSymbol[] = {0xB0, 0x00};
static uint8_t code = 0;
static boolean menuActive = false;
static boolean menuWasActive = false;
static boolean fcSettingChanged = false;
static void* activePage = NULL;
static boolean batterySelect = false;
static boolean triggerCleanScreen = false;
static boolean bat_clear = false;
static uint8_t activeMenuItem = 0;
static uint8_t stopWatch = 0x94;
static char batterySymbol[] = { 0xE7, 0xEC, 0xEC, 0xED, 0x00 };
static uint8_t krSymbol[4] = { 0x9C, 0x9C, 0x9C, 0x9C };
static unsigned long krTime[4] = { 0, 0, 0, 0 };

static int16_t bufminus1 = 0;
static int16_t checkCalced = 0;

#ifdef DEBUG
static int16_t versionProto = 0;
#endif

static unsigned long _StartupTime = 0;

void loop(){
  uint16_t i = 0;
  static uint8_t blink_i = 1;
  
  if(!OSD.status()) //trying to revive MAX7456 if it got a voltage hickup
  {
    ReviveOSD();
  }
  
  if(_StartupTime == 0)
  {
    _StartupTime = millis();
  }
  
  if(micros()-LastLoopTime > 10000)
  {
    LastLoopTime = micros();
    blink_i++;
    if (blink_i > 100){
      blink_i = 1;
    }
    
    if(!fcSettingsReceived)
    {
      NewSerial.write(0x30); // request settings
      ReadFCSettings(fcSettingChanged);
      #ifdef PROTODEBUG
      if(checkCalced != bufminus1)
      {
        OSD.printInt16(8, -4, checkCalced, 0, 1);
        OSD.printInt16(8, -3, bufminus1, 0, 1);
      }
      #endif
      return;
    }
    else
    {
      if(!fcSettingChanged || menuActive)
      {
        NewSerial.write(0x20); // request telemetry
      }
      if(!ReadTelemetry()) 
      {
        if(fcSettingChanged && !menuActive)
        {
          SendFCSettings();
          fcSettingChanged = false;
        }
        #ifdef DEBUG
        /*static char Aux1[15];
        OSD.setCursor(8,-3);
        OSD.print(fixStr("tele"));*/
        #endif 
        return;
      }
    }    
  
    while (!OSD.notInVSync());

      #ifdef DEBUG
      OSD.printInt16(8,-3,versionProto,0,1);
      #endif 
      
      if(triggerCleanScreen || abs((lastAuxVal+1000) - (AuxChanVals[settings.m_DVchannel]+1000)) > DV_PPM_INCREMENT)
      {
        lastAuxVal = AuxChanVals[settings.m_DVchannel];
        triggerCleanScreen = false;
        cleanScreen();
      }
      
      if(settings.m_tempUnit == 1)
      {
        tempSymbol[0] = fixChar(0xB1);
      }
      else
      {
        tempSymbol[0] = fixChar(0xB0);
      }
      
      code = inputChecker.ProcessStickInputs(roll, pitch, yaw, armed);
      
      if(armed == 0 && code & inputChecker.YAW_LONG_LEFT && code & inputChecker.ROLL_LEFT && code & inputChecker.PITCH_DOWN)
      {
        ReviveOSD();
      }
      
      if(armed == 0 && settings.m_lastMAH > 0)
      {
        FLASH_STRING(LAST_BATTERY_STR, "continue last battery?");
        OSD.printFS(COLS/2 - (LAST_BATTERY_STR.length())/2, ROWS/2 - 1, &LAST_BATTERY_STR);
        
        FLASH_STRING(ROLL_RIGHT_STR, "roll right to confirm");
        OSD.printFS(COLS/2 - ROLL_RIGHT_STR.length()/2, ROWS/2, &ROLL_RIGHT_STR);
        FLASH_STRING(ARM_CANCEL_STR, "roll left to cancel");
        OSD.printFS(COLS/2 - ARM_CANCEL_STR.length()/2, ROWS/2 + 1, &ARM_CANCEL_STR);
                
        if(code & inputChecker.ROLL_RIGHT)
        {
          previousMAH = settings.m_lastMAH;
          settings.m_lastMAH = 0;
          cleanScreen();
        }
        if(code & inputChecker.ROLL_LEFT)
        {
          settings.m_lastMAH = 0;
          settings.WriteLastMAH();
          cleanScreen();
        }  
        return;
      }
      
      if(batterySelect || (!menuActive && !armOnYaw && yaw > 1750 && armed == 0))
      {
        if(!showBat)
        {
          cleanScreen();
          showBat = true;
        }
        if((code & inputChecker.ROLL_LEFT) && settings.m_activeBattery > 0)
        {
          settings.m_activeBattery--;
          settings.FixBatWarning();
          settingChanged = true;
        }
        if((code & inputChecker.ROLL_RIGHT) && settings.m_activeBattery < 3)
        {
          settings.m_activeBattery++;
          settings.FixBatWarning();
          settingChanged = true;
        }
        if((code & inputChecker.PITCH_UP) && settings.m_batMAH[settings.m_activeBattery] < 32000)
        {
          settings.m_batMAH[settings.m_activeBattery] += BAT_MAH_INCREMENT;
          settings.FixBatWarning();
          settingChanged = true;
        }
        if((code & inputChecker.PITCH_DOWN) && settings.m_batMAH[settings.m_activeBattery] > 300)
        {
          settings.m_batMAH[settings.m_activeBattery] -= BAT_MAH_INCREMENT;
          settings.FixBatWarning();
          settingChanged = true;
        }
        FLASH_STRING(BATTERY_STR, "battery ");
        OSD.printInt16(COLS/2 - (BATTERY_STR.length()+1)/2, ROWS/2 - 1, &BATTERY_STR, settings.m_activeBattery+1, 0,1);        
       
        OSD.printInt16(COLS/2 - 3, ROWS/2, settings.m_batMAH[settings.m_activeBattery], 0, 1, "mah", true);

        FLASH_STRING(WARN_STR, "warn at ");
        OSD.printInt16(COLS/2 - (WARN_STR.length() + 6)/2, ROWS/2 + 1, &WARN_STR, settings.m_batWarningMAH, 0, 1, "mah", true);

        if(batterySelect)
        {
          FLASH_STRING(YAW_LEFT_STR, "yaw left to go back");
          OSD.printFS(COLS/2 - YAW_LEFT_STR.length()/2,ROWS/2 + 2, &YAW_LEFT_STR);
        }
        if(batterySelect && yaw < 250)
        {
          batterySelect = false;
        }
        return;
      }
      else
      {
        if(showBat)
        {
          cleanScreen();
          showBat = false;
        }
      }
      
      if(armed == 0 && ((code &  inputChecker.YAW_LONG_LEFT) || menuActive))
      {
        if(!menuActive)
        {
          menuActive = true;
          cleanScreen();
          activePage = (void*)MainMenu;
        }
        fptr temp = (fptr) activePage;
        activePage = (void*)temp();
        return;
      }
      else
      {
        if(menuWasActive)
        {
          menuWasActive = false;
          activeMenuItem = 0;
          cleanScreen();
        }        
      }
      
      if(settingChanged)
      {
        settings.WriteSettings();
      }
      settingChanged = false;
      if(armed == 0 && armedOnce && last_Aux_Val != AuxChanVals[settings.m_DVchannel]) 
      {
        DV_change_time = millis();
        last_Aux_Val = AuxChanVals[settings.m_DVchannel];
      }
      if(DV_change_time == 0 && armed == 0 && AuxChanVals[settings.m_DVchannel] > DISPLAY_STATS_DV && armedOnce) 
      {
        if(code & inputChecker.PITCH_UP && statPage > 0)
        {
          statPage--;
          cleanScreen();
        }
        if(code & inputChecker.PITCH_DOWN && statPage < 4)
        {
          statPage++;
          cleanScreen();
        }
        uint8_t middle_infos_y     = 2;    
        FLASH_STRING(STATS_STR, "stats ");
        OSD.printInt16( COLS/2 - (STATS_STR.length()+4)/2, middle_infos_y, &STATS_STR, statPage+1, 0, 1);
        OSD.print( fixStr("/5") );
        middle_infos_y++;
        
        if(statPage == 0)
        {
          FLASH_STRING(TIME_STR,     "time     : ");
          FLASH_STRING(MAX_AMP_STR,  "max amps : ");
          FLASH_STRING(MAX_C_STR,    "max c    : ");
          FLASH_STRING(MAH_STR,      "mah      : ");
          FLASH_STRING(MAX_RPM_STR,  "max rpm  : ");
          FLASH_STRING(MAX_WATT_STR, "max watt : ");
          FLASH_STRING(MAX_TEMP_STR, "max temp : ");
          FLASH_STRING(MIN_V_STR,    "min v    : ");

          uint8_t statCol = COLS/2 - (TIME_STR.length()+7)/2;
          OSD.printFS(statCol, ++middle_infos_y, &TIME_STR);
          OSD.printTime( statCol+TIME_STR.length(), middle_infos_y, total_time );  
          OSD.printInt16( statCol, ++middle_infos_y, &MAX_AMP_STR, MaxAmps, 1, 1, "a" );           
          OSD.printInt16( statCol, ++middle_infos_y, &MAX_C_STR, MaxC, 0, 1, "c");
          OSD.printInt16( statCol, ++middle_infos_y, &MAH_STR, LipoMAH+previousMAH, 0, 1, "mah" );
          OSD.printInt16( statCol, ++middle_infos_y, &MAX_RPM_STR, MaxRPMs, 1, 1, "kr" );  
          OSD.printInt16( statCol, ++middle_infos_y, &MAX_WATT_STR, MaxWatt, 1, 1, "w" );
          OSD.printInt16( statCol, ++middle_infos_y, &MAX_TEMP_STR, MaxTemp, 0, 1, tempSymbol);
          OSD.printInt16( statCol, ++middle_infos_y, &MIN_V_STR, MinBat, 2, 1, "v" );          
        }
        else
        {
          FLASH_STRING(ESC_STAT_STR,"esc");
          FLASH_STRING(ESC_RPM_STR, " max rpm : ");
          FLASH_STRING(ESC_A_STR,   " max a   : ");
          FLASH_STRING(ESC_TEMP_STR," max temp: ");
          FLASH_STRING(ESC_MINV_STR," min v   : ");
          FLASH_STRING(ESC_MAH_STR, " mah     : ");
          
          uint8_t startCol = COLS/2 - (ESC_RPM_STR.length()+ESC_STAT_STR.length()+7)/2;
          OSD.printInt16( startCol, ++middle_infos_y, &ESC_STAT_STR, statPage, 0, 1);
          OSD.printInt16( startCol + ESC_STAT_STR.length() + 1, middle_infos_y, &ESC_RPM_STR, maxKERPM[statPage-1], 1, 1, "kr");
          
          OSD.printInt16( startCol, ++middle_infos_y, &ESC_STAT_STR, statPage, 0, 1);
          OSD.printInt16( startCol + ESC_STAT_STR.length() + 1, middle_infos_y, &ESC_A_STR, maxCurrent[statPage-1], 2, 1, "a"); 
          
          OSD.printInt16( startCol, ++middle_infos_y, &ESC_STAT_STR, statPage, 0, 1);
          OSD.printInt16( startCol + ESC_STAT_STR.length() + 1, middle_infos_y, &ESC_TEMP_STR, maxTemps[statPage-1], 0, 1, tempSymbol);
          
          OSD.printInt16( startCol, ++middle_infos_y, &ESC_STAT_STR, statPage, 0, 1);
          OSD.printInt16( startCol + ESC_STAT_STR.length() + 1, middle_infos_y, &ESC_MINV_STR, minVoltage[statPage-1], 2, 1, "v");
          
          OSD.printInt16( startCol, ++middle_infos_y, &ESC_STAT_STR, statPage, 0, 1);
          OSD.printInt16( startCol + ESC_STAT_STR.length() + 1, middle_infos_y, &ESC_MAH_STR, ESCmAh[statPage-1], 0, 1, "mah"); 
        }
     }
     else 
      {
        uint8_t ESCmarginBot       = 0;
        uint8_t ESCmarginTop       = 0;
        uint8_t TMPmargin          = 0;
        uint8_t CurrentMargin      = 0;
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_RC_THROTTLE_DV)
        {
          OSD.printInt16( 0, 0, throttle, 0, 1, "%", true);
          ESCmarginTop = 1;
        }
          
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_NICKNAME_DV)
        {
          OSD.setCursor( COLS/2 - strlen(NICKNAME)/2 - 1, -1 );
          OSD.print( fixStr(NICKNAME) );
        }
    
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_COMB_CURRENT_DV)
        {
          OSD.printAligned(0, current, 1, 0, "a");
          ESCmarginTop = 1;
        }
        
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_LIPO_VOLTAGE_DV)
        {
          OSD.printInt16( 0, -1, LipoVoltage / 10, 1, 1, "v", true);
          ESCmarginBot = 1;
        }
        
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_MA_CONSUMPTION_DV)
        {
          if(settings.m_displaySymbols == 1)
          {
            uint8_t batCount = (LipoMAH+previousMAH) / settings.m_batSlice;
            uint8_t batStatus = 0xEC;
            while(batCount > 3)
            {
              batStatus--;
              batCount--;
            }
            batterySymbol[2] = (char)batStatus;
            batStatus = 0xEC;
            while(batCount > 0)
            {
              batStatus--;
              batCount--;
            }
            batterySymbol[1] = (char)batStatus;
            OSD.setCursor(-7, -1);
            OSD.print(batterySymbol);
          }
          else
          {
            OSD.printAligned(-1, LipoMAH+previousMAH, 0, 1, "mah");
          }
          ESCmarginBot = 1;
        }
        
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_ESC_KRPM_DV)
        {
          static char KR[3][4];
          if(settings.m_displaySymbols == 1)
          {
            for(i=0; i<4; i++)
            {
              if(millis() - krTime[i] > (10000/motorKERPM[i]))
              {
                krTime[i] = millis();
                krSymbol[i]++;
                if(krSymbol[i] > 0xA3)
                {
                  krSymbol[i] = 0x9C;
                }
              }
              KR[i][0] = (char)krSymbol[i];
              KR[i][1] = ' ';
            }
          }
          else
          {
            for(i=0; i<4; i++)
            {
              KR[i][0] = 'k';
              KR[i][1] = 'r';
              KR[i][2] = 0x00;
            }
          }
          OSD.printInt16(0, ESCmarginTop, motorKERPM[0], 1, 1, KR[0], true);
          OSD.printAligned(ESCmarginTop, motorKERPM[1], 1, 0, KR[1]);
          OSD.printAligned(-(1+ESCmarginBot), motorKERPM[2], 1, 0, KR[2]);
          OSD.printInt16( 0, -(1+ESCmarginBot), motorKERPM[3], 1, 1, KR[3], true);
          
          TMPmargin++;
          CurrentMargin++;
        }
     
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_ESC_CURRENT_DV)
        {
          OSD.printInt16( 0, CurrentMargin+ESCmarginTop, motorCurrent[0], 2, 1, "a", true);
          OSD.printAligned(CurrentMargin+ESCmarginTop, motorCurrent[1], 2, 0, "a");
          OSD.printAligned(-(1+CurrentMargin+ESCmarginBot), motorCurrent[2], 2, 0, "a" );
          OSD.printInt16( 0, -(1+CurrentMargin+ESCmarginBot), motorCurrent[3], 2 ,1, "a", true );
          TMPmargin++;
        }
    
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_ESC_TEMPERATURE_DV)
        {
          OSD.printInt16( 0, TMPmargin+ESCmarginTop, ESCTemps[0], 0, 1, tempSymbol, true);
          OSD.printAligned(TMPmargin+ESCmarginTop, ESCTemps[1], 0, 0, tempSymbol );
          OSD.printAligned(-(1+TMPmargin+ESCmarginBot), ESCTemps[2], 0, 0, tempSymbol );
          OSD.printInt16( 0, -(1+TMPmargin+ESCmarginBot), ESCTemps[3], 0, 1, tempSymbol, true );
        }
    
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_TIMER_DV) 
        {
          #ifdef NICE_FONT
          if(settings.m_displaySymbols == 1)
          {
            if(time - old_time > 1000)
            {
              stopWatch++;
              old_time = time;
            }
            if(stopWatch > 0x9B)
            {
              stopWatch = 0x94;
            }
            OSD.setCursor(COLS/2 - 4, -2);
            OSD.print((char)stopWatch);
          }
          #endif
          OSD.printTime(COLS/2 - 3, -2, time);
        }
        
        if(settings.m_batWarning > 0 && (LipoMAH+previousMAH) >= settings.m_batWarningMAH)
        {
          totalMAH = 0;
          FLASH_STRING(BATTERY_LOW,   "battery low");
          FLASH_STRING(BATTERY_EMPTY, "           ");
          if (blink_i % 20 == 0) 
          {
            if(bat_clear) 
            {
              if(settings.m_displaySymbols == 1)
              {
                OSD.setCursor(COLS/2 - 3, ROWS/2 + 3);
                OSD.print(batterySymbol);
              }
              else
              {
                OSD.printFS(COLS/2 - BATTERY_LOW.length()/2, ROWS/2 +3, &BATTERY_LOW);
              }
              bat_clear = false;
            }
            else 
            {              
              OSD.printFS(COLS/2 - BATTERY_LOW.length()/2, ROWS/2 +3, &BATTERY_EMPTY);
              bat_clear = true;
            }
          }
        }
        else
        {
          if(settings.m_batWarning > 0)
          {
            totalMAH = LipoMAH+previousMAH;
          }
          else
          {
            totalMAH = 0;
          }
        }
        if(DV_change_time > 0 && (millis() - DV_change_time) > 3000 && last_Aux_Val == AuxChanVals[settings.m_DVchannel]) 
        {
          DV_change_time = 0;
          cleanScreen();
        }
        if(!logoDone && armed == 0 && !menuActive && !armedOnce)
        {
          uint8_t logoCol = 11;
          uint8_t logoRow = 5;
          OSD.setCursor(logoCol, logoRow);
          uint8_t logoPos = 0xB4;
          for(i=0; i<7; i++)
          {
            OSD.print((char)logoPos++);
          }
          logoRow++;
          logoCol--;
          OSD.setCursor(logoCol, logoRow);
          for(i=0; i<8; i++)
          {
            OSD.print((char)logoPos++);
          }
          logoRow++;
          logoCol -= 2;
          OSD.setCursor(logoCol, logoRow);
          for(i=0; i<4; i++)
          {
            OSD.print((char)logoPos++);
          }
          if(dShotEnabled)
          {
            for(i=0; i<8; i++)
            {
              OSD.print((char)logoPos++);
            }
            logoRow++;
            logoCol += 4;
            OSD.setCursor(logoCol, logoRow);
            for(i=0; i<8; i++)
            {
              OSD.print((char)logoPos++);
            }
          }
          if(_StartupTime > 0 && millis() - _StartupTime > 60000)
          {
            logoDone = true;
            cleanScreen();
          }
        }
      }
  }    
}

#ifdef DEBUG
int freeRam() {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
#endif
