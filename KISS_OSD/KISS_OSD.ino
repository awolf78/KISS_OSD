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
#define LOCATION_BAT_WARNING_MAH_LSB	0x03
#define LOCATION_BAT_WARNING_MAH_MSB	0x04
//#define DEBUG

const char KISS_OSD_VER[] = "kiss osd v2.0";

#include <SPI.h>
#include <MAX7456.h>
#include <EEPROM.h>
#include "printInt16_t.h"
#include "SerialPort.h"
#include "CSettings.h"
#include "CStickInput.h"
#include "Flash.h"
#include "fixFont.h"


const byte osdChipSelect             =            6;
const byte masterOutSlaveIn          =            MOSI;
const byte masterInSlaveOut          =            MISO;
const byte slaveClock                =            SCK;
const byte osdReset                  =            2;

MAX7456 OSD( osdChipSelect );
SerialPort<0, 64, 0> NewSerial;
CSettings settings;
CStickInput inputChecker;

#ifdef PAL
static const uint8_t ROWS = MAX7456_ROWS_P1;
static const uint8_t COLS = MAX7456_COLS_P1;
#else
static const uint8_t ROWS = MAX7456_ROWS_N0;
static const uint8_t COLS = MAX7456_COLS_N1;
#endif

static char clean[COLS];

int16_t setupPPM(int16_t pos) 
{
  if(pos < 0) 
  {
    return 10000;
  }
  return -1000 + (pos * 100);
}

void cleanScreen() 
{
  uint8_t i;
 
  for(i=0;i<ROWS;i++)
  {
      OSD.setCursor( 0, i );
      OSD.print( clean );
  }
}

void setup()
{
  uint8_t i = 0;
  SPI.begin();
  SPI.setClockDivider( SPI_CLOCK_DIV2 ); 
  //OSD.begin();
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
  
  //clean used area
  for(i=0;i<COLS;i++) clean[i] = fixChar(' ');
  while (!OSD.notInVSync());
  cleanScreen();
  settings.ReadSettings();
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
  delay(3000); // Wait until FC is ready - otherwise we get garbage data
}

static int16_t  throttle = 0;
static int16_t  roll = 0;
static int16_t  pitch = 0;
static int16_t  yaw = 0;
static uint16_t current = 0;
static int8_t armed = 0;
static int8_t idleTime = 0;
static uint16_t LipoVoltage = 0;
static uint16_t LipoMAH = 0;
static int16_t  previousMAH = 0;
static int16_t totalMAH = 0;
static uint16_t MaxAmps = 0;
static uint16_t MaxC    = 0;
static uint16_t MaxRPMs = 0;
static uint16_t MaxWatt = 0;
static uint16_t MaxTemp = 0;
static uint16_t MinBat = 0;
static int16_t motorKERPM[4] = {0,0,0,0};
static int16_t maxKERPM[4] = {0,0,0,0};
static int16_t motorCurrent[4] = {0,0,0,0};
static int16_t maxCurrent[4] = {0,0,0,0};
static int16_t ESCTemps[4] = {0,0,0,0};
static int16_t maxTemps[4] = {0,0,0,0};
static int16_t  AuxChanVals[4] = {0,0,0,0};
static unsigned long start_time = 0;
static unsigned long time = 0;
static unsigned long total_time = 0;
static unsigned long DV_change_time = 0;
static boolean bat_clear = true;
static int16_t last_Aux_Val = -10000;
static boolean armedOnce = false;
static boolean triggerCleanScreen = false;
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

uint16_t findMax(uint16_t maxV, uint16_t newVal) 
{
  if(newVal > maxV) {
    return newVal;
  }
  return maxV;
}

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


static int16_t lastAuxVal = 0;  
static uint8_t serialBuf[256];
static uint8_t minBytes = 0;
static uint8_t recBytes = 0;
static uint32_t LastLoopTime = 0;

void ReadTelemetry()
{
  uint16_t i = 0;
  minBytes = 100;
  recBytes = 0;
   
  while(recBytes < minBytes && micros()-LastLoopTime < 20000)
  {
    #define STARTCOUNT 2
    while(NewSerial.available()) serialBuf[recBytes++] = NewSerial.read();
    if(recBytes == 1 && serialBuf[0] != 5)recBytes = 0; // check for start byte, reset if its wrong
    if(recBytes == 2) minBytes = serialBuf[1]+STARTCOUNT+1; // got the transmission length
    if(recBytes == minBytes){
       uint32_t checksum = 0;
       for(i=2;i<minBytes;i++){
          checksum += serialBuf[i];
       }
       checksum = (uint32_t)checksum/(minBytes-3);
       
       if(checksum == serialBuf[recBytes-1]){
        
         throttle = ((serialBuf[STARTCOUNT]<<8) | serialBuf[1+STARTCOUNT])/10;
         roll = 1000 + ((serialBuf[2+STARTCOUNT]<<8) | serialBuf[3+STARTCOUNT]);
         pitch = 1000 + ((serialBuf[4+STARTCOUNT]<<8) | serialBuf[5+STARTCOUNT]);
         yaw = 1000 + ((serialBuf[6+STARTCOUNT]<<8) | serialBuf[7+STARTCOUNT]);
         LipoVoltage =   ((serialBuf[17+STARTCOUNT]<<8) | serialBuf[18+STARTCOUNT]);

         int8_t current_armed = serialBuf[16+STARTCOUNT];
         // switch disarmed => armed
         if (armed == 0 && current_armed > 0) 
         {
           start_time = millis();
	   triggerCleanScreen = true;
           armedOnce = true;
           last_Aux_Val = AuxChanVals[settings.m_DVchannel];
           DV_change_time = 0;
         }
         // switch armed => disarmed
         else 
         {
           if (armed > 0 && current_armed == 0) 
           {
             total_time = total_time + (millis() - start_time);
             start_time = 0;
             triggerCleanScreen = true;
             if(settings.m_batWarning > 0)
             {
               settings.m_lastMAH = totalMAH;
               settings.WriteLastMAH();
               settings.m_lastMAH = 0;
             }
             else
             {
               settings.m_lastMAH = 0;
               settings.WriteLastMAH();
             }
           } 
           else if (armed > 0) 
           {
             time = millis() - start_time;
           }
         }
         armed = current_armed;
         
         uint32_t tmpVoltage = 0;
         uint32_t voltDev = 0;
         if(((serialBuf[85+STARTCOUNT]<<8) | serialBuf[86+STARTCOUNT]) > 5){ // the ESC's read the voltage better then the FC
           tmpVoltage += ((serialBuf[85+STARTCOUNT]<<8) | serialBuf[86+STARTCOUNT]);
           voltDev++;
         }
         if(((serialBuf[95+STARTCOUNT]<<8) | serialBuf[96+STARTCOUNT]) > 5){ 
           tmpVoltage += ((serialBuf[95+STARTCOUNT]<<8) | serialBuf[96+STARTCOUNT]);
           voltDev++;
         }
         if(((serialBuf[105+STARTCOUNT]<<8) | serialBuf[106+STARTCOUNT]) > 5){
           tmpVoltage += ((serialBuf[105+STARTCOUNT]<<8) | serialBuf[106+STARTCOUNT]);
           voltDev++;
         }
         if(((serialBuf[115+STARTCOUNT]<<8) | serialBuf[116+STARTCOUNT]) > 5){ 
           tmpVoltage += ((serialBuf[115+STARTCOUNT]<<8) | serialBuf[116+STARTCOUNT]);
           voltDev++;
         }
         if(((serialBuf[125+STARTCOUNT]<<8) | serialBuf[126+STARTCOUNT]) > 5){
           tmpVoltage += ((serialBuf[125+STARTCOUNT]<<8) | serialBuf[126+STARTCOUNT]);
           voltDev++;
         }
         
         if(voltDev!=0) LipoVoltage = tmpVoltage/voltDev;           
         
         LipoMAH =       ((serialBuf[148+STARTCOUNT]<<8) | serialBuf[149+STARTCOUNT]); 
         
         static uint32_t windedupfilterdatas[8];
         
         windedupfilterdatas[0] = ESC_filter((uint32_t)windedupfilterdatas[0],(uint32_t)((serialBuf[91+STARTCOUNT]<<8) | serialBuf[92+STARTCOUNT])/(MAGNETPOLECOUNT/2)<<4);
         windedupfilterdatas[1] = ESC_filter((uint32_t)windedupfilterdatas[1],(uint32_t)((serialBuf[101+STARTCOUNT]<<8) | serialBuf[102+STARTCOUNT])/(MAGNETPOLECOUNT/2)<<4);
         windedupfilterdatas[2] = ESC_filter((uint32_t)windedupfilterdatas[2],(uint32_t)((serialBuf[111+STARTCOUNT]<<8) | serialBuf[112+STARTCOUNT])/(MAGNETPOLECOUNT/2)<<4);
         windedupfilterdatas[3] = ESC_filter((uint32_t)windedupfilterdatas[3],(uint32_t)((serialBuf[121+STARTCOUNT]<<8) | serialBuf[122+STARTCOUNT])/(MAGNETPOLECOUNT/2)<<4);
         
         motorKERPM[0] = windedupfilterdatas[0]>>4;
         motorKERPM[1] = windedupfilterdatas[1]>>4;
         motorKERPM[2] = windedupfilterdatas[2]>>4;
         motorKERPM[3] = windedupfilterdatas[3]>>4;           
         
         windedupfilterdatas[4] = ESC_filter((uint32_t)windedupfilterdatas[4],(uint32_t)((serialBuf[87+STARTCOUNT]<<8) | serialBuf[88+STARTCOUNT])<<4);
         windedupfilterdatas[5] = ESC_filter((uint32_t)windedupfilterdatas[5],(uint32_t)((serialBuf[97+STARTCOUNT]<<8) | serialBuf[98+STARTCOUNT])<<4);
         windedupfilterdatas[6] = ESC_filter((uint32_t)windedupfilterdatas[6],(uint32_t)((serialBuf[107+STARTCOUNT]<<8) | serialBuf[108+STARTCOUNT])<<4);
         windedupfilterdatas[7] = ESC_filter((uint32_t)windedupfilterdatas[7],(uint32_t)((serialBuf[117+STARTCOUNT]<<8) | serialBuf[118+STARTCOUNT])<<4);
         
         motorCurrent[0] = windedupfilterdatas[4]>>4;
         motorCurrent[1] = windedupfilterdatas[5]>>4;
         motorCurrent[2] = windedupfilterdatas[6]>>4;
         motorCurrent[3] = windedupfilterdatas[7]>>4;
         
         
         ESCTemps[0] = ((serialBuf[83+STARTCOUNT]<<8) | serialBuf[84+STARTCOUNT]);
         ESCTemps[1] = ((serialBuf[93+STARTCOUNT]<<8) | serialBuf[94+STARTCOUNT]);
         ESCTemps[2] = ((serialBuf[103+STARTCOUNT]<<8) | serialBuf[104+STARTCOUNT]);
         ESCTemps[3] = ((serialBuf[113+STARTCOUNT]<<8) | serialBuf[114+STARTCOUNT]);           

         AuxChanVals[0] = ((serialBuf[8+STARTCOUNT]<<8) | serialBuf[9+STARTCOUNT]);
         AuxChanVals[1] = ((serialBuf[10+STARTCOUNT]<<8) | serialBuf[11+STARTCOUNT]);
         AuxChanVals[2] = ((serialBuf[12+STARTCOUNT]<<8) | serialBuf[13+STARTCOUNT]);
         AuxChanVals[3] = ((serialBuf[14+STARTCOUNT]<<8) | serialBuf[15+STARTCOUNT]);
         
         current = (uint16_t)(motorCurrent[0]+motorCurrent[1]+motorCurrent[2]+motorCurrent[3])/10;
         
         uint8_t i;
         if(settings.m_tempUnit == 1)
         {
           for(i=0; i<4; i++)
           {
             ESCTemps[i] = 9 * ESCTemps[i] / 5 + 32;
           }
         }
         if(armedOnce) 
         {
           MaxTemp = findMax4(MaxTemp, ESCTemps, 4);
           MaxRPMs = findMax4(MaxRPMs, motorKERPM, 4);
           if (MinBat == 0)
           {
             MinBat = LipoVoltage;
           }
           else if (LipoVoltage < MinBat)
           {
             MinBat = LipoVoltage;
           }
           MaxAmps = findMax(MaxAmps, current);
           uint32_t temp = (uint32_t)MaxAmps * 100 / (uint32_t)settings.m_batMAH[settings.m_activeBattery];
           MaxC = (int16_t) temp;
           uint32_t Watts = (uint32_t)LipoVoltage * (uint32_t)(current * 10);
           MaxWatt = findMax(MaxWatt, (uint16_t) (Watts / 1000));
           for(i=0; i<4; i++)
           {
             maxKERPM[i] = findMax(maxKERPM[i], motorKERPM[i]);
             maxCurrent[i] = findMax(maxCurrent[i], motorCurrent[i]);
             maxTemps[i] = findMax(maxTemps[i], ESCTemps[i]);
           }
         }
      }
    }
  }
}

static int16_t p_roll = 0;
static int16_t p_pitch, p_yaw, p_tpa, i_roll, i_pitch, i_yaw, i_tpa, d_roll, d_pitch, d_yaw, d_tpa;
static int16_t rcrate_roll, rate_roll, rccurve_roll, rcrate_pitch, rate_pitch, rccurve_pitch, rcrate_yaw, rate_yaw, rccurve_yaw;
static uint8_t minBytesSettings = 0;
static boolean fcSettingsReceived = false;
static uint8_t serialBuf2[256];

void ReadFCSettings(boolean skipValues = false)
{
  uint8_t i = 0;
  minBytes = 100;
  recBytes = 0;
   
  while(recBytes < minBytes && micros()-LastLoopTime < 20000)
  {
    #define STARTCOUNT 2
    while(NewSerial.available()) serialBuf2[recBytes++] = NewSerial.read();
    if(recBytes == 1 && serialBuf2[0] != 5)recBytes = 0; // check for start byte, reset if its wrong
    if(recBytes == 2) minBytes = serialBuf2[1]+STARTCOUNT+1; // got the transmission length
    if(recBytes == minBytes)
    {
       uint32_t checksum = 0;
       for(i=2;i<minBytes;i++){
          checksum += serialBuf2[i];
       }
       checksum = (uint32_t)checksum/(minBytes-3);
       
       if(checksum == serialBuf2[recBytes-1])
       {
         minBytesSettings = minBytes;
         fcSettingsReceived = true;
         if(!skipValues)
         {
           uint8_t index = 0;
           p_roll = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           p_pitch = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           p_yaw = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           i_roll = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           i_pitch = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           i_yaw = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           d_roll = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           d_pitch = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           d_yaw = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           
           index = 28;
           rcrate_roll = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           rcrate_pitch = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           rcrate_yaw = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           rate_roll = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           rate_pitch = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           rate_yaw = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           rccurve_roll = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           rccurve_pitch = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           rccurve_yaw = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           
           index = 93;
           p_tpa = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           i_tpa = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           d_tpa = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
         }
       }
    }
  }
}

boolean SendFCSettings()
{
  if(fcSettingsReceived)
  {
    #define STARTCOUNT 2
    uint8_t index = 0;
    
    serialBuf2[STARTCOUNT+index++] = (byte)((p_roll & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(p_roll & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((p_pitch & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(p_pitch & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((p_yaw & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(p_yaw & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((i_roll & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(i_roll & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((i_pitch & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(i_pitch & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((i_yaw & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(i_yaw & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((d_roll & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(d_roll & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((d_pitch & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(d_pitch & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((d_yaw & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(d_yaw & 0x00FF);
    
    index = 28;
    serialBuf2[STARTCOUNT+index++] = (byte)((rcrate_roll & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(rcrate_roll & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((rcrate_pitch & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(rcrate_pitch & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((rcrate_yaw & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(rcrate_yaw & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((rate_roll & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(rate_roll & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((rate_pitch & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(rate_pitch & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((rate_yaw & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(rate_yaw & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((rccurve_roll & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(rccurve_roll & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((rccurve_pitch & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(rccurve_pitch & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((rccurve_yaw & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(rccurve_yaw & 0x00FF);
    
    index = 93;
    serialBuf2[STARTCOUNT+index++] = (byte)((p_tpa & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(p_tpa & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((i_tpa & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(i_tpa & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((d_tpa & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(d_tpa & 0x00FF);
    
    uint32_t checksum = 0;
    uint16_t i;
    for(i=2;i<minBytesSettings;i++)
    {
     checksum += serialBuf2[i];
    }
    checksum = (uint32_t)checksum/(minBytesSettings-3);
    serialBuf2[minBytesSettings-1] = checksum;
    
    NewSerial.write(0x10); //Set settings
    for(i=0;i<minBytesSettings;i++)
    {
      NewSerial.write(serialBuf2[i]);
    }
    return true;
  }
  return false;
}


typedef void* (*fptr)();
extern void* MainMenu();
static char printBuf[15];
static char tempSymbol = 0x30;
FLASH_STRING(TWO_BLANKS, "  ");
static uint8_t code = 0;
static boolean menuActive = false;
static boolean menuWasActive = false;
static boolean fcSettingChanged = false;
static uint8_t activeMenuItem = 0;
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

boolean RatesGeneralMenu(uint8_t &activeGeneralMenuItem, int16_t &rcRate, int16_t &rate, int16_t &rcCurve, char* title)
{
  if(activeGeneralMenuItem < 3 && ((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT)))
  {
    fcSettingChanged = true;
  }
  if(code > 0)
  {
    switch(activeGeneralMenuItem)
    {
      case 0:
        rcRate = checkCode(rcRate, RATE_STEP);
      break;
      case 1:
        rate = checkCode(rate, RATE_STEP);        
      break;
      case 2:
        rcCurve = checkCode(rcCurve, RATE_STEP);        
      break;
      case 3:
        activeGeneralMenuItem = 0;
        return false;
      break;
    }
  }
  if((code &  inputChecker.PITCH_UP) && activeGeneralMenuItem > 0)
  {
    activeGeneralMenuItem--;
  }
  
  static const uint8_t RATES_GENERAL_MENU_ITEMS = 4;
  
  if((code &  inputChecker.PITCH_DOWN) && activeGeneralMenuItem < (RATES_GENERAL_MENU_ITEMS-1))
  {
    activeGeneralMenuItem++;
  } 
  
  FLASH_STRING(RC_RATE_STR,  "rc rate  : ");
  FLASH_STRING(RATE_STR,     "rate     : ");
  FLASH_STRING(RC_CURVE_STR, "rc curve : ");
  FLASH_STRING(BACK5_STR,    "back");
  
  uint8_t startRow = 1;
  uint8_t startCol = COLS/2 - (RC_RATE_STR.length()+6)/2;
  OSD.setCursor( COLS/2 - strlen(title)/2, ++startRow );
  OSD.print( fixStr(title) );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeGeneralMenuItem);
  OSD.print( fixFlashStr(&RC_RATE_STR) );
  print_int16(rcRate, printBuf,3,1);
  OSD.print( printBuf );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeGeneralMenuItem);
  OSD.print( fixFlashStr(&RATE_STR) );
  print_int16(rate, printBuf,3,1);
  OSD.print( printBuf );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeGeneralMenuItem);
  OSD.print( fixFlashStr(&RC_CURVE_STR) );
  print_int16(rcCurve, printBuf,3,1);
  OSD.print( printBuf );
 
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeGeneralMenuItem);
  OSD.print( fixFlashStr(&BACK5_STR) );
  
  return true;
}

void* RatesRollMenu()
{
  if(RatesGeneralMenu(activeRatesRollMenuItem, rcrate_roll, rate_roll, rccurve_roll, "roll"))
  {
    return (void*) RatesRollMenu;
  }
  cleanScreen();
  return (void*)RatesMenu;
}

void* RatesPitchMenu()
{
  if(RatesGeneralMenu(activeRatesPitchMenuItem, rcrate_pitch, rate_pitch, rccurve_pitch, "pitch"))
  {
    return (void*) RatesPitchMenu;
  }
  cleanScreen();
  return (void*)RatesMenu;
}

void* RatesYawMenu()
{
  if(RatesGeneralMenu(activeRatesYawMenuItem, rcrate_yaw, rate_yaw, rccurve_yaw, "yaw"))
  {
    return (void*) RatesYawMenu;
  }
  cleanScreen();
  return (void*)RatesMenu;
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
  if((code &  inputChecker.PITCH_UP) && activeRatesMenuItem > 0)
  {
    activeRatesMenuItem--;
  }
  
  static const uint8_t RATES_MENU_ITEMS = 4;
  
  if((code &  inputChecker.PITCH_DOWN) && activeRatesMenuItem < (RATES_MENU_ITEMS-1))
  {
    activeRatesMenuItem++;
  } 
  
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

boolean PIDGeneralMenu(uint8_t &activeGeneralMenuItem, int16_t &p, int16_t &i, int16_t &d, char* title)
{
  if(activeGeneralMenuItem < 3 && ((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT)))
  {
    fcSettingChanged = true;
  }
  if(code > 0)
  {
    switch(activeGeneralMenuItem)
    {
      case 0:
        p = checkCode(p, P_STEP);
      break;
      case 1:
        if((code &  inputChecker.ROLL_LEFT) && (i-I_STEP) >= 0)
        {
          i -= I_STEP;
        }
        if((code &  inputChecker.ROLL_RIGHT) && (i+I_STEP) <= 32000)
        {
          i += I_STEP;
        }
      break;
      case 2:
        d = checkCode(d, D_STEP);
      break;
      case 3:
        activeGeneralMenuItem = 0;
        return false;
      break;
    }
  }
  if((code &  inputChecker.PITCH_UP) && activeGeneralMenuItem > 0)
  {
    activeGeneralMenuItem--;
  }
  
  static const uint8_t PID_GENERAL_MENU_ITEMS = 4;
  
  if((code &  inputChecker.PITCH_DOWN) && activeGeneralMenuItem < (PID_GENERAL_MENU_ITEMS-1))
  {
    activeGeneralMenuItem++;
  } 
  
  FLASH_STRING(P_STR,     "p : ");
  FLASH_STRING(I_STR,     "i : ");
  FLASH_STRING(D_STR,     "d : ");
  FLASH_STRING(BACK3_STR, "back");
  
  uint8_t startRow = 1;
  uint8_t startCol = COLS/2 - (P_STR.length()+6)/2;
  OSD.setCursor( COLS/2 - strlen(title)/2, ++startRow );
  OSD.print( fixStr(title) );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeGeneralMenuItem);
  OSD.print( fixFlashStr(&P_STR) );
  print_int16(p, printBuf,3,1);
  OSD.print( printBuf );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeGeneralMenuItem);
  OSD.print( fixFlashStr(&I_STR) );
  print_int16(i, printBuf,3,1);
  OSD.print( printBuf );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeGeneralMenuItem);
  OSD.print( fixFlashStr(&D_STR) );
  print_int16(d, printBuf,3,1);
  OSD.print( printBuf );
 
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeGeneralMenuItem);
  OSD.print( fixFlashStr(&BACK3_STR) );
  
  return true;
}

void* PIDRollMenu()
{
  if(PIDGeneralMenu(activePIDRollMenuItem, p_roll, i_roll, d_roll, "roll"))
  {
    return (void*) PIDRollMenu;
  }
  cleanScreen();
  return (void*)PIDMenu;
}

void* PIDPitchMenu()
{
  if(PIDGeneralMenu(activePIDPitchMenuItem, p_pitch, i_pitch, d_pitch, "pitch"))
  {
    return (void*) PIDPitchMenu;
  }
  cleanScreen();
  return (void*)PIDMenu;
}

void* PIDYawMenu()
{
  if(PIDGeneralMenu(activePIDYawMenuItem, p_yaw, i_yaw, d_yaw, "yaw"))
  {
    return (void*) PIDYawMenu;
  }
  cleanScreen();
  return (void*)PIDMenu;
}

void* TPAMenu()
{
  if(activeTPAMenuItem < 3 && ((code &  inputChecker.ROLL_LEFT) ||  (code &  inputChecker.ROLL_RIGHT)))
  {
    fcSettingChanged = true;
  }
  if(code > 0)
  {
    switch(activeTPAMenuItem)
    {
      case 0:
        p_tpa = checkCode(p_tpa, TPA_STEP);        
      break;
      case 1:
        i_tpa = checkCode(i_tpa, TPA_STEP);        
      break;
      case 2:
        d_tpa = checkCode(d_tpa, TPA_STEP);        
      break;
      case 3:
        cleanScreen();
        activeTPAMenuItem = 0;
        return (void*)PIDMenu;
      break;
    }
  }
  if((code &  inputChecker.PITCH_UP) && activeTPAMenuItem > 0)
  {
    activeTPAMenuItem--;
  }
  
  static const uint8_t TPA_MENU_ITEMS = 4;
  
  if((code &  inputChecker.PITCH_DOWN) && activeTPAMenuItem < (TPA_MENU_ITEMS-1))
  {
    activeTPAMenuItem++;
  } 
  
  FLASH_STRING(TPA_P_STR, "tpa p : ");
  FLASH_STRING(TPA_I_STR, "tpa i : ");
  FLASH_STRING(TPA_D_STR, "tpa d : ");
  FLASH_STRING(BACK4_STR, "back");
  
  uint8_t startRow = 1;
  uint8_t startCol = COLS/2 - (TPA_P_STR.length()+6)/2;
  const char title[] = "tpa menu";
  OSD.setCursor( COLS/2 - strlen(title)/2, ++startRow );
  OSD.print( fixStr(title) );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeTPAMenuItem);
  OSD.print( fixFlashStr(&TPA_P_STR) );
  print_int16(p_tpa, printBuf,3,1);
  OSD.print( printBuf );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeTPAMenuItem);
  OSD.print( fixFlashStr(&TPA_I_STR) );
  print_int16(i_tpa, printBuf,3,1);
  OSD.print( printBuf );
  
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeTPAMenuItem);
  OSD.print( fixFlashStr(&TPA_D_STR) );
  print_int16(d_tpa, printBuf,3,1);
  OSD.print( printBuf );
 
  OSD.setCursor( startCol, ++startRow );
  checkArrow(startRow, activeTPAMenuItem);
  OSD.print( fixFlashStr(&BACK4_STR) );
  
  return (void*)TPAMenu;
}

void* PIDMenu()
{
  if(code &  inputChecker.ROLL_RIGHT)
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
        cleanScreen();
        activePIDMenuItem = 0;
        return (void*)MainMenu;
      break;
    }
  }
  if((code &  inputChecker.PITCH_UP) && activePIDMenuItem > 0)
  {
    activePIDMenuItem--;
  }
  
  static const uint8_t PID_MENU_ITEMS = 5;
  
  if((code &  inputChecker.PITCH_DOWN) && activePIDMenuItem < (PID_MENU_ITEMS-1))
  {
    activePIDMenuItem++;
  } 
  
  FLASH_STRING(ROLL_STR,  "roll  ");
  FLASH_STRING(PITCH_STR, "pitch ");
  FLASH_STRING(YAW_STR,   "yaw   ");
  FLASH_STRING(TPA_STR,   "tpa   ");
  FLASH_STRING(BACK2_STR, "back  ");
  
  uint8_t startRow = 1;
  uint8_t startCol = COLS/2 - ROLL_STR.length()/2;
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
      case 3:
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
      case 4:
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
      case 5:
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
      case 6:
        menuActive = false;
      break;
      case 7:
        menuActive = false;
        settingChanged = false;
        fcSettingChanged = false;
        settings.ReadSettings();
        fcSettingsReceived = false;
      break;
    }
  }
  if((code &  inputChecker.PITCH_UP) && activeMenuItem > 0)
  {
    activeMenuItem--;
  }
  
  static const uint8_t MAIN_MENU_ITEMS = 8;
  
  if((code &  inputChecker.PITCH_DOWN) && activeMenuItem < (MAIN_MENU_ITEMS-1))
  {
    activeMenuItem++;
  } 
  
  FLASH_STRING(PID_STR,             "pid");
  FLASH_STRING(RATES_STR,           "rates");
  FLASH_STRING(BATTERY_WARNING_STR, "batt. warning: ");
  FLASH_STRING(BATTERY_PERCENT_STR, "batt. % alarm: ");
  FLASH_STRING(DV_CHANNEL_STR,      "dv channel   : ");
  FLASH_STRING(TEMP_UNIT_STR,       "temp. unit   : ");
  FLASH_STRING(SAVE_EXIT_STR,       "save+exit");
  FLASH_STRING(CANCEL_STR,          "cancel");
  FLASH_STRING_ARRAY(ON_OFF_STR,  PSTR("off"), PSTR("on "));
  FLASH_STRING_ARRAY(AUX_CHANNEL_STR,  PSTR("aux1"), PSTR("aux2"), PSTR("aux3"), PSTR("aux4"));
  
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
  OSD.print( fixFlashStr(&BATTERY_WARNING_STR) );
  OSD.print( fixFlashStr(&ON_OFF_STR[settings.m_batWarning]) );
 
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
  OSD.print( fixFlashStr(&AUX_CHANNEL_STR[settings.m_DVchannel]) );
 
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

static void* activePage = NULL;

void loop(){
  uint16_t i = 0;
  static uint8_t blink_i = 1;
  
  if(micros()-LastLoopTime > 10000)
  {
    LastLoopTime = micros();
    blink_i++;
    if (blink_i > 100){
      blink_i = 1;
    }
    
    if(menuActive && !fcSettingsReceived)
    {
      NewSerial.write(0x30); // request settings
      ReadFCSettings(fcSettingChanged);
    }
    else
    {
      NewSerial.write(0x20); // request telemetry
      ReadTelemetry();
    }    
  
    while (!OSD.notInVSync());

      #ifdef DEBUG
      static char Aux1[15];
      print_int16((int16_t)freeRam(), Aux1,0,0);
      OSD.setCursor(8,-1);
      OSD.print(Aux1);
      print_int16(roll, Aux1,0,0);
      OSD.setCursor(8,-2);
      OSD.print(Aux1);
      #endif 
      
      if(triggerCleanScreen || lastAuxVal != AuxChanVals[settings.m_DVchannel])
      {
        lastAuxVal = AuxChanVals[settings.m_DVchannel];
        triggerCleanScreen = false;
        cleanScreen();
      }
      
      if(settings.m_tempUnit == 1)
      {
        tempSymbol = 0x31;
      }
      else
      {
        tempSymbol = 0x30;
      }
      
      code = inputChecker.ProcessStickInputs(roll, pitch, yaw, armed);
      
      if(armed == 0 && settings.m_lastMAH > 0)
      {
        FLASH_STRING(LAST_BATTERY_STR, "continue last battery?");
        OSD.setCursor(COLS/2 - (LAST_BATTERY_STR.length())/2,ROWS/2 - 1);
        OSD.print( fixFlashStr(&LAST_BATTERY_STR) );
        
        FLASH_STRING(ROLL_RIGHT_STR, "roll right to confirm");
        OSD.setCursor(COLS/2 - ROLL_RIGHT_STR.length()/2,ROWS/2);
        OSD.print( fixFlashStr(&ROLL_RIGHT_STR) );
        FLASH_STRING(ARM_CANCEL_STR, "arm to cancel");
        OSD.setCursor(COLS/2 - ARM_CANCEL_STR.length()/2,ROWS/2 + 1);
        OSD.print( fixFlashStr(&ARM_CANCEL_STR) );
                
        if(roll > 1950)
        {
          previousMAH = settings.m_lastMAH;
          settings.m_lastMAH = 0;
          cleanScreen();
        }
        else
        {
          return;
        }
      }
      
      if(armed == 0 && ((code &  inputChecker.YAW_LONG_LEFT) || menuActive))
      {
        if(!menuActive)
        {
          menuActive = true;
          menuWasActive = true;
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
          if(fcSettingChanged)
          {
            SendFCSettings();
          }
        }        
      }
      if(yaw > 1750 && armed == 0)
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
        OSD.setCursor(COLS/2 - (BATTERY_STR.length()+1)/2,ROWS/2 - 1);
        OSD.print( fixFlashStr(&BATTERY_STR) );
        print_int16(settings.m_activeBattery+1, printBuf,0,1);
        OSD.print( printBuf );
        print_int16(settings.m_batMAH[settings.m_activeBattery], printBuf,0,1);
        OSD.setCursor(COLS/2 - (strlen(printBuf)+3)/2,ROWS/2);
        OSD.print( printBuf );
        OSD.print( fixStr("mah") );
        OSD.print( fixFlashStr(&TWO_BLANKS) );
        FLASH_STRING(WARN_STR, "warn at ");
        print_int16(settings.m_batWarningMAH, printBuf,0,1);
        OSD.setCursor(COLS/2 - (WARN_STR.length() + strlen(printBuf) + 3)/2,ROWS/2 + 1);
        OSD.print( fixFlashStr(&WARN_STR) );
        OSD.print( printBuf );
        OSD.print( fixStr("mah") );
        OSD.print( fixFlashStr(&TWO_BLANKS) );
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
        OSD.setCursor( COLS/2 - (STATS_STR.length()+4)/2, middle_infos_y );
        OSD.print( fixFlashStr(&STATS_STR) );
        print_int16(statPage+1, printBuf,0,1);
        OSD.print(printBuf);
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
          print_time(total_time, printBuf);
          uint8_t statCol = COLS/2 - (TIME_STR.length()+7)/2;
          OSD.setCursor( statCol, ++middle_infos_y );
          OSD.print( fixFlashStr(&TIME_STR) );
          OSD.print( printBuf );
  
          OSD.setCursor( statCol, ++middle_infos_y ); 
          OSD.print( fixFlashStr(&MAX_AMP_STR) );
          print_int16(MaxAmps, printBuf,1,1);
          OSD.print( printBuf );
          OSD.print( fixChar('a') );
          
          OSD.setCursor( statCol, ++middle_infos_y );
          OSD.print( fixFlashStr(&MAX_C_STR) ); 
          print_int16(MaxC, printBuf,0,1);
          OSD.print( printBuf );
          OSD.print( fixChar('c') );
          
          OSD.setCursor( statCol, ++middle_infos_y );
          OSD.print( fixFlashStr(&MAH_STR) );
          print_int16(LipoMAH+previousMAH, printBuf,0,1);
          OSD.print( printBuf );
          OSD.print( fixStr("mah") );        
  
          OSD.setCursor( statCol, ++middle_infos_y );
          OSD.print( fixFlashStr(&MAX_RPM_STR) );
          print_int16(MaxRPMs, printBuf,1,1);
          OSD.print( printBuf );
          OSD.print( fixStr("kr") );
  
          OSD.setCursor( statCol, ++middle_infos_y );
          OSD.print( fixFlashStr(&MAX_WATT_STR) );
          print_int16(MaxWatt, printBuf,1,1);
          OSD.print( printBuf );
          OSD.print( fixChar('w') );
  
          OSD.setCursor( statCol, ++middle_infos_y );          
          OSD.print( fixFlashStr(&MAX_TEMP_STR) );
          print_int16(MaxTemp, printBuf,0,1);
          OSD.print( printBuf );
          OSD.print( tempSymbol );
  
          OSD.setCursor( statCol, ++middle_infos_y );          
          OSD.print( fixFlashStr(&MIN_V_STR) );
          print_int16(MinBat, printBuf,2,1);
          OSD.print( printBuf );
          OSD.print( fixChar('v') );
        }
        else
        {
          FLASH_STRING_ARRAY(ESC_RPM_STR,  PSTR("esc1 max rpm : "), PSTR("esc2 max rpm : "), PSTR("esc3 max rpm : "), PSTR("esc4 max rpm : "));
          FLASH_STRING_ARRAY(ESC_A_STR,    PSTR("esc1 max a   : "), PSTR("esc2 max a   : "), PSTR("esc3 max a   : "), PSTR("esc4 max a   : "));
          FLASH_STRING_ARRAY(ESC_TEMP_STR, PSTR("esc1 max temp: "), PSTR("esc2 max temp: "), PSTR("esc3 max temp: "), PSTR("esc4 max temp: "));
          
          uint8_t startCol = COLS/2 - (ESC_RPM_STR[0].length()+6)/2;
          OSD.setCursor( startCol, ++middle_infos_y );
          OSD.print( fixFlashStr(&ESC_RPM_STR[statPage-1]) );
          print_int16(maxKERPM[statPage-1], printBuf,1,1);
          OSD.print( printBuf );
          OSD.print( fixStr("kr") );
          
          OSD.setCursor( startCol, ++middle_infos_y );
          OSD.print( fixFlashStr(&ESC_A_STR[statPage-1]) );
          print_int16(maxCurrent[statPage-1], printBuf,2,1);
          OSD.print( printBuf );
          OSD.print( fixChar('a') );
          
          OSD.setCursor( startCol, ++middle_infos_y );
          OSD.print( fixFlashStr(&ESC_TEMP_STR[statPage-1]) );
          print_int16(maxTemps[statPage-1], printBuf,0,1);
          OSD.print( printBuf );
          OSD.print( tempSymbol );
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
          print_int16(throttle, printBuf,0,1);
          OSD.setCursor( 0, 0 );
          OSD.print( printBuf );
          OSD.print( fixChar('%') );
          OSD.print( fixFlashStr(&TWO_BLANKS) );
          ESCmarginTop = 1;
        }
          
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_NICKNAME_DV)
        {
          OSD.setCursor( 10, -1 );
          OSD.print( fixStr(NICKNAME) );
        }
    
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_COMB_CURRENT_DV)
        {
          uint8_t CurrentPos = print_int16(current, printBuf,1,0);
          OSD.setCursor( -(CurrentPos+2), 0 );
          OSD.print( printBuf );
          OSD.print( fixChar('a') );
          ESCmarginTop = 1;
        }
        
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_LIPO_VOLTAGE_DV)
        {
          print_int16(LipoVoltage / 10, printBuf,1,1);
          OSD.setCursor( 0, -1 );
          OSD.print( printBuf );
          OSD.print( fixChar('v') );
          OSD.print( fixFlashStr(&TWO_BLANKS) );
          ESCmarginBot = 1;
        }
        
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_MA_CONSUMPTION_DV)
        {
          uint8_t lipoMAHPos = print_int16(LipoMAH+previousMAH, printBuf,0,1);
          OSD.setCursor( -(4+lipoMAHPos), -1 );
          OSD.print( printBuf );
          OSD.print( fixStr("mah") );
          ESCmarginBot = 1;
        }
        
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_ESC_KRPM_DV)
        {
          static char KR[] = "kr";
          uint8_t KRPMPos = print_int16(motorKERPM[0], printBuf,1,1);
          OSD.setCursor( 0, ESCmarginTop );
          OSD.print( printBuf );
          OSD.print( fixStr(KR) );
          OSD.print( fixFlashStr(&TWO_BLANKS) );
          
          KRPMPos = print_int16(motorKERPM[1], printBuf,1,0);
          OSD.setCursor( -(KRPMPos+3), ESCmarginTop );
          OSD.print( printBuf );
          OSD.print( fixStr(KR) );
          
          KRPMPos = print_int16(motorKERPM[2], printBuf,1,0);
          OSD.setCursor( -(KRPMPos+3), -(1+ESCmarginBot) );
          OSD.print( printBuf );
          OSD.print( fixStr(KR) );
         
          print_int16(motorKERPM[3], printBuf,1,1);
          OSD.setCursor( 0, -(1+ESCmarginBot) );
          OSD.print( printBuf );
          OSD.print( fixStr(KR) );
          OSD.print( fixFlashStr(&TWO_BLANKS) );
          
          TMPmargin++;
          CurrentMargin++;
        }
     
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_ESC_CURRENT_DV)
        {
          print_int16(motorCurrent[0], printBuf,2,1);
          OSD.setCursor( 0, CurrentMargin+ESCmarginTop );
          OSD.print( printBuf );
          OSD.print( fixChar('a') );
          OSD.print( fixFlashStr(&TWO_BLANKS) );
          
          uint8_t CurrentPos = print_int16(motorCurrent[1], printBuf,2,0);
          OSD.setCursor( -(CurrentPos+2), CurrentMargin+ESCmarginTop );
          OSD.print( printBuf );
          OSD.print( fixChar('a') );
          
          CurrentPos = print_int16(motorCurrent[2], printBuf,2,0);
          OSD.setCursor( -(CurrentPos+2), -(1+CurrentMargin+ESCmarginBot) );
          OSD.print( printBuf );
          OSD.print( fixChar('a') );
         
          print_int16(motorCurrent[3], printBuf,2,1);
          OSD.setCursor( 0, -(1+CurrentMargin+ESCmarginBot) );
          OSD.print( printBuf );
          OSD.print( fixChar('a') );
          OSD.print( fixFlashStr(&TWO_BLANKS) );
          TMPmargin++;
        }
    
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_ESC_TEMPERATURE_DV)
        {
          print_int16(ESCTemps[0], printBuf,0,1);
          OSD.setCursor( 0, TMPmargin+ESCmarginTop );
          OSD.print( printBuf );
          OSD.print( tempSymbol );
          OSD.print( fixFlashStr(&TWO_BLANKS) );
          
          uint8_t TempPos = print_int16(ESCTemps[1], printBuf,0,0);
          OSD.setCursor( -(TempPos+2), TMPmargin+ESCmarginTop );
          OSD.print( printBuf );
          OSD.print( tempSymbol );
          
          TempPos = print_int16(ESCTemps[2], printBuf,0,0);
          OSD.setCursor( -(TempPos+2), -(1+TMPmargin+ESCmarginBot) );
          OSD.print( printBuf );
          OSD.print( tempSymbol );
         
          print_int16(ESCTemps[3], printBuf,0,1);
          OSD.setCursor( 0, -(1+TMPmargin+ESCmarginBot) );
          OSD.print( printBuf );
          OSD.print( tempSymbol );
          OSD.print( fixFlashStr(&TWO_BLANKS) );
        }  
    
        if(AuxChanVals[settings.m_DVchannel] > DISPLAY_TIMER_DV) 
        {
          OSD.setCursor( 12, -2 );
          print_time(time, printBuf);
          OSD.print( printBuf );
        }
        
        if(settings.m_batWarning > 0 && (LipoMAH+previousMAH) >= settings.m_batWarningMAH)
        {
          totalMAH = 0;
          FLASH_STRING(BATTERY_LOW,   "battery low");
          FLASH_STRING(BATTERY_EMPTY, "           ");
          OSD.setCursor(COLS/2 - BATTERY_LOW.length()/2,ROWS/2 +3);
          if (blink_i % 20 == 0) 
          {
            if(bat_clear) 
            {
              OSD.print( fixFlashStr(&BATTERY_LOW) );
              bat_clear = false;
            }
            else 
            {              
              OSD.print( fixFlashStr(&BATTERY_EMPTY) );
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
