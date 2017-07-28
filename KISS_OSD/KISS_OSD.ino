/*
KISS FC OSD v1.0
By Felix Niessen (felix.niessen@googlemail.com)
for Flyduino.net

KISS FC OSD v2.x
by Alexander Wolf (awolf78@gmx.de)
for everyone who loves KISS

This is free and unencumbered software released into the public domain.

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
static const uint8_t BAT_MAH_INCREMENT = 50;

// END OF CONFIGURATION
//=========================================================================================================================


// internals
//=============================

//#define DEBUG
//#define SIMULATE_VALUES
static bool doItOnce = true;

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
#if defined(ADVANCED_STATS) || defined(ADVANCED_ESC_STATS)
#include "CStatGenerator.h"
#endif

#ifdef STEELE_PDB
static const char KISS_OSD_VER[] PROGMEM = "steele osd v2.4";
#else
static const char KISS_OSD_VER[] PROGMEM = "kiss osd v2.4";
#endif


#if (defined(IMPULSERC_VTX) || defined(STEELE_PDB)) && !defined(STEELE_PDB_OVERRIDE)
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

static uint8_t vTxType = 0;
static uint8_t vTxChannel = 0; 
static uint8_t oldvTxChannel = 0;
static uint8_t oldvTxBand = 0;
static uint8_t vTxBand = 0;
static uint16_t vTxLowPower, vTxHighPower, oldvTxLowPower, oldvTxHighPower;
const uint8_t VTX_BAND_COUNT = 5;
const uint8_t VTX_CHANNEL_COUNT = 8;
static const uint16_t vtx_frequencies[VTX_BAND_COUNT][VTX_CHANNEL_COUNT] PROGMEM = {
    { 5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725 }, //A
    { 5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866 }, //B
    { 5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945 }, //E
    { 5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880 }, //F
    { 5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917 }  //R
  };
static const char bandSymbols[VTX_BAND_COUNT][2] = { {'a',0x00} , {'b', 0x00}, {'e', 0x00}, {'f', 0x00}, {'r', 0x00}};

#ifdef IMPULSERC_VTX
static uint8_t vTxPower;
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

void checkVideoMode()
{
  uint8_t videoSys = OSD.videoSystem();
  if(videoSys != settings.m_videoMode)
  {
    settings.m_videoMode = videoSys;
    if(settings.m_videoMode == MAX7456_PAL) settings.ROWS = 15;
    else settings.ROWS = 13;
    OSD.setDefaultSystem(videoSys);
    OSD.setTextArea(settings.COLS, settings.ROWS);           
  }  
}

void setupMAX7456()
{
  #if (defined(IMPULSERC_VTX) || defined(STEELE_PDB)) && !defined(STEELE_PDB_OVERRIDE)
  MAX7456Setup();
  delay(100);
  #endif
  OSD.begin(settings.COLS,13);    
  #ifdef FORCE_NTSC
  OSD.setDefaultSystem(MAX7456_NTSC);
  #elif defined(FORCE_PAL)
  OSD.setDefaultSystem(MAX7456_PAL);
  #else
  delay(1000);
  checkVideoMode();
  #endif
  OSD.setTextArea(settings.COLS, settings.ROWS);
  OSD.setSwitchingTime( 5 ); 
  OSD.display();
  delay(100);
  OSD.setTextOffset(settings.m_xOffset, settings.m_yOffset); 
}

static uint8_t  throttle = 0;
static uint16_t  roll = 0;
static uint16_t  pitch = 0;
static uint16_t  yaw = 0;
static uint16_t current = 0;
static uint8_t armed = 0;
static uint16_t LipoVoltage = 0;
static uint16_t LipoMAH = 0;
static uint16_t  previousMAH = 0;
static uint16_t totalMAH = 0;
static uint16_t statMAH = 0;
static uint16_t remainMAH = 0;
static uint16_t MaxAmps = 0;
static uint8_t  MaxC    = 0;
static uint16_t MaxRPMs = 0;
static uint16_t MaxWatt = 0;
static uint8_t  MaxTemp = 0;
static uint16_t MinBat = 0;
static uint8_t  MinRSSI = 100;
static uint16_t motorKERPM[4] = {0,0,0,0};
static uint16_t maxKERPM[4] = {0,0,0,0};
static uint16_t motorCurrent[4] = {0,0,0,0};
static uint16_t maxCurrent[4] = {0,0,0,0};
static uint16_t ESCTemps[4] = {0,0,0,0};
static uint16_t maxTemps[4] = {0,0,0,0};
static uint16_t ESCVoltage[4] = {0,0,0,0};
static uint16_t minVoltage[4] = {10000,10000,10000,10000};
static uint16_t ESCmAh[4] = {0,0,0,0};
static int16_t  AuxChanVals[5] = {0,0,0,0,0};
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
static bool flipOnce = true;
#ifdef NEW_FILTER
CMeanFilter rssiFilter(10);
#else
CMeanFilter rssiFilter(7);
#endif


uint16_t findMax4(uint16_t maxV, uint16_t *values, uint8_t length) 
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


uint16_t findMax(uint16_t maxV, uint16_t newVal) 
{
  if(newVal > maxV) {
    return newVal;
  }
  return maxV;
}

uint16_t findMin(uint16_t minV, uint16_t newVal) 
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

enum _ROLL_PITCH_YAW
{
  _ROLL,
  _PITCH,
  _YAW
};


struct _FC_FILTERS
{
  uint8_t lpf_frq;
  uint8_t yawFilterCut;
  uint8_t notchFilterEnabledR;
  uint16_t notchFilterCenterR;
  uint16_t notchFilterCutR;
  uint8_t notchFilterEnabledP;
  uint16_t notchFilterCenterP;
  uint16_t notchFilterCutP;
  uint8_t yawLpF;
  uint8_t DLpF;
} fc_filters;

struct _FC_TPA
{
  uint16_t tpa[3];
  uint8_t customTPAEnabled;
  uint8_t ctpa_bp1;
  uint8_t ctpa_bp2;
  uint8_t ctpa_infl[4];
} fc_tpa;

static uint16_t pid_p[3], pid_i[3], pid_d[3], rcrate[3], rate[3], rccurve[3];
static uint16_t rcrate_roll, rate_roll, rccurve_roll, rcrate_pitch, rate_pitch, rccurve_pitch, rcrate_yaw, rate_yaw, rccurve_yaw;
static boolean moreLPFfilters = false;
static boolean fcSettingsReceived = false;
static boolean armOnYaw = true;
static uint32_t LastLoopTime = 0;
static boolean dShotEnabled = false;
static boolean logoDone = false;
static uint8_t protoVersion = 0;
static uint8_t failSafeState = 10;
extern void ReadFCSettings(boolean skipValues);

typedef void* (*fptr)();
static char tempSymbol[] = {0xB0, 0x00};
static char ESCSymbol[] = {0x7E, 0x00};
static const char crossHairSymbol[] = {0x11, 0x00};
static uint8_t code = 0;
static boolean menuActive = false;
static boolean menuDisabled = false;
static boolean menuWasActive = false;
static boolean fcSettingChanged = false;
static void* activePage = NULL;
static boolean batterySelect = false;
static boolean triggerCleanScreen = true;
static uint8_t activeMenuItem = 0;
static uint8_t stopWatch = 0x94;
static char batteryIcon[] = { 0x83, 0x84, 0x84, 0x89, 0x00 };
static char wattMeterSymbol[] = { 0xD7, 0xD8, 0x00 };
/*static bool angleXpositive = true;
static bool angleYpositive = true;*/
static uint8_t krSymbol[4] = { 0x9C, 0x9C, 0x9C, 0x9C };
static unsigned long krTime[4] = { 0, 0, 0, 0 };
bool timer1sec = false;
bool timer40Hz = false;
static unsigned long timer1secTime = 0;
static bool symbolOnOffChanged = false;
static uint16_t fcNotConnectedCount = 0;
static bool telemetryReceived = false;
static bool batWarnSymbol = true;
static uint8_t zeroBlanks = 0;
static bool failsafeTriggered = false;
static bool vTxPowerActive = false;
static int8_t vTxPowerKnobChannel = -1;
static int16_t vTxPowerKnobLastPPM = -1;
static unsigned long vTxPowerTime = 0;
static uint8_t oldPrintCount = 0;
static uint8_t printCount = 0;
static bool statsActive = false;
#ifdef ADVANCED_STATS
static const uint8_t STAT_GENERATOR_SIZE = 6;
CStatGenerator statGenerators[STAT_GENERATOR_SIZE] = { CStatGenerator(5,20), CStatGenerator(20,40), CStatGenerator(40,60), CStatGenerator(60,80), CStatGenerator(80,100), CStatGenerator(95,100) };
#endif
#ifdef ADVANCED_ESC_STATS
CStatGenerator ESCstatGenerators[4] = { CStatGenerator(90,100), CStatGenerator(90,100), CStatGenerator(90,100), CStatGenerator(90,100) };
#endif

enum _SETTING_MODES 
{
  FC_SETTINGS,
  FC_RATES, 
  FC_PIDS, 
  FC_VTX, 
  FC_FILTERS,
  FC_TPA
};
static const uint8_t MAX_SETTING_MODES = 6;
static const uint8_t getSettingModes[MAX_SETTING_MODES] = { 0x30, 0x4D, 0x43, 0x45, 0x47, 0x4B }; 
static const uint8_t setSettingModes[MAX_SETTING_MODES] = { 0x10, 0x4E, 0x44, 0x46, 0x48, 0x4C };
static bool fcSettingModeChanged[MAX_SETTING_MODES] = { false, false, false, false, false, false };
static uint8_t settingMode = 0;


static unsigned long _StartupTime = 0;
extern void* MainMenu();

static char iconPrintBuf1[] = { 0x00, 0x00, 0x00 };
static char iconPrintBuf2[] = { 0x00, 0x00, 0x00 };

static char ESC_STAT_STR1[] = "esc";

void getIconPos(uint16_t value, uint16_t maxValue, uint8_t steps, char &iconPos1, char &iconPos2, char inc = 1)
{
  if(value > maxValue) value = maxValue;
  uint16_t halfMax = maxValue / 2;
  uint16_t stepValue = (uint16_t)(maxValue / (uint16_t)(steps));
  uint16_t tempValue1 = 0;
  if(value > halfMax) tempValue1 = value - halfMax;
  value -= tempValue1;
  iconPos1 += ((char)(value / stepValue)) * inc; 
  if(tempValue1 > 0)
  {    
    iconPos2 += ((char)(tempValue1 / stepValue)) * inc;    
  }
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

void loop(){
  uint8_t i = 0;

  unsigned long _millis = millis();

#ifdef IMPULSERC_VTX
    vtx_process_state(_millis, vTxBand, vTxChannel);
#endif
  
  /*if(!OSD.status()) //trying to revive MAX7456 if it got a voltage hickup
  {
    ReviveOSD();
  }*/
  
  if(_StartupTime == 0)
  {
    _StartupTime = _millis;
  }

  if((_millis-timer1secTime) > 500)
  {
    timer1secTime = _millis;
    timer1sec = !timer1sec;
  }

#ifdef IMPULSERC_VTX
      if(timer1sec) vtx_flash_led(1);
#endif
#if defined(STEELE_PDB) && !defined(STEELE_PDB_OVERRIDE)
      if(timer1sec) steele_flash_led(_millis, 1);
#endif
  
  if(micros()-LastLoopTime > 10000)
  { 
    LastLoopTime = micros();

    #if !defined(FORCE_PAL) && !defined(FORCE_NTSC)
    checkVideoMode();
    #endif
    
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
    
    if(!fcSettingsReceived && !menuDisabled)
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
    }
    else
    {
      if(!fcSettingChanged || menuActive)
      {
        uint8_t requestTelemetry = 0x20;
        if(protoVersion > 108) requestTelemetry = 0x13;
        NewSerial.write(requestTelemetry);
      }
      telemetryReceived = ReadTelemetry();
      if(!telemetryReceived) 
      {
        if(fcSettingChanged && !menuActive)
        {
          bool savedBefore = false;
          for(i=1; i<MAX_SETTING_MODES; i++)
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

    while (!OSD.notInVSync());

    if(fcNotConnectedCount > 500)
    {
      cleanScreen();
      static const char FC_NOT_CONNECTED_STR[] PROGMEM = "no connection to kiss fc";
      OSD.printP(settings.COLS/2 - strlen_P(FC_NOT_CONNECTED_STR)/2, settings.ROWS-2, FC_NOT_CONNECTED_STR);
      triggerCleanScreen = true;
      logoDone = true;
      fcNotConnectedCount = 0;
      /*OSD.printInt16(0, settings.ROWS/2, checksumDebug, 0);
      OSD.printInt16(0, settings.ROWS/2+1, bufMinusOne, 0);
      OSD.printInt16(0, settings.ROWS/2+2, settingMode, 0);*/
      return;
    }

    if(telemetryReceived) code = inputChecker.ProcessStickInputs(roll, pitch, yaw, armed);

    /*OSD.printInt16(0, settings.ROWS/2, armed, 0);
    OSD.printInt16(0, settings.ROWS/2+1, vTxPowerActive, 0);*/
    if(settings.m_lastMAH > 0)
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
      if(code & inputChecker.ROLL_LEFT || armed > 0)
      {
        settings.m_lastMAH = 0;
        settings.WriteLastMAH();
        cleanScreen();
      }  
      return;
    }
      
    #ifdef SHOW_KISS_LOGO
    if(!logoDone && armed == 0 && !menuActive && !armedOnce && settings.m_IconSettings[KISS_ICON] == 1)
    {
      #ifdef STEELE_PDB
      uint8_t logoCol = settings.COLS/2-3;
      uint8_t logoRow = settings.ROWS/2-2;
      static const char impulseRC_logo[][7] PROGMEM = { { 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0x00 },
                                                        { 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0x00 },
                                                        { 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0x00 },
                                                        { 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0x00 } };
      for(i=0; i<4; i++)
      {
        OSD.setCursor(logoCol, logoRow);
        OSD.print(fixPStr(impulseRC_logo[i]));
        logoRow++;            
      }
      // This was for the big logo
      /*static const char impulseRC_logo2[][17] = { { 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xCC, 0xCD, 0xCE, 0xCF, 0x00 },
                                                  { 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0x00 } };
      logoCol = settings.COLS/2-8;
      for(i=0; i<2; i++)
      {
        OSD.setCursor(logoCol, logoRow);
        OSD.print(impulseRC_logo2[i]);
        logoRow++;            
      }*/
      static const char impulseRC_logo2[][13] PROGMEM = { { 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0x00 },
                                                          { 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xED, 0x00 } };
      logoCol = settings.COLS/2-6;
      for(i=0; i<2; i++)
      {
        OSD.setCursor(logoCol, logoRow);
        OSD.print(fixPStr(impulseRC_logo2[i]));
        logoRow++;            
      }        
      logoCol = settings.COLS/2-2;
      OSD.setCursor(logoCol, logoRow);
      static const char mustache[] PROGMEM = { 0x7F, 0x80, 0x81, 0x82, 0x00 };
      OSD.print(fixPStr(mustache));
      #else
      uint8_t logoCol = 11;
      uint8_t logoRow = 5;
      OSD.setCursor(logoCol, logoRow);
      static const char kiss_logo1[] PROGMEM = { 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0x00 };
      static const char kiss_logo2[] PROGMEM = { 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0x00 };
      static const char kiss_logo3[] PROGMEM = { 0xC3, 0xC4, 0xC5, 0xC6, 0x00 };
      OSD.print(fixPStr(kiss_logo1));
      logoRow++;
      logoCol--;
      OSD.setCursor(logoCol, logoRow);
      OSD.print(fixPStr(kiss_logo2));
      logoRow++;
      logoCol -= 2;
      OSD.setCursor(logoCol, logoRow);
      OSD.print(fixPStr(kiss_logo3));
      if(dShotEnabled)
      {
        static const char DShot_logo1[] PROGMEM = { 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0x00 };
        static const char DShot_logo2[] PROGMEM = { 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0x00 };
        OSD.print(fixPStr(DShot_logo1));
        logoRow++;
        logoCol += 4;
        OSD.setCursor(logoCol, logoRow);
        OSD.print(fixPStr(DShot_logo2));
      }
      #endif
      if(_StartupTime > 0 && _millis - _StartupTime > 60000)
      {
        logoDone = true;
        cleanScreen();
      }
    }
    #else
    logoDone = true;
    #endif

    if(fcNotConnectedCount <= 500 && (!fcSettingsReceived || !telemetryReceived)) return;

#ifdef IMPULSERC_VTX
      vtx_flash_led(1);
#endif
#if defined(STEELE_PDB) && !defined(STEELE_PDB_OVERRIDE)
      steele_flash_led(_millis, 1);
#endif

    #ifdef FAILSAFE
    if(armedOnce && failSafeState > 9)
    {
      if(!failsafeTriggered) cleanScreen();
      static const char FAILSAFE_STR[] PROGMEM = "failsafe";
      OSD.printP(settings.COLS/2 - strlen_P(FAILSAFE_STR)/2, settings.ROWS/2, FAILSAFE_STR);
      failsafeTriggered = true;
      return;
    }
    else if(failsafeTriggered)
    {
      failsafeTriggered = false;
      cleanScreen();
    }
    #endif

      #ifdef DEBUG
      vTxType = 2;
      OSD.printInt16(0,8,protoVersion,0);
      #endif

      if(triggerCleanScreen)
      {
        triggerCleanScreen = false;
        cleanScreen();
      }

      if(oldPrintCount != printCount)
      {
        cleanScreen();
        oldPrintCount = printCount;
      }
      
      if(settings.m_tempUnit == 1)
      {
        tempSymbol[0] = fixChar(0xB1);
      }
      else
      {
        tempSymbol[0] = fixChar(0xB0);
      }      

      if(code & inputChecker.ROLL_RIGHT) 
      {
        logoDone = true;
        cleanScreen();
      }
      
      if(armed == 0 && code & inputChecker.YAW_LONG_LEFT && code & inputChecker.ROLL_LEFT && code & inputChecker.PITCH_DOWN)
      {
        ReviveOSD();
      }
      
      if(batterySelect || (!menuActive && !armOnYaw && yaw > 1900 && armed == 0))
      {
        if(!showBat)
        {
          cleanScreen();
          showBat = true;
          logoDone = true;
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
          settings.m_batMAH[settings.m_activeBattery] += (int16_t)BAT_MAH_INCREMENT;
          settings.FixBatWarning();
          settingChanged = true;
        }
        if((code & inputChecker.PITCH_DOWN) && settings.m_batMAH[settings.m_activeBattery] > 300)
        {
          settings.m_batMAH[settings.m_activeBattery] -= (int16_t)BAT_MAH_INCREMENT;
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
      
      if(armed == 0 && ((code &  inputChecker.YAW_LONG_LEFT) || menuActive) && !menuDisabled)
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
        DV_change_time = _millis;
        last_Aux_Val = AuxChanVals[settings.m_DVchannel];
      }

      #ifndef IMPULSERC_VTX
      #ifdef VTX_POWER_KNOB
      //OSD.printInt16(0, settings.ROWS/2, vTxPowerKnobChannel, 0);      
      if(vTxPowerKnobChannel > -1 && vTxPowerKnobLastPPM == -1 && failSafeState < 10) vTxPowerKnobLastPPM = AuxChanVals[vTxPowerKnobChannel]+1000;                 
      if(vTxPowerKnobChannel > -1 && failSafeState < 10 && ((AuxChanVals[vTxPowerKnobChannel]+1000 != vTxPowerKnobLastPPM) || vTxPowerActive))
      {          
        if((!vTxPowerActive || AuxChanVals[vTxPowerKnobChannel]+1000 != vTxPowerKnobLastPPM) && armed == 0)
        {
          if(!vTxPowerActive) 
          {            
            logoDone = true;
            cleanScreen();
          }
          vTxPowerTime = _millis;
          vTxPowerActive = true;
          vTxPowerKnobLastPPM = AuxChanVals[vTxPowerKnobChannel]+1000;          
        }
        else 
        {
          if(_millis - vTxPowerTime > 1000 || armed > 0)
          {
            vTxPowerActive = false;
            cleanScreen();
          }
        }
        if(vTxPowerActive)
        {              
          static const char VTX_POWER_STATE[] PROGMEM = "vtx power:";
          OSD.printP(settings.COLS/2 - strlen_P(VTX_POWER_STATE)/2, settings.ROWS/2, VTX_POWER_STATE);
          static const char suffix[] = "mw";
          uint8_t maxMWmult = 60;            
          if(vTxType > 2)
          {
            maxMWmult = 80;              
          }
          if(settings.m_vTxMaxPower > 0) maxMWmult = (uint8_t)(settings.m_vTxMaxPower/(int16_t)10);
          int16_t currentVTXPower = ((vTxPowerKnobLastPPM/(int16_t)20)*(int16_t)maxMWmult)/(int16_t)10;          
          OSD.printInt16(settings.COLS/2 - 2, settings.ROWS/2+1, currentVTXPower, 0, suffix, 1);
        }
      }
      #endif
      #endif

      //OSD.printInt16(0, settings.ROWS/2, settings.m_stats, 0);
      if(!vTxPowerActive && DV_change_time == 0 && armed == 0 && armedOnce) 
      {
        switch(settings.m_stats)
        {
          case 1:
            statsActive = true;
          break;
          case 2:
            if(code & inputChecker.ROLL_RIGHT) 
            {
              cleanScreen();
              statsActive = false;
            }
            if(code & inputChecker.ROLL_LEFT) 
            {
              cleanScreen();
              statsActive = true;
            }
          break;
        }       
      }
      else statsActive = false;

      statMAH = LipoMAH+previousMAH;
      if(statsActive) 
      {
        if(code & inputChecker.PITCH_UP && statPage > 0)
        {
          statPage--;
          cleanScreen();
        }        
        #ifdef ADVANCED_STATS
        uint8_t maxPages = 5;
        #else
        uint8_t maxPages = 4;
        #endif
        if(code & inputChecker.PITCH_DOWN && statPage < maxPages)
        {
          statPage++;
          cleanScreen();
        }
        uint8_t middle_infos_y     = 1;    
        static const char STATS_STR[] PROGMEM = "stats ";
        OSD.printInt16P( settings.COLS/2 - (strlen_P(STATS_STR)+4)/2, middle_infos_y, STATS_STR, statPage+1, 0);
        #ifdef ADVANCED_STATS
        OSD.print( fixStr("/6") );
        #else
        OSD.print( fixStr("/5") );
        #endif
        middle_infos_y++;
        uint8_t statCol, j;
        int16_t avgTotal = 0;
        
        switch(statPage)
        {
          case 0:
            static const char TIME_STR[] PROGMEM =     "time     : ";
            static const char MAX_AMP_STR[] PROGMEM =  "max amps : ";
            #ifdef ADVANCED_ESC_STATS
            static const char MAX_AVG_STR[] PROGMEM =  "max avg  : ";
            #endif
            static const char MIN_V_STR[] PROGMEM =    "min v    : ";
            static const char MAX_WATT_STR[] PROGMEM = "max watt : ";          
            static const char MAX_C_STR[] PROGMEM =    "c rating : ";
            static const char MAH_STR[] PROGMEM =      "mah      : ";
            static const char MAX_RPM_STR[] PROGMEM =  "max rpm  : ";          
            static const char MAX_TEMP_STR[] PROGMEM = "max temp : ";
            static const char MIN_RSSI_STR[] PROGMEM = "min rssi : ";         
  
            statCol = settings.COLS/2 - (strlen_P(TIME_STR)+7)/2;
            OSD.printP(statCol, ++middle_infos_y, TIME_STR);
            OSD.printTime( statCol+strlen_P(TIME_STR), middle_infos_y, total_time);  
            OSD.printInt16P( statCol, ++middle_infos_y, MAX_AMP_STR, MaxAmps, 2, "a" );
            #ifdef ADVANCED_ESC_STATS
            for(i=0; i<4; i++) avgTotal += ESCstatGenerators[i].GetAverage();
            OSD.printInt16P( statCol, ++middle_infos_y, MAX_AVG_STR, avgTotal, 2, "a" );
            #endif
            OSD.printInt16P( statCol, ++middle_infos_y, MIN_V_STR, MinBat, 2, "v" );
            OSD.printInt16P( statCol, ++middle_infos_y, MAX_WATT_STR, MaxWatt, 1, "w" ); //OSD.printInt16( OSD.cursorRow()+1, middle_infos_y, settings.m_maxWatts, 1, "w)", 0, 0, "(" );          
            OSD.printInt16P( statCol, ++middle_infos_y, MAX_C_STR, MaxC, 0, "c");
            OSD.printInt16P( statCol, ++middle_infos_y, MAH_STR, statMAH, 0, "mah" );
            OSD.printInt16P( statCol, ++middle_infos_y, MAX_RPM_STR, MaxRPMs, 1, "kr" );            
            OSD.printInt16P( statCol, ++middle_infos_y, MAX_TEMP_STR, MaxTemp, 0, tempSymbol);
            if(settings.m_RSSIchannel > -1)
            {
              OSD.printInt16P( statCol, ++middle_infos_y, MIN_RSSI_STR, MinRSSI, 0, "db");                    
            }
            OSD.printInt16( 0, 1, LipoVoltage, 2, "v");
          break;
          case 1:
          #ifdef ADVANCED_STATS
            static const char STAT_GEN_STRS[][17] PROGMEM  = { {"5-20   thr avg: "},                                                               
                                                               {"20-40  thr avg: "},                                                                      
                                                               {"40-60  thr avg: "},                                                               
                                                               {"60-80  thr avg: "},
                                                               {"80-100 thr avg: "},
                                                               {"95-100 thr avg: "},
                                                               {"5-20   thr max: "},
                                                               {"20-40  thr max: "},
                                                               {"40-60  thr max: "},          
                                                               {"60-80  thr max: "} };          
  
            statCol = settings.COLS/2 - (strlen_P(STAT_GEN_STRS[0])+7)/2;
            j = 0;
            for(i=0; i<MAX_SETTING_MODES; i++)
            {
              OSD.printInt16P( statCol, ++middle_infos_y, STAT_GEN_STRS[j], statGenerators[i].GetAverage(), 2, "a" );
              j++;
            }
            for(i=0; i<4; i++)
            {
              OSD.printInt16P( statCol, ++middle_infos_y, STAT_GEN_STRS[j], statGenerators[i].m_Max, 2, "a" );
              j++;
            }
          break;
          #endif
          case 2:
          case 3:
          case 4:
          case 5:            
            char* ESC_STAT_STR = ESC_STAT_STR1;
            if(settings.m_displaySymbols == 1 && settings.m_IconSettings[ESC_ICON] == 1)
            {
              ESC_STAT_STR = ESCSymbol;
            }          
            static const char ESC_A_STR[] PROGMEM         =  " max a   : ";
            #ifdef ADVANCED_ESC_STATS
            static const char ESC_AVG_MAX_A_STR[] PROGMEM =  " avg max : ";
            #endif          
            static const char ESC_MINV_STR[] PROGMEM      =  " min v   : ";
            static const char ESC_RPM_STR[] PROGMEM       =  " max rpm : ";
            static const char ESC_TEMP_STR[] PROGMEM      =  " max temp: ";
            static const char ESC_MAH_STR[] PROGMEM       =  " mah     : ";            
            
            uint8_t startCol = settings.COLS/2 - (strlen_P(ESC_RPM_STR)+strlen(ESC_STAT_STR)+7)/2;
            #ifdef ADVANCED_STATS
            uint8_t ESCnumber = statPage - 1;
            #else
            uint8_t ESCnumber = statPage;
            #endif                    
            OSD.printInt16( startCol, ++middle_infos_y, ESC_STAT_STR, ESCnumber, 0);
            OSD.printInt16P( startCol + strlen(ESC_STAT_STR) + 1, middle_infos_y, ESC_A_STR, maxCurrent[ESCnumber-1], 2, "a");

            #ifdef ADVANCED_ESC_STATS
            OSD.printInt16( startCol, ++middle_infos_y, ESC_STAT_STR, ESCnumber, 0);
            OSD.printInt16P( startCol + strlen(ESC_STAT_STR) + 1, middle_infos_y, ESC_AVG_MAX_A_STR, ESCstatGenerators[ESCnumber-1].GetAverage(), 2, "a");
            #endif
  
            OSD.printInt16( startCol, ++middle_infos_y, ESC_STAT_STR, ESCnumber, 0);
            OSD.printInt16P( startCol + strlen(ESC_STAT_STR) + 1, middle_infos_y, ESC_MINV_STR, minVoltage[ESCnumber-1], 2, "v");
  
            OSD.printInt16( startCol, ++middle_infos_y, ESC_STAT_STR, ESCnumber, 0);
            OSD.printInt16P( startCol + strlen(ESC_STAT_STR) + 1, middle_infos_y, ESC_RPM_STR, maxKERPM[ESCnumber-1], 1, "kr");
            
            OSD.printInt16( startCol, ++middle_infos_y, ESC_STAT_STR, ESCnumber, 0);
            OSD.printInt16P( startCol + strlen(ESC_STAT_STR) + 1, middle_infos_y, ESC_TEMP_STR, maxTemps[ESCnumber-1], 0, tempSymbol);                    
            
            OSD.printInt16( startCol, ++middle_infos_y, ESC_STAT_STR, ESCnumber, 0);
            OSD.printInt16P( startCol + strlen(ESC_STAT_STR) + 1, middle_infos_y, ESC_MAH_STR, ESCmAh[ESCnumber-1], 0, "mah");             
          break;
        }
     }
     else 
      {
        #ifdef SIMULATE_VALUES
        static int16_t currentRT2 = 0;
        static int16_t LipoMAH2 = 0;
        static int16_t rssi2 = 0;
        if(doItOnce && timer1sec)
        {
          currentRT2 += 50;
          if(currentRT2 > 1000) currentRT2 = 0;
          LipoMAH2 += 100;
          if(LipoMAH2 > settings.m_batMAH[settings.m_activeBattery]) LipoMAH2 = 0;
          rssi2 += 10;
          if(rssi2 > 100) rssi2 = 0;
          doItOnce = false;
        }
        if(!timer1sec) doItOnce = true;
        currentRT = currentRT2;
        LipoMAH = LipoMAH2;
        AuxChanVals[settings.m_RSSIchannel] = rssi2;
        #endif 

        printCount = 0;
         
        if(armed == 0 && armedOnce && last_Aux_Val != AuxChanVals[settings.m_DVchannel]) 
        {
          DV_change_time = _millis;
          last_Aux_Val = AuxChanVals[settings.m_DVchannel];
        }
        
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_RC_THROTTLE])
        {
          printCount++;
          OSD.printInt16( settings.m_OSDItems[THROTTLE][0], settings.m_OSDItems[THROTTLE][1], throttle, 0, "%", 2, THROTTLEp);
        }
          
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_NICKNAME])
        {
          printCount++;
          OSD.checkPrintLength(settings.m_OSDItems[NICKNAME][0], settings.m_OSDItems[NICKNAME][1], strlen(settings.m_nickname), zeroBlanks, NICKNAMEp);
          OSD.print( fixStr(settings.m_nickname) );          
        }        
    
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_COMB_CURRENT])
        {
          printCount++;
          #ifdef WATTMETER
          if(settings.m_wattMeter > 0)
          {
            const int16_t wattPercentage = 20; //95% of max
            uint32_t Watts = (uint32_t)LipoVoltage * (uint32_t)current;            
            int16_t tempWatt = (int16_t)(Watts/1000);
            if(settings.m_displaySymbols == 1 && settings.m_IconSettings[WATT_ICON] == 1)
            {
              int8_t wattRow2 = settings.m_OSDItems[AMPS][1]-1;            
              if(wattRow2 < 0) wattRow2 = 0;
              int8_t wattRow1 = wattRow2 + 1;
              uint8_t steps;
              switch(settings.m_wattMeter)
              {              
                case 1:
                #ifndef BEERMUG
                case 2:
                #endif
                  iconPrintBuf1[0] = 0xD7;
                  iconPrintBuf2[0] = 0xE3;
                  steps = 8;              
                break;
                #ifdef BEERMUG
                case 2:
                  iconPrintBuf1[0] = 0x01;
                  iconPrintBuf2[0] = 0x09;
                  steps = 6;
                  wattRow1--;
                  wattRow2++;
                break;
                #endif
              }
              getIconPos(tempWatt, settings.m_maxWatts - settings.m_maxWatts/wattPercentage, steps, iconPrintBuf1[0], iconPrintBuf2[0], 2);
              if((uint8_t)iconPrintBuf2[0] > 0xE3) iconPrintBuf1[0] = 0xE1;
              if((uint8_t)iconPrintBuf1[0] == 0x07 && (uint8_t)iconPrintBuf2[0] == 0x09) iconPrintBuf2[0] = 0x0B;
              iconPrintBuf1[1] = iconPrintBuf1[0] + 1;
              iconPrintBuf2[1] = iconPrintBuf2[0] + 1;
              OSD.checkPrintLength(settings.m_OSDItems[AMPS][0], wattRow1, 2, zeroBlanks, AMPSp);
              OSD.print(iconPrintBuf1);
              OSD.checkPrintLength(settings.m_OSDItems[AMPS][0], wattRow2, 2, zeroBlanks, AMPSp);
              OSD.print(iconPrintBuf2);
            }
            else
            {
              tempWatt /= 10;
              if(tempWatt > 1000) OSD.printInt16(settings.m_OSDItems[AMPS][0], settings.m_OSDItems[AMPS][1], tempWatt/100, 1, "kw", 1, AMPSp);
              else OSD.printInt16(settings.m_OSDItems[AMPS][0], settings.m_OSDItems[AMPS][1], tempWatt, 0, "w", 1, AMPSp);
            }
          }
          else
          #endif
          {
            OSD.printInt16(settings.m_OSDItems[AMPS][0], settings.m_OSDItems[AMPS][1], current/10, 1, "a", 2, AMPSp);
          }
        }
        
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_LIPO_VOLTAGE])
        {
          printCount++;
          OSD.printInt16( settings.m_OSDItems[VOLTAGE][0], settings.m_OSDItems[VOLTAGE][1], LipoVoltage / 10, 1, "v", 1, VOLTAGEp);
        }
        
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_MA_CONSUMPTION])
        {
          printCount++;
          #ifdef _MAH_ICON        
          if(settings.m_displaySymbols == 1 && settings.m_IconSettings[MAH_ICON] == 1)
          {            
            batteryIcon[1] = 0x84;
            batteryIcon[2] = 0x84;
            remainMAH = settings.m_batMAH[settings.m_activeBattery]-statMAH;
            if(statMAH > settings.m_batMAH[settings.m_activeBattery]) remainMAH = 0;
            getIconPos(remainMAH, settings.m_batMAH[settings.m_activeBattery], 8, batteryIcon[2], batteryIcon[1]);
            OSD.checkPrintLength(settings.m_OSDItems[MAH][0], settings.m_OSDItems[MAH][1], 4, zeroBlanks, MAHp);
            OSD.print(batteryIcon);           
          }
          else
          #endif
          {
            OSD.printInt16(settings.m_OSDItems[MAH][0], settings.m_OSDItems[MAH][1], statMAH, 0, "mah", 0, MAHp);
          }
        }
  
        if(settings.m_displaySymbols == 1 && settings.m_IconSettings[ESC_ICON] == 1)
        {
          ESCSymbol[0] = fixChar((char)0x7E);
        }
        else
        {
          ESCSymbol[0] = 0x00;
        }

        
        uint8_t TMPmargin          = 0;
        uint8_t CurrentMargin      = 0;
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_ESC_KRPM])
        {
          printCount++;
          #ifdef PROP_ICON
          static char KR[4][2];
          if(settings.m_displaySymbols == 1 && settings.m_IconSettings[PROPS_ICON] == 1)
          {
            for(i=0; i<4; i++)
            {
              if(_millis - krTime[i] > (10000/motorKERPM[i]))
              {
                krTime[i] = _millis;
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
              KR[i][0] = (char)krSymbol[i];
              KR[i][1] = 0x00;
            }
            OSD.checkPrintLength(settings.m_OSDItems[ESC1kr][0], settings.m_OSDItems[ESC1kr][1], 1, zeroBlanks, ESC1kr);
            OSD.print(KR[0]);
            OSD.checkPrintLength(settings.m_OSDItems[ESC2kr][0], settings.m_OSDItems[ESC2kr][1], 1, zeroBlanks, ESC2kr);
            OSD.print(KR[1]);
            OSD.checkPrintLength(settings.m_OSDItems[ESC3kr][0], settings.m_OSDItems[ESC3kr][1], 1, zeroBlanks, ESC3kr);
            OSD.print(KR[2]);
            OSD.checkPrintLength(settings.m_OSDItems[ESC4kr][0], settings.m_OSDItems[ESC4kr][1], 1, zeroBlanks, ESC4kr);
            OSD.print(KR[3]);
          }
          else
          #endif
          {
            static char KR2[4];
            KR2[0] = 'k';
            KR2[1] = 'r';
            KR2[2] = 0x00;
            KR2[3] = 0x00;
            OSD.printInt16(settings.m_OSDItems[ESC1kr][0], settings.m_OSDItems[ESC1kr][1], motorKERPM[0], 1, KR2, 1, ESC1kr, ESCSymbol);
            KR2[2] = ESCSymbol[0];
            OSD.printInt16(settings.m_OSDItems[ESC2kr][0], settings.m_OSDItems[ESC2kr][1], motorKERPM[1], 1, KR2, 1, ESC2kr);
            OSD.printInt16(settings.m_OSDItems[ESC3kr][0], settings.m_OSDItems[ESC3kr][1], motorKERPM[2], 1, KR2, 1, ESC3kr);
            KR2[2] = 0x00;
            OSD.printInt16(settings.m_OSDItems[ESC4kr][0], settings.m_OSDItems[ESC4kr][1], motorKERPM[3], 1, KR2, 1, ESC4kr, ESCSymbol);
          }
          
          TMPmargin++;
          CurrentMargin++;
        }
     
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_ESC_CURRENT])
        {
          printCount++;
          static char ampESC[3];
          ampESC[0] = 'a';
          ampESC[1] = ESCSymbol[0];
          ampESC[2] = 0x00;
          OSD.printInt16(settings.m_OSDItems[ESC1voltage][0], settings.m_OSDItems[ESC1voltage][1]+CurrentMargin, motorCurrent[0]/10, 1, "a", 1, ESC1voltage, ESCSymbol);
          OSD.printInt16(settings.m_OSDItems[ESC2voltage][0], settings.m_OSDItems[ESC2voltage][1]+CurrentMargin, motorCurrent[1]/10, 1, ampESC, 1, ESC2voltage);
          OSD.printInt16(settings.m_OSDItems[ESC3voltage][0], settings.m_OSDItems[ESC3voltage][1]-CurrentMargin, motorCurrent[2]/10, 1, ampESC, 1, ESC3voltage);
          OSD.printInt16(settings.m_OSDItems[ESC4voltage][0], settings.m_OSDItems[ESC4voltage][1]-CurrentMargin, motorCurrent[3]/10, 1, "a", 1, ESC4voltage, ESCSymbol);
          TMPmargin++;
        }
    
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_ESC_TEMPERATURE])
        {
          printCount++;
          static char tempESC[3];
          tempESC[0] = tempSymbol[0];
          tempESC[1] = ESCSymbol[0];
          tempESC[2] = 0x00;
          OSD.printInt16(settings.m_OSDItems[ESC1temp][0], settings.m_OSDItems[ESC1temp][1]+TMPmargin, ESCTemps[0], 0, tempSymbol, 1, ESC1temp, ESCSymbol);
          OSD.printInt16(settings.m_OSDItems[ESC2temp][0], settings.m_OSDItems[ESC2temp][1]+TMPmargin, ESCTemps[1], 0, tempESC, 1, ESC2temp);
          OSD.printInt16(settings.m_OSDItems[ESC3temp][0], settings.m_OSDItems[ESC3temp][1]-TMPmargin, ESCTemps[2], 0, tempESC, 1, ESC3temp);
          OSD.printInt16(settings.m_OSDItems[ESC4temp][0], settings.m_OSDItems[ESC4temp][1]-TMPmargin, ESCTemps[3], 0, tempSymbol, 1, ESC4temp, ESCSymbol);
        }
    
        if(AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_TIMER]) 
        {
          printCount++;
          static char stopWatchStr[] = { 0x00, 0x00 };
          #ifdef STOPWATCH_ICON
          if(settings.m_displaySymbols == 1 && settings.m_IconSettings[TIMER_ICON] == 1)
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
          #endif
          if(settings.m_timerMode == 2 && (time / 1000) > 109) 
          {
            OSD.blink1sec();
          }
          OSD.printTime(settings.m_OSDItems[STOPW][0], settings.m_OSDItems[STOPW][1], time, stopWatchStr, STOPWp);
        }                

        #ifdef CROSSHAIR
        if(settings.m_crossHair && (logoDone || armed > 0) && !vTxPowerActive)
        {
          int8_t crossOffset = 0;
          if(settings.m_crossHair > 1) crossOffset = (int8_t)settings.m_crossHair - (int8_t)5;
          OSD.setCursor(settings.COLS/2 - 1, (int8_t)(settings.ROWS/2) + crossOffset);
          OSD.print(crossHairSymbol);
        }
        #endif

        #ifdef RSSI_
        static int16_t rssiVal;
        if(settings.m_RSSIchannel > -1)
        {
          rssiVal = AuxChanVals[settings.m_RSSIchannel];
          if(rssiVal > 100)
          {
            rssiVal = rssiFilter.ProcessValue(rssiVal);
            if(settings.m_RSSImax > -1001 && settings.m_RSSImin > -1001)
            {
              int16_t rssiMin = settings.m_RSSImin+1000;
              rssiVal = ((rssiVal+1000)-rssiMin)/((settings.m_RSSImax+1000)-rssiMin);
            }
            else while(rssiVal > 100) rssiVal /= 10;          
          }
          if(MinRSSI > rssiVal && armedOnce) MinRSSI = rssiVal;
        }
        if(settings.m_RSSIchannel > -1 && AuxChanVals[settings.m_DVchannel] > DV_PPMs[DISPLAY_RSSI])
        {                                       
          //OSD.printInt16(0, settings.ROWS/2, rssiVal, 0, "db", 1);
          printCount++;
          if(rssiVal < 45 && timer1sec)
          {
            uint8_t spaces = 5;
            if(settings.m_displaySymbols == 1 && settings.m_IconSettings[RSSI_ICON] == 1) spaces = 2;
            OSD.checkPrintLength(settings.m_OSDItems[RSSIp][0], settings.m_OSDItems[RSSIp][1], 2, zeroBlanks, RSSIp);
            OSD.printSpaces(spaces);
          }
          else
          {
            #ifdef _RSSI_ICON
            if(settings.m_displaySymbols == 1 && settings.m_IconSettings[RSSI_ICON] == 1)
            {
              static const uint8_t maxRSSIvalue = 50;
              iconPrintBuf1[0] = 0x12;
              iconPrintBuf1[1] = 0x15;
              rssiVal -= 30;
              getIconPos(rssiVal, maxRSSIvalue, 5, iconPrintBuf1[0], iconPrintBuf1[1]);
              if(iconPrintBuf1[0] < 0x14) iconPrintBuf1[1] = 0x20;
              OSD.checkPrintLength(settings.m_OSDItems[RSSIp][0], settings.m_OSDItems[RSSIp][1], 2, zeroBlanks, RSSIp);
              OSD.print(iconPrintBuf1);
            }
            else
            #endif
            {
              OSD.printInt16(settings.m_OSDItems[RSSIp][0], settings.m_OSDItems[RSSIp][1], rssiVal, 0, "db", 1, RSSIp);
            }
          }
        }
        #endif
        
        if(settings.m_batWarning > 0 && statMAH >= settings.m_batWarningMAH)
        {
          totalMAH = 0;
          static const char BATTERY_LOW[] PROGMEM =   "battery low";
          if(timer1sec) 
          {
            if(batWarnSymbol)
            {
              if(settings.m_displaySymbols == 1)// && settings.m_IconSettings[MAH_ICON] == 1)
              {
                OSD.setCursor(settings.COLS/2 - 2, settings.ROWS/2 + 3);
                OSD.print(batteryIcon);            
              }
              else
              {
                OSD.printP(settings.COLS/2 - strlen_P(BATTERY_LOW)/2, settings.ROWS/2 +3, BATTERY_LOW);
              }              
            }
            else
            {
              OSD.printInt16(settings.COLS/2 - 3, settings.ROWS/2 + 3, statMAH, 0, "mah");              
            }
            flipOnce = true;
          }
          else 
          {
            if(flipOnce)
            {
              batWarnSymbol = !batWarnSymbol;
              flipOnce = false;              
            }
            OSD.setCursor(settings.COLS/2 - strlen_P(BATTERY_LOW)/2, settings.ROWS/2 +3);
            OSD.printSpaces(11);
          }
        }
        else
        {
          if(settings.m_batWarning > 0)
          {
            totalMAH = statMAH;
          }
          else
          {
            totalMAH = 0;
          }
        }

        if(armed > 0)
        {
          if(settings.m_voltWarning > 0 && settings.m_minVolts > (LipoVoltage / 10) && !timer1sec)
          {
            OSD.printInt16(settings.COLS/2 - 3, settings.ROWS/2 + 2, LipoVoltage / 10, 1, "v", 1);                      
          }
          else
          {
            OSD.setCursor(settings.COLS/2 - 3, settings.ROWS/2 + 2);
            OSD.printSpaces(5);
          }        
        }
        
        if(DV_change_time > 0 && (_millis - DV_change_time) > 3000 && last_Aux_Val == AuxChanVals[settings.m_DVchannel]) 
        {
          DV_change_time = 0;
          cleanScreen();
        }

        if(symbolOnOffChanged)
        {
          cleanScreen();
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
