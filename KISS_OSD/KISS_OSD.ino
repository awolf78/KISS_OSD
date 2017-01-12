/*
KISS FC OSD v1.0
By Felix Niessen (felix.niessen@googlemail.com)
for Flyduino.net

KISS FC OSD v2.x
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

// motors magnepole count (to display the right RPMs)
//=============================
#define MAGNETPOLECOUNT 14 // 2 for ERPMs

// Filter for ESC datas (higher value makes them less erratic) 0 = no filter, 20 = very strong filter
//=============================
#define ESC_FILTER 10

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

const char KISS_OSD_VER[] = "kiss osd v2.2.1";

#include "Flash.h"
#include <SPI.h>
#include "MAX7456.h"
#include "MyMAX7456.h"
#include <EEPROM.h>
#include "SerialPort.h"
#include "CSettings.h"
#include "CStickInput.h"
#include "fixFont.h"
#include "CMeanFilter.h"
#include "Config.h"

#ifdef IMPULSERC_VTX
const uint8_t osdChipSelect          =            10;
#else
const uint8_t osdChipSelect          =            6;
#endif
const byte masterOutSlaveIn          =            MOSI;
const byte masterInSlaveOut          =            MISO;
const byte slaveClock                =            SCK;
const byte osdReset                  =            2;


CMyMax7456 OSD( osdChipSelect );
SerialPort<0, 64, 0> NewSerial;
CStickInput inputChecker;
CSettings settings;
static int16_t DV_PPMs[CSettings::DISPLAY_DV_SIZE];

#ifdef IMPULSERC_VTX
static volatile uint8_t vTxChannel, vTxBand, vTxPower, oldvTxChannel, oldvTxBand;
const uint8_t VTX_BAND_COUNT = 5;
const uint8_t VTX_CHANNEL_COUNT = 8;
//static unsigned long changevTxTime = 0;
static const char bandSymbols[][2] = { {'a',0x00} , {'b', 0x00}, {'e', 0x00}, {'f', 0x00}, {'r', 0x00}};

const uint16_t vtx_frequencies[VTX_BAND_COUNT][VTX_CHANNEL_COUNT] PROGMEM = {
    { 5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725 }, //A
    { 5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866 }, //B
    { 5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945 }, //E
    { 5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880 }, //F
    { 5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917 }  //R
  };

void setvTxSettings()
{
  oldvTxChannel = vTxChannel = settings.m_vTxChannel;
  oldvTxBand = vTxBand =  settings.m_vTxBand;
  vTxPower = settings.m_vTxPower;
}
#endif

void cleanScreen() 
{
  OSD.clear();
  while(!OSD.clearIsBusy());
}

static uint8_t lastTempUnit;

void setupMAX7456()
{
  OSD.begin(settings.COLS,13);
  delay(100);
  settings.m_videoMode = OSD.videoSystem();
  if(settings.m_videoMode == MAX7456_PAL) settings.ROWS = 15;
  else settings.ROWS = 13;
  OSD.setTextArea(settings.COLS, settings.ROWS);  
  OSD.setDefaultSystem(settings.m_videoMode);
  OSD.setSwitchingTime( 5 );   
  OSD.setCharEncoding( MAX7456_ASCII );  
  OSD.display();
#ifdef IMPULSERC_VTX
  delay(100);
  MAX7456Setup();
#endif
  delay(100);
  OSD.setTextOffset(settings.m_xOffset, settings.m_yOffset); 
}

void setup()
{
  uint8_t i = 0;
  SPI.begin();
  SPI.setClockDivider( SPI_CLOCK_DIV2 ); 
  setupMAX7456();
  settings.cleanEEPROM();
  settings.ReadSettings(); 
  OSD.setTextOffset(settings.m_xOffset, settings.m_yOffset); 
  
  //clean used area
  while (!OSD.notInVSync());
  cleanScreen();
#ifdef IMPULSERC_VTX
  //Ignore setting because this is critical to making sure we can detect the
  //VTX power jumper being installed. If we aren't using 5V ref there is
  //the chance we will power up on wrong frequency.
  analogReference(DEFAULT);
  setvTxSettings();
  vtx_init();
  vtx_set_frequency(vTxBand, vTxChannel);
  vtx_flash_led(5);
#endif
  lastTempUnit = settings.m_tempUnit;
  settings.SetupPPMs(DV_PPMs);
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
static int16_t lpf_frq, minCommand, minThrottle;
static uint8_t notchFilterEnabled = 2;
static int16_t notchFilterCenter, notchFilterCut;
static int16_t yawFilterCut;
static boolean fcSettingsReceived = false;
static boolean armOnYaw = true;
static uint32_t LastLoopTime = 0;
static boolean dShotEnabled = false;
static boolean logoDone = false;
extern void ReadFCSettings(boolean skipValues);

typedef void* (*fptr)();
static char tempSymbol[] = {0xB0, 0x00};
static char ESCSymbol[] = {0x7E, 0x00};
static uint8_t code = 0;
static boolean menuActive = false;
static boolean menuWasActive = false;
static boolean fcSettingChanged = false;
static void* activePage = NULL;
static boolean batterySelect = false;
static boolean triggerCleanScreen = true;
static uint8_t activeMenuItem = 0;
static uint8_t stopWatch = 0x94;
static char batterySymbol[] = { 0xE7, 0xEC, 0xEC, 0xED, 0x00 };
static uint8_t krSymbol[4] = { 0x9C, 0x9C, 0x9C, 0x9C };
static unsigned long krTime[4] = { 0, 0, 0, 0 };
volatile bool timer1sec = false;
static unsigned long timer1secTime = 0;
static uint8_t currentDVItem = 0;
static bool symbolOnOffChanged = false;
static int16_t bufminus1 = 0;
static int16_t checkCalced = 0;
static uint16_t fcNotConnectedCount = 0;
static bool telemetryReceived = false;

#ifdef DEBUG
static int16_t versionProto = 0;
#endif

static unsigned long _StartupTime = 0;
extern void* MainMenu();

void loop(){
  uint16_t i = 0;
  static uint8_t blink_i = 1;

#ifdef IMPULSERC_VTX
  /*if(oldvTxBand != vTxBand || oldvTxChannel != vTxChannel || changevTxTime > 0)
  {
    if(changevTxTime == 0)
    {
      changevTxTime = millis();
      oldvTxBand = vTxBand;
      oldvTxChannel = vTxChannel;
    }
  }
  else
  {*/
    vtx_process_state(millis(), vTxBand, vTxChannel);
  //}
#endif
  
  if(!OSD.status()) //trying to revive MAX7456 if it got a voltage hickup
  {
    ReviveOSD();
  }
  
  if(_StartupTime == 0)
  {
    _StartupTime = millis();
  }

  if((millis()-timer1secTime) > 500)
  {
    timer1secTime = millis();
    timer1sec = !timer1sec;
  }
  
  if(micros()-LastLoopTime > 10000)
  { 
    LastLoopTime = micros();
    
#ifdef IMPULSERC_VTX
     if (timer1sec) 
     {
        vtx_set_power(armed ? vTxPower : 0);
     }
#endif
    
    if(!fcSettingsReceived)
    {
      NewSerial.write(0x30); // request settings
      ReadFCSettings(fcSettingChanged);
      if(!fcSettingsReceived) fcNotConnectedCount++;
      else fcNotConnectedCount = 0;
      #ifdef PROTODEBUG
      if(checkCalced != bufminus1)
      {
        OSD.printInt16(8, -4, checkCalced, 0, 1);
        OSD.printInt16(8, -3, bufminus1, 0, 1);
      }
      #endif
    }
    else
    {
      if(!fcSettingChanged || menuActive)
      {
        NewSerial.write(0x20); // request telemetry
      }
      telemetryReceived = ReadTelemetry();
      if(!telemetryReceived) 
      {
        if(fcSettingChanged && !menuActive)
        {
          SendFCSettings();
          fcSettingChanged = false;
        }
        else fcNotConnectedCount++;
        #ifdef DEBUG
        /*static char Aux1[15];
        OSD.setCursor(8,-3);
        OSD.print(fixStr("tele"));*/
        #endif 
      }
      else fcNotConnectedCount = 0;
    }

    if(fcNotConnectedCount <= 500 && (!fcSettingsReceived || !telemetryReceived)) return;

    while (!OSD.notInVSync());

    if(fcNotConnectedCount > 500)
    {
      FLASH_STRING(FC_NOT_CONNECTED_STR, "no connection to kiss fc");
      OSD.printFS(settings.COLS/2 - FC_NOT_CONNECTED_STR.length()/2, settings.ROWS/2, &FC_NOT_CONNECTED_STR);
      triggerCleanScreen = true;
      fcNotConnectedCount = 0;
      return;
    }

#ifdef IMPULSERC_VTX
      /*if(changevTxTime > 0)
      {
        FLASH_STRING(CHANGE_CHANNELS_STR, "changing vtx to ");
        OSD.printFS(settings.COLS/2 - (CHANGE_CHANNELS_STR.length()/2 + 5), 8, &CHANGE_CHANNELS_STR);
        OSD.printInt16(settings.COLS/2 - (CHANGE_CHANNELS_STR.length()/2 + 5) + CHANGE_CHANNELS_STR.length() + 1, 8, bandSymbols[vTxBand], (int16_t)vTxChannel, 0, 1, "=");
        OSD.printInt16(settings.COLS/2 - (CHANGE_CHANNELS_STR.length()/2 + 5) + CHANGE_CHANNELS_STR.length() + 5, 8, (int16_t)pgm_read_word(&vtx_frequencies[settings.m_vTxBand][settings.m_vTxChannel]), 0, 1, "mhz");
        uint8_t timeLeft = (uint8_t)((6000 - (millis() - changevTxTime))/1000);   
        OSD.printInt16(settings.COLS/2 - 6, 9, "in ", (int16_t)timeLeft, 0, 1, " seconds");
        if(timeLeft < 1)
        {
          changevTxTime = 0;
          cleanScreen();
        }
      }*/
      vtx_flash_led(1);
#endif

      #ifdef DEBUG
      OSD.printInt16(8,-3,versionProto,0,1);
      #endif 
      
      if(triggerCleanScreen || (abs((DV_PPMs[currentDVItem]+1000) - (AuxChanVals[settings.m_DVchannel]+1000)) >= CSettings::DV_PPM_INCREMENT && (AuxChanVals[settings.m_DVchannel]+1000) < (CSettings::DV_PPM_INCREMENT*(CSettings::DISPLAY_DV_SIZE))))
      {
        currentDVItem = CSettings::DISPLAY_DV_SIZE-1;
        while(abs((DV_PPMs[currentDVItem]+1000) - (AuxChanVals[settings.m_DVchannel]+1000)) >= CSettings::DV_PPM_INCREMENT && currentDVItem > 0) currentDVItem--;
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
        OSD.printFS(settings.COLS/2 - (LAST_BATTERY_STR.length())/2, settings.ROWS/2 - 1, &LAST_BATTERY_STR);
        
        FLASH_STRING(ROLL_RIGHT_STR, "roll right to confirm");
        OSD.printFS(settings.COLS/2 - ROLL_RIGHT_STR.length()/2, settings.ROWS/2, &ROLL_RIGHT_STR);
        FLASH_STRING(ARM_CANCEL_STR, "roll left to cancel");
        OSD.printFS(settings.COLS/2 - ARM_CANCEL_STR.length()/2, settings.ROWS/2 + 1, &ARM_CANCEL_STR);
                
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
      
      if(batterySelect || (!menuActive && !armOnYaw && yaw > 1900 && armed == 0))
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
        uint8_t batCol = settings.COLS/2 - (BATTERY_STR.length()+1)/2;
        OSD.printInt16(batCol, settings.ROWS/2 - 1, &BATTERY_STR, settings.m_activeBattery+1, 0,1);        
        batCol = settings.COLS/2 - 3;
        OSD.printInt16(batCol, settings.ROWS/2, settings.m_batMAH[settings.m_activeBattery], 0, 1, "mah", 2);

        FLASH_STRING(WARN_STR, "warn at ");
        OSD.printInt16(settings.COLS/2 - (WARN_STR.length() + 6)/2, settings.ROWS/2 + 1, &WARN_STR, settings.m_batWarningMAH, 0, 1, "mah", 2);

        if(batterySelect)
        {
          FLASH_STRING(YAW_LEFT_STR, "yaw left to go back");
          OSD.printFS(settings.COLS/2 - YAW_LEFT_STR.length()/2,settings.ROWS/2 + 2, &YAW_LEFT_STR);
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
        settingChanged = false;
      }
      
      if(armed == 0 && armedOnce && last_Aux_Val != AuxChanVals[settings.m_DVchannel]) 
      {
        DV_change_time = millis();
        last_Aux_Val = AuxChanVals[settings.m_DVchannel];
      }
      
      if(DV_change_time == 0 && armed == 0 && armedOnce) 
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
        OSD.printInt16( settings.COLS/2 - (STATS_STR.length()+4)/2, middle_infos_y, &STATS_STR, statPage+1, 0, 1);
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

          uint8_t statCol = settings.COLS/2 - (TIME_STR.length()+7)/2;
          OSD.printFS(statCol, ++middle_infos_y, &TIME_STR);
          OSD.printTime( statCol+TIME_STR.length(), middle_infos_y, total_time);  
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
          static char ESC_STAT_STR1[] = "esc";
          char* ESC_STAT_STR = ESC_STAT_STR1;
          if(settings.m_displaySymbols == 1)
          {
            ESC_STAT_STR = ESCSymbol;
          }
          FLASH_STRING(ESC_RPM_STR, " max rpm : ");
          FLASH_STRING(ESC_A_STR,   " max a   : ");
          FLASH_STRING(ESC_TEMP_STR," max temp: ");
          FLASH_STRING(ESC_MINV_STR," min v   : ");
          FLASH_STRING(ESC_MAH_STR, " mah     : ");
          
          uint8_t startCol = settings.COLS/2 - (ESC_RPM_STR.length()+strlen(ESC_STAT_STR)+7)/2;
          OSD.printInt16( startCol, ++middle_infos_y, ESC_STAT_STR, statPage, 0, 1);
          OSD.printInt16( startCol + strlen(ESC_STAT_STR) + 1, middle_infos_y, &ESC_RPM_STR, maxKERPM[statPage-1], 1, 1, "kr");
          
          OSD.printInt16( startCol, ++middle_infos_y, ESC_STAT_STR, statPage, 0, 1);
          OSD.printInt16( startCol + strlen(ESC_STAT_STR) + 1, middle_infos_y, &ESC_A_STR, maxCurrent[statPage-1], 2, 1, "a"); 
          
          OSD.printInt16( startCol, ++middle_infos_y, ESC_STAT_STR, statPage, 0, 1);
          OSD.printInt16( startCol + strlen(ESC_STAT_STR) + 1, middle_infos_y, &ESC_TEMP_STR, maxTemps[statPage-1], 0, 1, tempSymbol);
          
          OSD.printInt16( startCol, ++middle_infos_y, ESC_STAT_STR, statPage, 0, 1);
          OSD.printInt16( startCol + strlen(ESC_STAT_STR) + 1, middle_infos_y, &ESC_MINV_STR, minVoltage[statPage-1], 2, 1, "v");
          
          OSD.printInt16( startCol, ++middle_infos_y, ESC_STAT_STR, statPage, 0, 1);
          OSD.printInt16( startCol + strlen(ESC_STAT_STR) + 1, middle_infos_y, &ESC_MAH_STR, ESCmAh[statPage-1], 0, 1, "mah"); 
        }
     }
     else 
      {
        if(armed == 0 && armedOnce && last_Aux_Val != AuxChanVals[settings.m_DVchannel]) 
        {
          DV_change_time = millis();
          last_Aux_Val = AuxChanVals[settings.m_DVchannel];
        }
        
        uint8_t TMPmargin          = 0;
        uint8_t CurrentMargin      = 0;
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_RC_THROTTLE])
        {
          OSD.printInt16( settings.m_OSDItems[THROTTLE][0], settings.m_OSDItems[THROTTLE][1], throttle, 0, 1, "%", 2, THROTTLEp);
        }
          
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_NICKNAME])
        {
          uint8_t nickBlanks = 0;
          OSD.checkPrintLength(settings.m_OSDItems[NICKNAME][0], settings.m_OSDItems[NICKNAME][1], strlen(settings.m_nickname), nickBlanks, NICKNAMEp);
          OSD.print( fixStr(settings.m_nickname) );          
        }
    
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_COMB_CURRENT])
        {          
          OSD.printInt16(settings.m_OSDItems[AMPS][0], settings.m_OSDItems[AMPS][1], current, 1, 0, "a", 2, AMPSp);
        }
        
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_LIPO_VOLTAGE])
        {
          OSD.printInt16( settings.m_OSDItems[VOLTAGE][0], settings.m_OSDItems[VOLTAGE][1], LipoVoltage / 10, 1, 1, "v", 1, VOLTAGEp);
        }
        
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_MA_CONSUMPTION])
        {        
          if(settings.m_displaySymbols == 1)
          {
            uint8_t batCount = (LipoMAH+previousMAH) / settings.m_batSlice;
            uint8_t batStatus = 0xEC;
            while(batCount > 4)
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
            uint8_t mahBlanks = 0;
            OSD.checkPrintLength(settings.m_OSDItems[MAH][0], settings.m_OSDItems[MAH][1], 4, mahBlanks, MAHp);
            OSD.print(batterySymbol);            
          }
          else
          {
            OSD.printInt16(settings.m_OSDItems[MAH][0], settings.m_OSDItems[MAH][1], LipoMAH+previousMAH, 0, 1, "mah", MAHp);
          }
        }
  
        if(settings.m_displaySymbols == 1)
        {
          ESCSymbol[0] = fixChar((char)0x7E);
        }
        else
        {
          ESCSymbol[0] = 0x00;
        }
        
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_ESC_KRPM])
        {
          static char KR[4];
          if(settings.m_displaySymbols == 1)
          {
            for(i=0; i<4; i++)
            {
              if(millis() - krTime[i] > (10000/motorKERPM[i]))
              {
                krTime[i] = millis();
                if((i+1)%2 == 0)
                {
                  krSymbol[i]--;
                }
                else
                {
                  krSymbol[i]++;
                }
                if(krSymbol[i] > 0xA3)
                {
                  krSymbol[i] = 0x9C;
                }
                if(krSymbol[i] < 0x9C)
                {
                  krSymbol[i] = 0xA3;
                }
              }
              KR[i] = (char)krSymbol[i];
            }
            uint8_t ESCkrBlanks = 0;
            OSD.checkPrintLength(settings.m_OSDItems[ESC1kr][0], settings.m_OSDItems[ESC1kr][1], 1, ESCkrBlanks, ESC1kr);
            OSD.print(KR[0]);
            OSD.checkPrintLength(settings.m_OSDItems[ESC2kr][0], settings.m_OSDItems[ESC2kr][1], 1, ESCkrBlanks, ESC2kr);
            OSD.print(KR[1]);
            OSD.checkPrintLength(settings.m_OSDItems[ESC3kr][0], settings.m_OSDItems[ESC3kr][1], 1, ESCkrBlanks, ESC3kr);
            OSD.print(KR[2]);
            OSD.checkPrintLength(settings.m_OSDItems[ESC4kr][0], settings.m_OSDItems[ESC4kr][1], 1, ESCkrBlanks, ESC4kr);
            OSD.print(KR[3]);
          }
          else
          {
            static char KR2[3];
            KR2[0] = 'k';
            KR2[1] = 'r';
            KR2[2] = 0x00;
            OSD.printInt16(settings.m_OSDItems[ESC1kr][0], settings.m_OSDItems[ESC1kr][1], motorKERPM[0], 1, 1, KR2, 1, ESC1kr);
            OSD.printInt16(settings.m_OSDItems[ESC2kr][0], settings.m_OSDItems[ESC2kr][1], motorKERPM[1], 1, 0, KR2, 1, ESC2kr);
            OSD.printInt16(settings.m_OSDItems[ESC3kr][0], settings.m_OSDItems[ESC3kr][1], motorKERPM[2], 1, 0, KR2, 1, ESC3kr);
            OSD.printInt16(settings.m_OSDItems[ESC4kr][0], settings.m_OSDItems[ESC4kr][1], motorKERPM[3], 1, 1, KR2, 1, ESC4kr);
          }
          
          TMPmargin++;
          CurrentMargin++;
        }
     
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_ESC_CURRENT])
        {
          OSD.printInt16(settings.m_OSDItems[ESC1voltage][0], settings.m_OSDItems[ESC1voltage][1]+CurrentMargin, motorCurrent[0], 2, 1, "a", 1, ESC1voltage, ESCSymbol);
          OSD.printInt16(settings.m_OSDItems[ESC2voltage][0], settings.m_OSDItems[ESC2voltage][1]+CurrentMargin, motorCurrent[1], 2, 0, "a", 1, ESC2voltage, ESCSymbol);
          OSD.printInt16(settings.m_OSDItems[ESC3voltage][0], settings.m_OSDItems[ESC3voltage][1]-CurrentMargin, motorCurrent[2], 2, 0, "a", 1, ESC3voltage, ESCSymbol);
          OSD.printInt16(settings.m_OSDItems[ESC4voltage][0], settings.m_OSDItems[ESC4voltage][1]-CurrentMargin, motorCurrent[3], 2 ,1, "a", 1, ESC4voltage, ESCSymbol);
          TMPmargin++;
        }
    
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_ESC_TEMPERATURE])
        {
          OSD.printInt16( settings.m_OSDItems[ESC1temp][0], settings.m_OSDItems[ESC1temp][1]+TMPmargin, ESCTemps[0], 0, 1, tempSymbol, 1, ESC1temp, ESCSymbol);
          OSD.printInt16(settings.m_OSDItems[ESC2temp][0], settings.m_OSDItems[ESC2temp][1]+TMPmargin, ESCTemps[1], 0, 0, tempSymbol, 1, ESC2temp, ESCSymbol);
          OSD.printInt16(settings.m_OSDItems[ESC3temp][0], settings.m_OSDItems[ESC3temp][1]-TMPmargin, ESCTemps[2], 0, 0, tempSymbol, 1, ESC3temp, ESCSymbol);
          OSD.printInt16(settings.m_OSDItems[ESC4temp][0], settings.m_OSDItems[ESC4temp][1]-TMPmargin, ESCTemps[3], 0, 1, tempSymbol, 1, ESC4temp, ESCSymbol);
        }
    
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_TIMER]) 
        {
          char stopWatchStr[] = { 0x00, 0x00 };
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
            stopWatchStr[0] = stopWatch;
          }
          else
          {
            stopWatchStr[0] = 0x00;
          }
          OSD.printTime(settings.m_OSDItems[STOPW][0], settings.m_OSDItems[STOPW][1], time, stopWatchStr, STOPWp);
        }
        
        if(settings.m_batWarning > 0 && (LipoMAH+previousMAH) >= settings.m_batWarningMAH)
        {
          totalMAH = 0;
          FLASH_STRING(BATTERY_LOW,   "battery low");
          FLASH_STRING(BATTERY_EMPTY, "           ");
          if(timer1sec) 
          {
            if(settings.m_displaySymbols == 1)
            {
              OSD.setCursor(settings.COLS/2 - 3, settings.ROWS/2 + 3);
              OSD.print(batterySymbol);
            }
            else
            {
              OSD.printFS(settings.COLS/2 - BATTERY_LOW.length()/2, settings.ROWS/2 +3, &BATTERY_LOW);
            }
          }
          else 
          {              
            OSD.printFS(settings.COLS/2 - BATTERY_LOW.length()/2, settings.ROWS/2 +3, &BATTERY_EMPTY);
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

        if(symbolOnOffChanged)
        {
          settings.fixColBorders();
          symbolOnOffChanged = false;
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
