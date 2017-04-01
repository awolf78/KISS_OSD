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

const char KISS_OSD_VER[] = "kiss osd v2.3.1";

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
SerialPort<0, 63, 0> NewSerial;
CStickInput inputChecker;
CSettings settings;
static int16_t DV_PPMs[CSettings::DISPLAY_DV_SIZE];

static volatile uint8_t vTxType = 0;
static volatile uint8_t vTxChannel = 0; 
static volatile uint8_t oldvTxChannel = 0;
static volatile uint8_t oldvTxBand = 0;
static volatile uint8_t vTxBand = 0;
static volatile int16_t vTxLowPower, vTxHighPower, oldvTxLowPower, oldvTxHighPower;
const uint8_t VTX_BAND_COUNT = 5;
const uint8_t VTX_CHANNEL_COUNT = 8;
static const uint16_t vtx_frequencies[VTX_BAND_COUNT][VTX_CHANNEL_COUNT] PROGMEM = {
    { 5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725 }, //A
    { 5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866 }, //B
    { 5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945 }, //E
    { 5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880 }, //F
    { 5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917 }  //R
  };
static const char bandSymbols[][2] = { {'a',0x00} , {'b', 0x00}, {'e', 0x00}, {'f', 0x00}, {'r', 0x00}};

#ifdef IMPULSERC_VTX
static volatile uint8_t vTxPower;
//static unsigned long changevTxTime = 0;

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
  OSD.display();
#ifdef IMPULSERC_VTX
  delay(100);
  MAX7456Setup();
#endif
  delay(100);
  OSD.setTextOffset(settings.m_xOffset, settings.m_yOffset); 
}

static int8_t moustacheRow, moustacheCol;

void setup()
{
  uint8_t i = 0;
  SPI.begin();
  SPI.setClockDivider( SPI_CLOCK_DIV2 ); 
  setupMAX7456();
  settings.cleanEEPROM();
  settings.ReadSettings(); 
  OSD.setTextOffset(settings.m_xOffset, settings.m_yOffset);
  moustacheCol = settings.COLS;
  moustacheRow = settings.ROWS-3; 
  
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
static int16_t angleX = 0;
static int16_t angleY = 0;
static uint16_t current = 0;
static uint16_t currentRT = 0;
static int8_t armed = 0;
static int8_t idleTime = 0;
static int16_t LipoVoltage = 0;
static int16_t LipoVoltageRT = 0;
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
static int16_t motorKERPMRT[4] = {0,0,0,0};
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
static bool flipOnce = false;


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
  delay(2000);
}

static int16_t p_roll = 0;
static int16_t p_pitch, p_yaw, p_tpa, i_roll, i_pitch, i_yaw, i_tpa, d_roll, d_pitch, d_yaw, d_tpa;
static int16_t rcrate_roll, rate_roll, rccurve_roll, rcrate_pitch, rate_pitch, rccurve_pitch, rcrate_yaw, rate_yaw, rccurve_yaw;
static uint8_t customTPAEnabled;
static uint8_t ctpa_bp1, ctpa_bp2, ctpa_infl0, ctpa_infl1, ctpa_infl2, ctpa_infl3;
static int16_t lpf_frq;
static uint8_t notchFilterEnabledR, notchFilterEnabledP;
static int16_t notchFilterCenterR, notchFilterCutR, notchFilterCenterP, notchFilterCutP;
static int16_t yawFilterCut;
static boolean fcSettingsReceived = false;
static boolean armOnYaw = true;
static uint32_t LastLoopTime = 0;
static boolean dShotEnabled = false;
static boolean logoDone = false;
static uint8_t protoVersion = 0;
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
static char batterySymbol[] = { 0x83, 0x88, 0x88, 0x89, 0x00 };
static char wattMeterSymbol[] = { 0xD7, 0xD8, 0x00 };
static char moustacheSymbol[] = { 0x7F, 0x80, 0x81, 0x82, 0x00 };
static uint8_t moustacheStarted = 0;
static bool angleXpositive = true;
static bool angleYpositive = true;
static uint8_t krSymbol[4] = { 0x9C, 0x9C, 0x9C, 0x9C };
static unsigned long krTime[4] = { 0, 0, 0, 0 };
volatile bool timer1sec = false;
volatile bool timer40Hz = false;
static unsigned long timer1secTime = 0;
static unsigned long timer40HzTime = 0;
static uint8_t currentDVItem = 0;
static bool symbolOnOffChanged = false;
static int16_t bufminus1 = 0;
static int16_t checkCalced = 0;
static uint16_t fcNotConnectedCount = 0;
static bool telemetryReceived = false;
static bool batWarnSymbol = true;
#ifdef NEW_FC_SETTINGS
enum _SETTING_MODES 
{
  FC_RATES, 
  FC_PIDS, 
  FC_VTX, 
  FC_FILTERS,
  FC_TPA,
  FC_SETTINGS
};
static const uint8_t MAX_SETTING_MODES = 6;
static const uint8_t getSettingModes[MAX_SETTING_MODES] = { 0x4D, 0x43, 0x45, 0x47, 0x4B, 0x30 }; 
static const uint8_t setSettingModes[MAX_SETTING_MODES] = { 0x4E, 0x44, 0x46, 0x48, 0x4C, 0x10 };
static bool fcSettingModeChanged[MAX_SETTING_MODES] = { false, false, false, false, false, false };
static uint8_t settingMode = 0;
#endif


static unsigned long _StartupTime = 0;
extern void* MainMenu();

void loop(){
  uint16_t i = 0;

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
  
  /*if(!OSD.status()) //trying to revive MAX7456 if it got a voltage hickup
  {
    ReviveOSD();
  }*/
  
  if(_StartupTime == 0)
  {
    _StartupTime = millis();
  }

  if((millis()-timer1secTime) > 500)
  {
    timer1secTime = millis();
    timer1sec = !timer1sec;
  }

  if((millis()-timer40HzTime) > 20)
  {
    timer40HzTime = millis();
    timer40Hz = !timer40Hz;
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

    for(i=0; i<MAX_SETTING_MODES; i++)
    {
      if(fcSettingModeChanged[i]) fcSettingChanged = true;
    }
    #ifdef NEW_FC_SETTINGS    
    if(!fcSettingsReceived)
    {
      if(settingMode >= MAX_SETTING_MODES) settingMode = 0;
      if(fcNotConnectedCount % 10 == 0) 
      {
        NewSerial.write(getSettingModes[settingMode]); // request settings
        if(getSettingModes[settingMode] > 0x30)
        {
          NewSerial.write((uint8_t)0x00);
          NewSerial.write((uint8_t)0x00);
        }
      }
      ReadFCSettings(fcSettingChanged, settingMode);
      if(!fcSettingsReceived) fcNotConnectedCount++;
      else fcNotConnectedCount = 0;
      if(fcSettingsReceived && settingMode < MAX_SETTING_MODES)
      {
        settingMode++;
        if(settingMode < MAX_SETTING_MODES) fcSettingsReceived = false;
      }
      if(settingMode == MAX_SETTING_MODES && 
            (p_roll < 0 || i_roll < 0 || d_roll < 0 || 
            p_pitch < 0 || i_pitch < 0 || d_pitch < 0 ||
            p_yaw < 0 || i_yaw < 0 || d_yaw < 0 ||
            rcrate_roll < 0 || rcrate_pitch < 0 || rcrate_yaw < 0 ||
            rate_roll < 0 || rate_pitch < 0 || rate_yaw < 0 ||
            p_tpa < 0 || i_tpa < 0 || d_tpa < 0))
      {
        fcSettingsReceived = false;
        settingMode = 0;
        return;
      }
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
          bool savedBefore = false;
          for(i=0; i<(MAX_SETTING_MODES-1); i++)
          {
            if(savedBefore && fcSettingModeChanged[i])
            {              
              while (!OSD.notInVSync());
              cleanScreen();
              static const char WAIT_SAVE_STR[] PROGMEM = "saving please wait!!!";
              OSD.printP(settings.COLS/2 - strlen_P(WAIT_SAVE_STR)/2, settings.ROWS/2, WAIT_SAVE_STR);
              delay(1000);
            }
            if(fcSettingModeChanged[i])
            {
              SendFCSettings(i);
              fcSettingModeChanged[i] = false;
              savedBefore = true;
            }      
          }
          fcSettingChanged = false;
        }
        else fcNotConnectedCount++;
      }
      else fcNotConnectedCount = 0;
    }
    #else
    if(!fcSettingsReceived)
    {
      NewSerial.write(0x30); // request settings
      ReadFCSettings(fcSettingChanged);
      if(!fcSettingsReceived) fcNotConnectedCount++;
      else fcNotConnectedCount = 0;
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
      }
      else fcNotConnectedCount = 0;
    }
    #endif

    if(fcNotConnectedCount <= 500 && (!fcSettingsReceived || !telemetryReceived)) return;

    while (!OSD.notInVSync());

    if(fcNotConnectedCount > 500)
    {
      cleanScreen();
      static const char FC_NOT_CONNECTED_STR[] PROGMEM = "no connection to kiss fc";
      OSD.printP(settings.COLS/2 - strlen_P(FC_NOT_CONNECTED_STR)/2, settings.ROWS/2, FC_NOT_CONNECTED_STR);
      triggerCleanScreen = true;
      fcNotConnectedCount = 0;
      return;
    }

#ifdef IMPULSERC_VTX
      vtx_flash_led(1);
#endif

      #ifdef DEBUG
      vTxType = 2;
      OSD.printInt16(0,8,protoVersion,0);
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
        static const char LAST_BATTERY_STR[] PROGMEM = "continue last battery?";
        OSD.printP(settings.COLS/2 - strlen_P(LAST_BATTERY_STR)/2, settings.ROWS/2 - 1, LAST_BATTERY_STR);
        
        static const char ROLL_RIGHT_STR[] PROGMEM = "roll right to confirm";
        OSD.printP(settings.COLS/2 - strlen_P(ROLL_RIGHT_STR)/2, settings.ROWS/2, ROLL_RIGHT_STR);
        static const char ARM_CANCEL_STR[] PROGMEM = "roll left to cancel";
        OSD.printP(settings.COLS/2 - strlen_P(ARM_CANCEL_STR)/2, settings.ROWS/2 + 1, ARM_CANCEL_STR);
                
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
        static const char BATTERY_STR[] PROGMEM = "battery ";
        uint8_t batCol = settings.COLS/2 - (strlen_P(BATTERY_STR)+1)/2;
        OSD.printInt16P(batCol, settings.ROWS/2 - 1, BATTERY_STR, settings.m_activeBattery+1, 0);        
        batCol = settings.COLS/2 - 3;
        OSD.printInt16(batCol, settings.ROWS/2, settings.m_batMAH[settings.m_activeBattery], 0, "mah", 2);

        static const char WARN_STR[] PROGMEM = "warn at ";
        OSD.printInt16P(settings.COLS/2 - (strlen_P(WARN_STR) + 6)/2, settings.ROWS/2 + 1, WARN_STR, settings.m_batWarningMAH, 0, "mah", 2);

        if(batterySelect)
        {
          static const char YAW_LEFT_STR[] PROGMEM = "yaw left to go back";
          OSD.printP(settings.COLS/2 - strlen_P(YAW_LEFT_STR)/2,settings.ROWS/2 + 2, YAW_LEFT_STR);
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
          fcSettingsReceived = false;
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
        static const char STATS_STR[] PROGMEM = "stats ";
        OSD.printInt16P( settings.COLS/2 - (strlen_P(STATS_STR)+4)/2, middle_infos_y, STATS_STR, statPage+1, 0);
        OSD.print( fixStr("/5") );
        middle_infos_y++;
        
        if(statPage == 0)
        {
          static const char TIME_STR[] PROGMEM =     "time     : ";
          static const char MAX_AMP_STR[] PROGMEM =  "max amps : ";
          static const char MAX_C_STR[] PROGMEM =    "max c    : ";
          static const char MAH_STR[] PROGMEM =      "mah      : ";
          static const char MAX_RPM_STR[] PROGMEM =  "max rpm  : ";
          static const char MAX_WATT_STR[] PROGMEM = "max watt : ";
          static const char MAX_TEMP_STR[] PROGMEM = "max temp : ";
          static const char MIN_V_STR[] PROGMEM =    "min v    : ";

          uint8_t statCol = settings.COLS/2 - (strlen_P(TIME_STR)+7)/2;
          OSD.printP(statCol, ++middle_infos_y, TIME_STR);
          OSD.printTime( statCol+strlen_P(TIME_STR), middle_infos_y, total_time);  
          OSD.printInt16P( statCol, ++middle_infos_y, MAX_AMP_STR, MaxAmps, 1, "a" );           
          OSD.printInt16P( statCol, ++middle_infos_y, MAX_C_STR, MaxC, 0, "c");
          OSD.printInt16P( statCol, ++middle_infos_y, MAH_STR, LipoMAH+previousMAH, 0, "mah" );
          OSD.printInt16P( statCol, ++middle_infos_y, MAX_RPM_STR, MaxRPMs, 1, "kr" );  
          OSD.printInt16P( statCol, ++middle_infos_y, MAX_WATT_STR, MaxWatt, 1, "w" );
          OSD.printInt16P( statCol, ++middle_infos_y, MAX_TEMP_STR, MaxTemp, 0, tempSymbol);
          OSD.printInt16P( statCol, ++middle_infos_y, MIN_V_STR, MinBat, 2, "v" );          
        }
        else
        {
          static char ESC_STAT_STR1[] = "esc";
          char* ESC_STAT_STR = ESC_STAT_STR1;
          if(settings.m_displaySymbols == 1)
          {
            ESC_STAT_STR = ESCSymbol;
          }
          static const char ESC_RPM_STR[] PROGMEM =  " max rpm : ";
          static const char ESC_A_STR[] PROGMEM =    " max a   : ";
          static const char ESC_TEMP_STR[] PROGMEM = " max temp: ";
          static const char ESC_MINV_STR[] PROGMEM = " min v   : ";
          static const char ESC_MAH_STR[] PROGMEM =  " mah     : ";
          
          uint8_t startCol = settings.COLS/2 - (strlen_P(ESC_RPM_STR)+strlen(ESC_STAT_STR)+7)/2;
          OSD.printInt16( startCol, ++middle_infos_y, ESC_STAT_STR, statPage, 0);
          OSD.printInt16P( startCol + strlen(ESC_STAT_STR) + 1, middle_infos_y, ESC_RPM_STR, maxKERPM[statPage-1], 1, "kr");
          
          OSD.printInt16( startCol, ++middle_infos_y, ESC_STAT_STR, statPage, 0);
          OSD.printInt16P( startCol + strlen(ESC_STAT_STR) + 1, middle_infos_y, ESC_A_STR, maxCurrent[statPage-1], 2, "a"); 
          
          OSD.printInt16( startCol, ++middle_infos_y, ESC_STAT_STR, statPage, 0);
          OSD.printInt16P( startCol + strlen(ESC_STAT_STR) + 1, middle_infos_y, ESC_TEMP_STR, maxTemps[statPage-1], 0, tempSymbol);
          
          OSD.printInt16( startCol, ++middle_infos_y, ESC_STAT_STR, statPage, 0);
          OSD.printInt16P( startCol + strlen(ESC_STAT_STR) + 1, middle_infos_y, ESC_MINV_STR, minVoltage[statPage-1], 2, "v");
          
          OSD.printInt16( startCol, ++middle_infos_y, ESC_STAT_STR, statPage, 0);
          OSD.printInt16P( startCol + strlen(ESC_STAT_STR) + 1, middle_infos_y, ESC_MAH_STR, ESCmAh[statPage-1], 0, "mah"); 
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
          OSD.printInt16( settings.m_OSDItems[THROTTLE][0], settings.m_OSDItems[THROTTLE][1], throttle, 0, "%", 2, THROTTLEp);
        }
          
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_NICKNAME])
        {
          uint8_t nickBlanks = 0;
          OSD.checkPrintLength(settings.m_OSDItems[NICKNAME][0], settings.m_OSDItems[NICKNAME][1], strlen(settings.m_nickname), nickBlanks, NICKNAMEp);
          OSD.print( fixStr(settings.m_nickname) );          
        }
    
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_COMB_CURRENT])
        {
          if(settings.m_displaySymbols == 1)
          {
            const int16_t wattPercentage = 20; //95% of max
            uint32_t Watts = (uint32_t)LipoVoltageRT * (uint32_t)(currentRT * 10);            
            int16_t tempWatt = (int16_t)(Watts/1000);
            uint8_t ampBlanks = 0;
            int8_t wattRow = settings.m_OSDItems[AMPS][1]-1;
            if(wattRow < 0) wattRow = 0;
            int16_t wattSlice;
            uint8_t wattStatus;
            switch(settings.m_wattMeter)
            {              
              case 1:
                wattSlice = (settings.m_maxWatts - settings.m_maxWatts/wattPercentage) / 8;
                wattStatus = 0xD7;
                while((tempWatt-wattSlice) > 0 && wattStatus < 0xE1) 
                {
                  tempWatt -= wattSlice;
                  wattStatus += 2;                    
                }
                if(wattStatus == 0xD7 && armed > 0) wattStatus += 2;
                wattMeterSymbol[0] = wattStatus;
                wattMeterSymbol[1] = wattStatus+1;                                
                OSD.checkPrintLength(settings.m_OSDItems[AMPS][0], wattRow+1, 2, ampBlanks, AMPSp);
                OSD.print(wattMeterSymbol);
                if(wattStatus == 0xE1) wattStatus = 0xE5;
                else wattStatus = 0xE3;
                while((tempWatt-wattSlice) > 0 && wattStatus < 0xEB) 
                {
                  tempWatt -= wattSlice;
                  wattStatus += 2;                    
                }                
                wattMeterSymbol[0] = wattStatus;
                wattMeterSymbol[1] = wattStatus+1;
                OSD.checkPrintLength(settings.m_OSDItems[AMPS][0], wattRow, 2, ampBlanks, AMPSp);
                OSD.print(wattMeterSymbol);
              break;
              case 2:                
                wattSlice = (settings.m_maxWatts - settings.m_maxWatts/wattPercentage) / 5;
                //wattSlice = settings.m_maxWatts / 5;
                wattStatus = 0x01;
                while((tempWatt-wattSlice) > 0 && wattStatus < 0x07) 
                {
                  tempWatt -= wattSlice;
                  wattStatus += 2;                    
                }
                wattMeterSymbol[0] = wattStatus;
                wattMeterSymbol[1] = wattStatus+1;
                OSD.checkPrintLength(settings.m_OSDItems[AMPS][0], wattRow, 2, ampBlanks, AMPSp);
                OSD.print(wattMeterSymbol);
                if(wattStatus == 0x07) wattStatus = 0x0B;
                else wattStatus = 0x09;
                while((tempWatt-wattSlice) > 0 && wattStatus < 0x0D) 
                {
                  tempWatt -= wattSlice;
                  wattStatus += 2;                    
                }      
                if(wattStatus == 0x0D)
                {
                  wattSlice /= 2;
                  while((tempWatt-wattSlice) > 0 && wattStatus < 0x11) 
                  {
                    tempWatt -= wattSlice;
                    wattStatus += 2;                    
                  }
                }
                wattMeterSymbol[0] = wattStatus;
                wattMeterSymbol[1] = wattStatus+1;
                OSD.checkPrintLength(settings.m_OSDItems[AMPS][0], wattRow+1, 2, ampBlanks, AMPSp);
                OSD.print(wattMeterSymbol);
              break;
            }            
          }
          else
          {
            OSD.printInt16(settings.m_OSDItems[AMPS][0], settings.m_OSDItems[AMPS][1], current, 1, "a", 2, AMPSp);
          }
        }
        
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_LIPO_VOLTAGE])
        {
          OSD.printInt16( settings.m_OSDItems[VOLTAGE][0], settings.m_OSDItems[VOLTAGE][1], LipoVoltage / 10, 1, "v", 1, VOLTAGEp);
        }
        
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_MA_CONSUMPTION])
        {        
          if(settings.m_displaySymbols == 1)
          {
            uint8_t batCount = (LipoMAH+previousMAH) / settings.m_batSlice;
            uint8_t batStatus = 0x88;
            while(batCount > 4)
            {
              batStatus--;
              batCount--;
            }
            batterySymbol[2] = (char)batStatus;
            batStatus = 0x88;
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
            OSD.printInt16(settings.m_OSDItems[MAH][0], settings.m_OSDItems[MAH][1], LipoMAH+previousMAH, 0, "mah", 0, MAHp);
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
          if(settings.m_displaySymbols == 1 && settings.m_props == 1)
          {
            for(i=0; i<4; i++)
            {
              if(millis() - krTime[i] > (10000/motorKERPMRT[i]))
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
            OSD.printInt16(settings.m_OSDItems[ESC1kr][0], settings.m_OSDItems[ESC1kr][1], motorKERPM[0], 1, KR2, 1, ESC1kr);
            OSD.printInt16(settings.m_OSDItems[ESC2kr][0], settings.m_OSDItems[ESC2kr][1], motorKERPM[1], 1, KR2, 1, ESC2kr);
            OSD.printInt16(settings.m_OSDItems[ESC3kr][0], settings.m_OSDItems[ESC3kr][1], motorKERPM[2], 1, KR2, 1, ESC3kr);
            OSD.printInt16(settings.m_OSDItems[ESC4kr][0], settings.m_OSDItems[ESC4kr][1], motorKERPM[3], 1, KR2, 1, ESC4kr);
          }
          
          TMPmargin++;
          CurrentMargin++;
        }
     
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_ESC_CURRENT])
        {
          static char ampESC[] = { 'a', 0x7E, 0x00};
          OSD.printInt16(settings.m_OSDItems[ESC1voltage][0], settings.m_OSDItems[ESC1voltage][1]+CurrentMargin, motorCurrent[0], 2, "a", 1, ESC1voltage, ESCSymbol);
          OSD.printInt16(settings.m_OSDItems[ESC2voltage][0], settings.m_OSDItems[ESC2voltage][1]+CurrentMargin, motorCurrent[1], 2, ampESC, 1, ESC2voltage);
          OSD.printInt16(settings.m_OSDItems[ESC3voltage][0], settings.m_OSDItems[ESC3voltage][1]-CurrentMargin, motorCurrent[2], 2, ampESC, 1, ESC3voltage);
          OSD.printInt16(settings.m_OSDItems[ESC4voltage][0], settings.m_OSDItems[ESC4voltage][1]-CurrentMargin, motorCurrent[3], 2, "a", 1, ESC4voltage, ESCSymbol);
          TMPmargin++;
        }
    
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_ESC_TEMPERATURE])
        {
          static char tempESC[] = { tempSymbol[0], 0x7E, 0x00};
          OSD.printInt16( settings.m_OSDItems[ESC1temp][0], settings.m_OSDItems[ESC1temp][1]+TMPmargin, ESCTemps[0], 0, tempSymbol, 1, ESC1temp, ESCSymbol);
          OSD.printInt16(settings.m_OSDItems[ESC2temp][0], settings.m_OSDItems[ESC2temp][1]+TMPmargin, ESCTemps[1], 0, tempESC, 1, ESC2temp);
          OSD.printInt16(settings.m_OSDItems[ESC3temp][0], settings.m_OSDItems[ESC3temp][1]-TMPmargin, ESCTemps[2], 0, tempESC, 1, ESC3temp);
          OSD.printInt16(settings.m_OSDItems[ESC4temp][0], settings.m_OSDItems[ESC4temp][1]-TMPmargin, ESCTemps[3], 0, tempSymbol, 1, ESC4temp, ESCSymbol);
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
          static const char BATTERY_LOW[] PROGMEM =   "battery low";
          static const char BATTERY_EMPTY[] PROGMEM = "           ";
          if(timer1sec) 
          {
            if(settings.m_displaySymbols == 1)
            {
              if(batWarnSymbol)
              {
                OSD.setCursor(settings.COLS/2 - 3, settings.ROWS/2 + 3);
                OSD.print(batterySymbol);
              }
              else
              {
                OSD.printInt16(settings.COLS/2 - 3, settings.ROWS/2 + 3, LipoMAH+previousMAH, 0, "mah");
              }
              flipOnce = true;              
            }
            else
            {
              OSD.printP(settings.COLS/2 - strlen_P(BATTERY_LOW)/2, settings.ROWS/2 +3, BATTERY_LOW);
            }
          }
          else 
          {
            if(flipOnce)
            {
              batWarnSymbol = !batWarnSymbol;
              flipOnce = false;              
            }
            OSD.printP(settings.COLS/2 - strlen_P(BATTERY_LOW)/2, settings.ROWS/2 +3, BATTERY_EMPTY);
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

        if(settings.m_voltWarning > 0 && settings.m_minVolts > (LipoVoltage / 10) && !timer1sec)
        {
          OSD.printInt16(settings.COLS/2 - 3, settings.ROWS/2 + 2, LipoVoltage / 10, 1, "v", 1);                      
        }
        else
        {
          OSD.setCursor(settings.COLS/2 - 3, settings.ROWS/2 + 2);
          OSD.printSpaces(5);
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
