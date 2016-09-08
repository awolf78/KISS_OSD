#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

/*
KISS FC OSD v1.0
By Felix Niessen (felix.niessen@googlemail.com)
for Flyduino.net

KISS FC OSD v1.3
by Greg

KISS FC OSD v1.4
by Alexander Wolf (awolf78@gmx.de)
Donate: www.paypal.me/awolf78

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

// displayed datas
//=============================
#define DISPLAY_NICKNAME
#define DISPLAY_TIMER
#define DISPLAY_RC_THROTTLE
//#define DISPLAY_COMB_CURRENT
#define DISPLAY_LIPO_VOLTAGE
#define DISPLAY_MA_CONSUMPTION
//#define DISPLAY_ESC_KRPM
//#define DISPLAY_ESC_CURRENT
#define DISPLAY_ESC_TEMPERATURE

// displayed datas in reduced mode
//=============================
//#define RED_DISPLAY_NICKNAME
//#define RED_DISPLAY_TIMER
//#define RED_DISPLAY_RC_THROTTLE
//#define RED_DISPLAY_COMB_CURRENT
#define RED_DISPLAY_LIPO_VOLTAGE
#define RED_DISPLAY_MA_CONSUMPTION
//#define RED_DISPLAY_ESC_KRPM
//#define RED_DISPLAY_ESC_CURRENT
#define RED_DISPLAY_STATS
//#define RED_DISPLAY_ESC_TEMPERATURE

// reduced mode channel config
//=============================
#define RED_MODE_AUX_CHAN 1 // 1-4

#define RED_ON_AUX_LOW
//#define RED_ON_AUX_MID
//#define RED_ON_AUX_HIGH

/*
Digital Volume configuration tool
---------------------------------
Use this feature to increase the amount of data displayed in reduced mode.
Only makes sense of you have a remote with a digital volume (DV) dial. 
Assign the DV ony our remote to your RED_MODE_AUX_CHAN. Anything you do not want to 
display set to a very high value (like 1000).
Example:
You want the Lipo voltage and mAh consumption to be shown at all times and then be able
to show your nickname and after the the combination current with the DV dial:
- set RED_MODE_AUX_CHAN to the channel for the DV dial (and set it to the same channel on your radio)
- set USE_DV = 1;
- set RED_DISPLAY_MA_CONSUMPTION_DV to 0
- set RED_DISPLAY_LIPO_VOLTAGE_DV to 0
- set RED_DISPLAY_NICKNAME_DV to 1
- set RED_DISPLAY_COMB_CURRENT_DV to 2
- set the other RED_DISPLAY_..._DV values to -1 
When you turn your radio on with DV all the way to minimum volume you will see only the Lipo voltage and
the the mAh consumption. Turning the DV dial a little higher your nickname will pop up (don't forget to arm 
first). Turning it even further will display the combination current.
=============================*/
static const int16_t USE_DV = 1; // 0=off,1=on
static int16_t RED_DISPLAY_NICKNAME_DV =           2;
static int16_t RED_DISPLAY_TIMER_DV =              1;
static int16_t RED_DISPLAY_RC_THROTTLE_DV =        6;
static int16_t RED_DISPLAY_COMB_CURRENT_DV =       3;
static int16_t RED_DISPLAY_LIPO_VOLTAGE_DV =       0;
static int16_t RED_DISPLAY_MA_CONSUMPTION_DV =     0;
static int16_t RED_DISPLAY_ESC_KRPM_DV =           5;
static int16_t RED_DISPLAY_ESC_CURRENT_DV =        4;
static int16_t RED_DISPLAY_STATS_DV =              0;
static int16_t RED_DISPLAY_ESC_TEMPERATURE_DV =    7;

/* Low battery warning config
-----------------------------
This feature will show a flashing BATT LOW!!! warning just below the center of your 
display when bat_mAh_warning is exceeded. I have found the mAh calculation of my KISS
setup (KISS FC with 24RE ESCs using ESC telemetry) to be very accurate, so be careful
when using other ESCs - mAh calculation might not be as accurate. Simply change bat_mAh_warning
to the maximum mAh you are comfortable with for your battery just before you land. If
you use different sizes batteries, you might want to check out the Digital volume feature. This
will allow you to change the bat_mAh_warning setting while the quad is disarmed. To activate this
feature:
- set BAT_AUX_DV_CHANNEL to the Aux channel you wish to use
- setup your radio's DV to BAT_AUX_DV_CHANNEL
Set the DV on the radio approximately in the middle. Once powered up, arm and disarm immediately to be able 
to change the mAh setting. You need to arm and disarm immediately every time your want to change this setting.
Turning the dial up with increase bat_mAh_warning  by BAT_MAH_INCREMENT, turning it down will decrease it by 
BAT_MAH_INCREMENT. Arm your quad to save the value.
To turn the this feature off, set BAT_AUX_DV_CHANNEL = 0;
To turn the battery warning off as well set bat_mAh_warning = -1;
In a future version I will probably replace the mAh adjustment with a OSD menu entry - stay tuned :)
=============================*/
static int16_t bat_mAh_warning = 1000;
static const uint16_t BAT_AUX_DV_CHANNEL = 1; // 0-4, 0 = none
static const int16_t BAT_MAH_INCREMENT = 50;
static const int16_t DV_JITTER = 40; // increase in 10s if the mAh number keeps jumping around the value you want to set

// END OF CONFIGURATION
//=========================================================================================================================


// internals
//=============================
#define LOCATION_BAT_WARNING_MAH_LSB	0x03
#define LOCATION_BAT_WARNING_MAH_MSB	0x04

#include <SPI.h>
#include <MAX7456.h>

const byte osdChipSelect             =            6;
const byte masterOutSlaveIn          =            MOSI;
const byte masterInSlaveOut          =            MISO;
const byte slaveClock                =            SCK;
const byte osdReset                  =            2;

MAX7456 OSD( osdChipSelect );


static char clean[30];

int16_t setupPPM(int16_t pos) {
  if(pos < 0) {
    return 10000;
  }
  return -1000 + (pos * 100);
}

void setup(){
  uint8_t i = 0;
  SPI.begin();
  SPI.setClockDivider( SPI_CLOCK_DIV2 ); 
  //OSD.begin();
  #if defined(PAL)
    OSD.begin(28,15,0);
    OSD.setTextOffset(-1,-6);
    OSD.setDefaultSystem(MAX7456_PAL);
  #endif
  #if defined(NTSC)
    OSD.begin(MAX7456_COLS_N1,MAX7456_ROWS_N0);
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
  for(i=0;i<30;i++) clean[i] = ' ';
  while (!OSD.notInVSync());
  for(i=0;i<20;i++){
      OSD.setCursor( 0, i );
      OSD.print( clean );
  }
  readBatWarning();
  RED_DISPLAY_NICKNAME_DV = setupPPM(RED_DISPLAY_NICKNAME_DV);
  RED_DISPLAY_TIMER_DV = setupPPM(RED_DISPLAY_TIMER_DV);
  RED_DISPLAY_RC_THROTTLE_DV = setupPPM(RED_DISPLAY_RC_THROTTLE_DV);
  RED_DISPLAY_COMB_CURRENT_DV = setupPPM(RED_DISPLAY_COMB_CURRENT_DV);
  RED_DISPLAY_LIPO_VOLTAGE_DV = setupPPM(RED_DISPLAY_LIPO_VOLTAGE_DV);
  RED_DISPLAY_MA_CONSUMPTION_DV = setupPPM(RED_DISPLAY_MA_CONSUMPTION_DV);
  RED_DISPLAY_ESC_KRPM_DV = setupPPM(RED_DISPLAY_ESC_KRPM_DV);
  RED_DISPLAY_ESC_CURRENT_DV = setupPPM(RED_DISPLAY_ESC_CURRENT_DV);
  RED_DISPLAY_STATS_DV = setupPPM(RED_DISPLAY_STATS_DV);
  RED_DISPLAY_ESC_TEMPERATURE_DV = setupPPM(RED_DISPLAY_ESC_TEMPERATURE_DV);
  Serial.begin(115200);
}

static int16_t  throttle = 0;
static uint16_t current = 0;
static int8_t armed = 0;
static int8_t calybGyro = 0;
static uint8_t failsafe = 0;
static int8_t mode = 0;
static int8_t idleTime = 0;
static uint16_t LipoVoltage = 0;
static uint16_t LipoMAH = 0;
static uint16_t MaxAmps = 0;
static uint16_t MaxC    = 0;
static uint16_t MaxRPMs = 0;
static uint16_t MaxWatt = 0;
static uint16_t MaxTemp = 0;
static uint16_t MinBat = 0;
static uint16_t motorKERPM[4] = {0,0,0,0};
static uint16_t motorCurrent[4] = {0,0,0,0};
static uint16_t ESCTemps[4] = {0,0,0,0};
static int16_t  AuxChanVals[4] = {0,0,0,0};
static uint8_t  reducedMode = 0;
static unsigned long start_time = 0;
static unsigned long time = 0;
static unsigned long total_time = 0;
static unsigned long red_DV_change_time = 0;
static boolean bat_clear = true;
static boolean save_bat_warning = false;
static boolean unblock_bat_DV = false;
static int16_t last_Bat_Aux_Val = -10000;
static int16_t last_Red_Aux_Val = -10000;
static boolean armedOnce = false;

uint8_t print_int16(int16_t p_int, char *str, uint8_t dec, uint8_t AlignLeft){
    uint16_t useVal = p_int;
    uint8_t pre = ' ';
    if(p_int < 0){
        useVal = p_int*-1;
        pre = '-';
    }
    uint8_t aciidig[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    uint8_t i = 0;
        uint8_t digits[6] = {0,0,0,0,0,0};
    while(useVal >= 10000){digits[0]++; useVal-=10000;}
    while(useVal >= 1000){digits[1]++; useVal-=1000;}
    while(useVal >= 100){digits[2]++; useVal-=100;}
    while(useVal >= 10){digits[3]++; useVal-=10;}
    digits[4] = useVal;
        char result[6] = {' ',' ',' ',' ',' ','0'};
    uint8_t signdone = 0;
    for(i = 0; i < 6;i++){
        if(i == 5 && signdone == 0) continue;
        else if(aciidig[digits[i]] != '0' && signdone == 0){
            result[i] = pre;
            signdone = 1;
        }else if(signdone) result[i] = aciidig[digits[i-1]];
    }
        uint8_t CharPos = 0;
        for(i = 0; i < 6;i++){
          if(result[i] != ' ' || (AlignLeft == 0 || (i > 5-dec))) str[CharPos++] = result[i];
          if(dec != 0 && i == 5-dec) str[CharPos++] = '.';
          if(dec != 0 && i > 5-dec && str[CharPos-1] == ' ') str[CharPos-1] = '0';
        }
        
        return CharPos;
}	

void print_time(unsigned long time, char *time_str) {
    uint16_t seconds = time / 1000;
    uint8_t minutes = seconds / 60;
    if (seconds >= 60) {
      minutes = seconds/60;
    } else {
      minutes = 0;
    }
    seconds = seconds - (minutes * 60); // reste
    static char time_sec[6];
    uint8_t i = 0;
    uint8_t time_pos = print_int16(minutes, time_str,0,1);
    time_str[time_pos++] = ':';

    uint8_t sec_pos = print_int16(seconds, time_sec,0,1);
    if(seconds < 10) {
      time_str[time_pos++] = '0';
    }
    for (i=0; i<sec_pos; i++)
    {
      time_str[time_pos++] = time_sec[i];
    }
    //milliseconds - I don't need those...
    /*time_str[time_pos++] = '.';
    static char time_mil[6];
    uint8_t mills = time % 1000;
    uint8_t mil_pos = print_int16(mills, time_mil,0,1);
    time_str[time_pos++] = time_mil[0];
    /*
    for (i=0; i<mil_pos; i++)
    {
      time_str[time_pos++] = time_mil[i];
    }
    */

    /*for (i=time_pos; i<9; i++)
    {
      time_str[time_pos++] = ' ';
    }*/
}

uint32_t ESC_filter(uint32_t oldVal, uint32_t newVal){
  return (uint32_t)((uint32_t)((uint32_t)((uint32_t)oldVal*ESC_FILTER)+(uint32_t)newVal))/(ESC_FILTER+1);
}

void writeBatWarning(){
  byte msb, lsb;
  //Record bat_mAh_warning to EEPROM
  lsb = (byte)(bat_mAh_warning & 0x00FF);
  msb = (byte)((bat_mAh_warning & 0xFF00) >> 8);

  EEPROM.write(LOCATION_BAT_WARNING_MAH_LSB, lsb); // LSB
  EEPROM.write(LOCATION_BAT_WARNING_MAH_MSB, msb); // MSB
}

void readBatWarning() {
  byte msb, lsb;

  lsb = EEPROM.read(LOCATION_BAT_WARNING_MAH_LSB);
  msb = EEPROM.read(LOCATION_BAT_WARNING_MAH_MSB);

  if((lsb == 255 && msb == 255) || (lsb == 0 && msb == 0)) {
    writeBatWarning();
    return;
  }
  
  //Combine two 8-bit EEPROM spots into one 16-bit number
  bat_mAh_warning = msb;
  bat_mAh_warning = bat_mAh_warning << 8;
  bat_mAh_warning |= lsb;
  
  if(bat_mAh_warning < 300) {
    bat_mAh_warning = 300;
    writeBatWarning();
  }
  if(bat_mAh_warning > 32000) {
    bat_mAh_warning = 32000;
    writeBatWarning();
  }
}

void cleanScreen() {
  uint16_t i = 0;
  for(i=0;i<20;i++){
      OSD.setCursor( 0, i );
      OSD.print( clean );
  }
  while (!OSD.notInVSync());
}

uint16_t findMaxUint16(uint16_t maxV, uint16_t *values, uint16_t length) {
  for(uint16_t i = 0; i < length; i++) {
    if(values[i] > maxV) {
      maxV = values[i];
    }
  }
  return maxV;
}

uint16_t findMax(uint16_t maxV, uint16_t newVal) {
  if(newVal > maxV) {
    return newVal;
  }
  return maxV;
}

void loop(){
  uint16_t i = 0;
  uint8_t KRPMPoses[4];
  static uint8_t lastMode = 0;
  static int16_t lastAuxVal = 0;
  
  static char Motor1KERPM[30];
  static char Motor2KERPM[30];
  static char Motor3KERPM[30];
  static char Motor4KERPM[30];  
  
  uint8_t CurrentPoses[4];
  static char Motor1Current[30];
  static char Motor2Current[30];
  static char Motor3Current[30];
  static char Motor4Current[30];  
  
  uint8_t TempPoses[4];
  static char ESC1Temp[30];
  static char ESC2Temp[30];
  static char ESC3Temp[30];
  static char ESC4Temp[30];  
  
  static char LipoVoltC[30];
  static char LipoMinVoltC[30];
  static char LipoMAHC[30];
  
  static char Throttle[30];
  static char Current[30];

  static char Time[10];
  static char TotalTime[10];
  static char MaxTempC[30];
  static char MaxAmpC[30];
  static char MaxRPMC[30];
  static char MaxWattC[30];
  
  static uint8_t serialBuf[255];
  static uint8_t minBytes = 0;
  static uint8_t recBytes = 0;
  
  static uint32_t LastLoopTime = 0;
  static uint8_t blink_i = 1;
  
  if(micros()-LastLoopTime > 10000){
    LastLoopTime = micros();
    blink_i++;
    if (blink_i > 100){
      blink_i = 1;
    }
  
    Serial.write(0x20); // request telemetrie
    
    minBytes = 100;
    recBytes = 0;
   
    while(recBytes < minBytes && micros()-LastLoopTime < 20000){
      #define STARTCOUNT 2
      while(Serial.available()) serialBuf[recBytes++] = Serial.read();
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
           LipoVoltage =   ((serialBuf[17+STARTCOUNT]<<8) | serialBuf[18+STARTCOUNT]);

           int8_t current_armed = serialBuf[16+STARTCOUNT];
           // switch disarmed => armed
           if (armed == 0 && current_armed > 0) {
             start_time = millis();
             lastMode = 10; // triggers "cleaning" of screen
             armedOnce = true;
             if(BAT_AUX_DV_CHANNEL > 0) {
               last_Bat_Aux_Val = AuxChanVals[BAT_AUX_DV_CHANNEL-1];
             }
             last_Red_Aux_Val = AuxChanVals[RED_MODE_AUX_CHAN-1];
             if(save_bat_warning) {
               save_bat_warning = false;
               writeBatWarning();
             }
             unblock_bat_DV = true;
             red_DV_change_time = 0;
           }
           // switch armed => disarmed
           else {
             if (armed > 0 && current_armed == 0) {
               total_time = total_time + (millis() - start_time);
               start_time = 0;
               lastMode = 10; // triggers "cleaning" of screen
             } 
             else if (armed > 0) {
               time = millis() - start_time;
             }
           }
           armed = current_armed;

           #ifdef RED_DISPLAY_DEBUG
           /*
           armed =   ((serialBuf[15+STARTCOUNT]<<8) | serialBuf[16+STARTCOUNT]);
           calybGyro =   ((serialBuf[39+STARTCOUNT]<<8) | serialBuf[40+STARTCOUNT]);
           */
           mode =   serialBuf[65+STARTCOUNT];
           idleTime =   serialBuf[82+STARTCOUNT];
           //calybGyro =   serialBuf[40+STARTCOUNT];
           if (serialBuf[41+STARTCOUNT] > 0)
           {
               failsafe =   1;
           } else {
               failsafe =   0;
           }
           //failsafe =   ((serialBuf[40+STARTCOUNT]<<8) | serialBuf[41+STARTCOUNT]);
           if ((serialBuf[36+STARTCOUNT] + serialBuf[37+STARTCOUNT] + serialBuf[38+STARTCOUNT] + serialBuf[39+STARTCOUNT] + serialBuf[40+STARTCOUNT]) == 0)
           {
             calybGyro = 1;
           } else {
             calybGyro = 0;
           }
           #endif

           
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
           if(((serialBuf[125+STARTCOUNT]<<8) | serialBuf[126+STARTCOUNT]) > 5){ 
             tmpVoltage += ((serialBuf[125+STARTCOUNT]<<8) | serialBuf[126+STARTCOUNT]);
             voltDev++;
           }
           
           if(voltDev!=0) LipoVoltage = tmpVoltage/voltDev;

           if (MinBat == 0)
           {
             MinBat = LipoVoltage;
           }
           else if (LipoVoltage < MinBat)
           {
             MinBat = LipoVoltage;
           }
           
           //MaxAmps =       findMax(MaxAmps, ((serialBuf[146+STARTCOUNT]<<8) | serialBuf[147+STARTCOUNT]) / 1000); 
           LipoMAH =       ((serialBuf[148+STARTCOUNT]<<8) | serialBuf[149+STARTCOUNT]);
           //MaxRPMs =       findMax(MaxRPMs, ((serialBuf[150+STARTCOUNT]<<8) | serialBuf[151+STARTCOUNT])); 
           //MaxWatt =       findMax(MaxWatt, ((serialBuf[152+STARTCOUNT]<<8) | serialBuf[153+STARTCOUNT])); 
           
           static uint32_t windedupfilterdatas[8];
           
           windedupfilterdatas[0] = ESC_filter((uint32_t)windedupfilterdatas[0],(uint32_t)((serialBuf[91+STARTCOUNT]<<8) | serialBuf[92+STARTCOUNT])/(MAGNETPOLECOUNT/2)<<4);
           windedupfilterdatas[1] = ESC_filter((uint32_t)windedupfilterdatas[1],(uint32_t)((serialBuf[101+STARTCOUNT]<<8) | serialBuf[102+STARTCOUNT])/(MAGNETPOLECOUNT/2)<<4);
           windedupfilterdatas[2] = ESC_filter((uint32_t)windedupfilterdatas[2],(uint32_t)((serialBuf[111+STARTCOUNT]<<8) | serialBuf[112+STARTCOUNT])/(MAGNETPOLECOUNT/2)<<4);
           windedupfilterdatas[3] = ESC_filter((uint32_t)windedupfilterdatas[3],(uint32_t)((serialBuf[121+STARTCOUNT]<<8) | serialBuf[122+STARTCOUNT])/(MAGNETPOLECOUNT/2)<<4);
           
           motorKERPM[0] = windedupfilterdatas[0]>>4;
           motorKERPM[1] = windedupfilterdatas[1]>>4;
           motorKERPM[2] = windedupfilterdatas[2]>>4;
           motorKERPM[3] = windedupfilterdatas[3]>>4;
           MaxRPMs = findMaxUint16(MaxRPMs, motorKERPM, 4);
           
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
           MaxTemp = findMaxUint16(MaxTemp, ESCTemps, 4);

           AuxChanVals[0] = ((serialBuf[8+STARTCOUNT]<<8) | serialBuf[9+STARTCOUNT]);
           AuxChanVals[1] = ((serialBuf[10+STARTCOUNT]<<8) | serialBuf[11+STARTCOUNT]);
           AuxChanVals[2] = ((serialBuf[12+STARTCOUNT]<<8) | serialBuf[13+STARTCOUNT]);
           AuxChanVals[3] = ((serialBuf[14+STARTCOUNT]<<8) | serialBuf[15+STARTCOUNT]);
           
           current = (uint16_t)(motorCurrent[0]+motorCurrent[1]+motorCurrent[2]+motorCurrent[3])/10;
           MaxAmps = findMax(MaxAmps, current);
           uint32_t Watts = (uint32_t)LipoVoltage * (uint32_t)(current * 10);
           MaxWatt = findMax(MaxWatt, (uint16_t) (Watts / 1000));
        }
      }
    }
    
  
    while (!OSD.notInVSync());
    
    
    for(i=0;i<10;i++){
      Motor1KERPM[i] = ' ';
      Motor2KERPM[i] = ' ';
      Motor3KERPM[i] = ' ';
      Motor4KERPM[i] = ' ';
      
      Motor1Current[i] = ' ';
      Motor2Current[i] = ' ';
      Motor3Current[i] = ' ';
      Motor4Current[i] = ' ';
      
      ESC1Temp[i] = ' ';
      ESC2Temp[i] = ' ';
      ESC3Temp[i] = ' ';
      ESC4Temp[i] = ' ';
      
      LipoVoltC[i] = ' ';
      LipoMinVoltC[i] = ' ';
      LipoMAHC[i] = ' ';
      Throttle[i] = ' ';
      MaxTempC[i] = ' ';
      MaxAmpC[i] = ' ';
      MaxRPMC[i] = ' ';
      MaxWattC[i] = ' ';
    }
    
    
    uint8_t ThrottlePos = print_int16(throttle, Throttle,0,1);
    Throttle[ThrottlePos++] = '%';
    
    uint8_t CurrentPos = print_int16(current, Current,1,0);
    Current[CurrentPos++] = 'a';
    Current[CurrentPos++] = 't';

    KRPMPoses[0] = print_int16(motorKERPM[0], Motor1KERPM,1,1);
    Motor1KERPM[KRPMPoses[0]++] = 'k';
    Motor1KERPM[KRPMPoses[0]++] = 'r';
    
    KRPMPoses[1] = print_int16(motorKERPM[1], Motor2KERPM,1,0);
    Motor2KERPM[KRPMPoses[1]++] = 'k';
    Motor2KERPM[KRPMPoses[1]++] = 'r';
    
    KRPMPoses[2] = print_int16(motorKERPM[2], Motor3KERPM,1,0);
    Motor3KERPM[KRPMPoses[2]++] = 'k';
    Motor3KERPM[KRPMPoses[2]++] = 'r';
   
    KRPMPoses[3] = print_int16(motorKERPM[3], Motor4KERPM,1,1);
    Motor4KERPM[KRPMPoses[3]++] = 'k';
    Motor4KERPM[KRPMPoses[3]++] = 'r';
    
    
    CurrentPoses[0] = print_int16(motorCurrent[0], Motor1Current,2,1);
    Motor1Current[CurrentPoses[0]++] = 'a';
    
    CurrentPoses[1] = print_int16(motorCurrent[1], Motor2Current,2,0);
    Motor2Current[CurrentPoses[1]++] = 'a';
    
    CurrentPoses[2] = print_int16(motorCurrent[2], Motor3Current,2,0);
    Motor3Current[CurrentPoses[2]++] = 'a';
   
    CurrentPoses[3] = print_int16(motorCurrent[3], Motor4Current,2,1);
    Motor4Current[CurrentPoses[3]++] = 'a';
    
    
    
    TempPoses[0] = print_int16(ESCTemps[0], ESC1Temp,0,1);
    ESC1Temp[TempPoses[0]++] = '°';
    
    TempPoses[1] = print_int16(ESCTemps[1], ESC2Temp,0,0);
    ESC2Temp[TempPoses[1]++] = '°';
    
    TempPoses[2] = print_int16(ESCTemps[2], ESC3Temp,0,0);
    ESC3Temp[TempPoses[2]++] = '°';
   
    TempPoses[3] = print_int16(ESCTemps[3], ESC4Temp,0,1);
    ESC4Temp[TempPoses[3]++] = '°';

    uint8_t lipoVoltPos = print_int16(LipoVoltage, LipoVoltC,2,1);
    LipoVoltC[lipoVoltPos++] = 'v';
    uint8_t lipoMinVoltPos = print_int16(MinBat, LipoMinVoltC,2,1);
    LipoMinVoltC[lipoMinVoltPos++] = 'v';
    
    uint8_t lipoMAHPos = print_int16(LipoMAH, LipoMAHC,0,1);
    LipoMAHC[lipoMAHPos++] = 'm';
    LipoMAHC[lipoMAHPos++] = 'a';
    LipoMAHC[lipoMAHPos++] = 'h';
    
    uint8_t pos = print_int16(MaxAmps, MaxAmpC,1,1);
    MaxAmpC[pos++] = 'a';

    pos = print_int16(MaxRPMs, MaxRPMC,1,1);
    MaxRPMC[pos++] = 'k';
    MaxRPMC[pos++] = 'r';
    
    pos = print_int16(MaxWatt, MaxWattC,1,1);
    MaxWattC[pos++] = 'w';

    uint8_t ESCmarginBot       = 0;
    uint8_t ESCmarginTop       = 0;
    uint8_t TMPmargin          = 0;
    uint8_t CurrentMargin      = 0;
    uint8_t middle_infos_y     = 7;

    uint8_t displayNickname    = 0;
    uint8_t displayRCthrottle  = 0;
    uint8_t displayCombCurrent = 0;
    uint8_t displayLipoVoltage = 0;
    uint8_t displayConsumption = 0;
    uint8_t displayKRPM        = 0;
    uint8_t displayCurrent     = 0;
    uint8_t displayTemperature = 0;
    uint8_t displayStats       = 0;
    uint8_t displayTime        = 0;
    
     
    
    #if(RED_MODE_AUX_CHAN != 0)
    
      #if defined(RED_ON_AUX_LOW)
        #define RED_MODE_ACTIVE AuxChanVals[RED_MODE_AUX_CHAN-1] < -250
      #endif
      #if defined(RED_ON_AUX_MID)
        #define RED_MODE_ACTIVE AuxChanVals[RED_MODE_AUX_CHAN-1] > -250 && AuxChanVals[RED_MODE_AUX_CHAN-1] < 250
      #endif
      #if defined(RED_ON_AUX_HIGH)
        #define RED_MODE_ACTIVE AuxChanVals[RED_MODE_AUX_CHAN-1] > 250
      #endif
      
      if(RED_MODE_ACTIVE)reducedMode = 1;
      else reducedMode = 0;
    #endif
    // debug
    #ifdef RED_DISPLAY_DEBUG
    reducedMode = 1;
    #endif
    
    #if(USE_DV == 1)
    reducedMode = 1;
    #endif
    
    if(reducedMode != lastMode || lastAuxVal != AuxChanVals[RED_MODE_AUX_CHAN-1]){
      lastMode = reducedMode;
      lastAuxVal = AuxChanVals[RED_MODE_AUX_CHAN-1];
      cleanScreen();
    }
    
    if(reducedMode == 0){
      #if defined(DISPLAY_NICKNAME)
      displayNickname = 1;
      #endif
      #if defined(DISPLAY_RC_THROTTLE)
      displayRCthrottle = 1;
      #endif
      #if defined(DISPLAY_COMB_CURRENT)
      displayCombCurrent = 1;
      #endif
      #if defined(DISPLAY_LIPO_VOLTAGE)
      displayLipoVoltage = 1;
      #endif
      #if defined(DISPLAY_MA_CONSUMPTION)
      displayConsumption = 1;
      #endif
      #if defined(DISPLAY_ESC_KRPM)
      displayKRPM = 1;
      #endif
      #if defined(DISPLAY_ESC_CURRENT)
      displayCurrent = 1;
      #endif
      #if defined(DISPLAY_ESC_TEMPERATURE)
      displayTemperature = 1;
      #endif      
      #if defined(DISPLAY_TIMER)
      displayTime = 1;
      #endif 
    }else{
      #if defined(RED_DISPLAY_NICKNAME)
      displayNickname = 1;
      #endif
      #if defined(RED_DISPLAY_RC_THROTTLE)
      displayRCthrottle = 1;
      #endif
      #if defined(RED_DISPLAY_COMB_CURRENT)
      displayCombCurrent = 1;
      #endif
      #if defined(RED_DISPLAY_LIPO_VOLTAGE)                                                                                 
      displayLipoVoltage = 1;
      #endif
      #if defined(RED_DISPLAY_MA_CONSUMPTION)
      displayConsumption = 1;
      #endif
      #if defined(RED_DISPLAY_ESC_KRPM)
      displayKRPM = 1;
      #endif
      #if defined(RED_DISPLAY_ESC_CURRENT)
      displayCurrent = 1;
      #endif
      #if defined(RED_DISPLAY_ESC_TEMPERATURE)
      displayTemperature = 1;
      #endif    

      #if defined(RED_DISPLAY_STATS)
      #if defined(RED_DISPLAY_TIMER)
      // if armed at least 1x
      if (total_time > 10000) {
      #endif    
        displayStats = 1;
      #if defined(RED_DISPLAY_TIMER)
      }
      #endif    
      #endif    

      #if defined(RED_DISPLAY_TIMER)
      displayTime = 1;
      #endif 
    }

    print_time(time, Time);
    if(armed == 0 && armedOnce && last_Red_Aux_Val != AuxChanVals[RED_MODE_AUX_CHAN-1]) {
      red_DV_change_time = millis();
    }
    if(BAT_AUX_DV_CHANNEL > 0 && armedOnce && armed != 0 && last_Bat_Aux_Val != AuxChanVals[BAT_AUX_DV_CHANNEL-1]) {
      last_Bat_Aux_Val = AuxChanVals[BAT_AUX_DV_CHANNEL-1];
    }
    if(start_time > 0 && (millis() - start_time) > 2000) {
      unblock_bat_DV = false;
    }
    if(unblock_bat_DV && (save_bat_warning || (armedOnce && armed == 0 && BAT_AUX_DV_CHANNEL > 0 && last_Bat_Aux_Val != AuxChanVals[BAT_AUX_DV_CHANNEL-1]))) {
      if(!save_bat_warning) {
        cleanScreen();
      }
      else {
        if(abs(abs(last_Bat_Aux_Val) - abs(AuxChanVals[BAT_AUX_DV_CHANNEL-1])) > DV_JITTER) {
          if(last_Bat_Aux_Val < AuxChanVals[BAT_AUX_DV_CHANNEL-1]) {
            if((bat_mAh_warning + BAT_MAH_INCREMENT) < 32000) {
              bat_mAh_warning += BAT_MAH_INCREMENT;
            }
          }
          if(last_Bat_Aux_Val > AuxChanVals[BAT_AUX_DV_CHANNEL-1]) {
            if((bat_mAh_warning - BAT_MAH_INCREMENT) >= 300) {
              bat_mAh_warning -= BAT_MAH_INCREMENT;
            }
          }
        }
      }      
      OSD.setCursor(3,7);
      OSD.print( "new bat warning mah:" );
      OSD.setCursor(8,8);
      static char mAh_Warning[30];
      print_int16(bat_mAh_warning, mAh_Warning,0,0);
      OSD.print(mAh_Warning);
      OSD.setCursor(6,9);
      OSD.print( "arm to save" );
      save_bat_warning = true;
      if(abs(abs(last_Bat_Aux_Val) - abs(AuxChanVals[BAT_AUX_DV_CHANNEL-1])) > DV_JITTER) {
        last_Bat_Aux_Val = AuxChanVals[BAT_AUX_DV_CHANNEL-1];
      }
    }
    else {
      if(red_DV_change_time == 0 && armed == 0 && (displayStats || (AuxChanVals[RED_MODE_AUX_CHAN-1] > RED_DISPLAY_STATS_DV)) && armedOnce) {
        if(displayStats || (AuxChanVals[RED_MODE_AUX_CHAN-1] > RED_DISPLAY_STATS_DV)){
          middle_infos_y = middle_infos_y - 4;
        }
        if(displayNickname || (AuxChanVals[RED_MODE_AUX_CHAN-1] > RED_DISPLAY_NICKNAME_DV)){
          OSD.setCursor( 11, middle_infos_y );
          OSD.print( NICKNAME );
        }
        if (displayStats || (AuxChanVals[RED_MODE_AUX_CHAN-1] > RED_DISPLAY_STATS_DV)){
          middle_infos_y++;
  
          print_time(total_time, TotalTime);
          OSD.setCursor( 5, ++middle_infos_y );
          OSD.print( "time     : " );
          OSD.print( TotalTime );
  
          OSD.setCursor( 5, ++middle_infos_y );
          OSD.print( "max amps : " );
          OSD.print( MaxAmpC );
          if (MaxC > 0) {
            OSD.print( "a | " );
            OSD.print( MaxC );
            OSD.print( "c     " );
          }
  
          OSD.setCursor( 5, ++middle_infos_y );
          OSD.print( "mah      : " );
          OSD.print( LipoMAHC );
  
          OSD.setCursor( 5, ++middle_infos_y );
          OSD.print( "max rpm : " );
          OSD.print( MaxRPMC );
  
          OSD.setCursor( 5, ++middle_infos_y );
          OSD.print( "max watt : " );
          OSD.print( MaxWattC );
  
          OSD.setCursor( 5, ++middle_infos_y );
          OSD.print( "max temp : " );
          uint8_t MaxTempPos = print_int16(MaxTemp, MaxTempC,0,1);
          MaxTempC[MaxTempPos++] = '°';
          OSD.print( MaxTempC );
          OSD.print( "        " );
  
          OSD.setCursor( 5, ++middle_infos_y );
          OSD.print( "min v    : " );
          OSD.print( LipoMinVoltC );
        }
      }
      else {
        if(displayRCthrottle || (AuxChanVals[RED_MODE_AUX_CHAN-1] > RED_DISPLAY_RC_THROTTLE_DV)){
          OSD.setCursor( 0, 0 );
          //OSD.print( "throt:" );
          OSD.print( Throttle );
          ESCmarginTop = 1;
        }
          
        if(displayNickname || (AuxChanVals[RED_MODE_AUX_CHAN-1] > RED_DISPLAY_NICKNAME_DV)){
          OSD.setCursor( 10, -1 );
          OSD.print( NICKNAME );
        }
    
        if(displayCombCurrent || (AuxChanVals[RED_MODE_AUX_CHAN-1] > RED_DISPLAY_COMB_CURRENT_DV)){
          OSD.setCursor( -CurrentPos, 0 );
          OSD.print( Current );
          ESCmarginTop = 1;
        }
        
        if(displayLipoVoltage || (AuxChanVals[RED_MODE_AUX_CHAN-1] > RED_DISPLAY_LIPO_VOLTAGE_DV)){
          OSD.setCursor( 0, -1 );
          //OSD.print( "bat:" );
          OSD.print( LipoVoltC );
          ESCmarginBot = 1;
        }
        
        if(displayConsumption || (AuxChanVals[RED_MODE_AUX_CHAN-1] > RED_DISPLAY_MA_CONSUMPTION_DV)){
          OSD.setCursor( -(1+lipoMAHPos), -1 );
          OSD.print( LipoMAHC );
          ESCmarginBot = 1;
        }
        
        if(displayKRPM  || (AuxChanVals[RED_MODE_AUX_CHAN-1] > RED_DISPLAY_ESC_KRPM_DV)){
          OSD.setCursor( 0, ESCmarginTop );
          OSD.print( Motor1KERPM );
          OSD.setCursor( -KRPMPoses[1], ESCmarginTop );
          OSD.print( Motor2KERPM );
          OSD.setCursor( -KRPMPoses[2], -(1+ESCmarginBot) );
          OSD.print( Motor3KERPM );
          OSD.setCursor( 0, -(1+ESCmarginBot) );
          OSD.print( Motor4KERPM );
          TMPmargin++;
          CurrentMargin++;
        }
     
        if(displayCurrent  || (AuxChanVals[RED_MODE_AUX_CHAN-1] > RED_DISPLAY_ESC_CURRENT_DV)){
          OSD.setCursor( 0, CurrentMargin+ESCmarginTop );
          OSD.print( Motor1Current );
          OSD.setCursor( -CurrentPoses[1], CurrentMargin+ESCmarginTop );
          OSD.print( Motor2Current );
          OSD.setCursor( -CurrentPoses[2], -(1+CurrentMargin+ESCmarginBot) );
          OSD.print( Motor3Current );
          OSD.setCursor( 0, -(1+CurrentMargin+ESCmarginBot) );
          OSD.print( Motor4Current );
          TMPmargin++;
        }
    
        if(displayTemperature  || (AuxChanVals[RED_MODE_AUX_CHAN-1] > RED_DISPLAY_ESC_TEMPERATURE_DV)){
          OSD.setCursor( 0, TMPmargin+ESCmarginTop );
          OSD.print( ESC1Temp );
          OSD.setCursor( -TempPoses[1], TMPmargin+ESCmarginTop );
          OSD.print( ESC2Temp );
          OSD.setCursor( -TempPoses[2], -(1+TMPmargin+ESCmarginBot) );
          OSD.print( ESC3Temp );
          OSD.setCursor( 0, -(1+TMPmargin+ESCmarginBot) );
          OSD.print( ESC4Temp );
        }  
    
        if((displayTime || (AuxChanVals[RED_MODE_AUX_CHAN-1] > RED_DISPLAY_TIMER_DV)) && time > 0) {
          OSD.setCursor( 12, -2 );
          OSD.print( Time );
          //ESCmarginTop = 1;
        }
        
        if(bat_mAh_warning > 0 && LipoMAH >= bat_mAh_warning){
          OSD.setCursor(9,9);
          if (blink_i % 20 == 0) {
            if(bat_clear) {
              OSD.print( "BAT LOW!!!" );
              bat_clear = false;
            }
            else {
              OSD.print( "          " );
              bat_clear = true;
            }
          }
        }
        if(red_DV_change_time > 0 && (millis() - red_DV_change_time) > 3000 && last_Red_Aux_Val == AuxChanVals[RED_MODE_AUX_CHAN-1]) {
          red_DV_change_time = 0;
        }
      }
    }
    /*static char Aux1[30];
    print_int16(AuxChanVals[RED_MODE_AUX_CHAN-1], Aux1,0,0);
    OSD.setCursor(8,10);
    OSD.print(Aux1);
      
    if (blink_i % 10 == 0) {
      middle_infos_y -= 6;
      OSD.setCursor( 0, middle_infos_y );
      OSD.print( "armed=" );
      OSD.print(armed);
      OSD.print( " mode=" );
      OSD.print(mode);
      OSD.print( " idle=" );
      OSD.print(idleTime);
      OSD.print( "         " );

      middle_infos_y++;
      OSD.setCursor( 0, middle_infos_y );
      OSD.print( "calyb=" );
      OSD.print(calybGyro);
      OSD.print( " failsafe=" );
      OSD.print(failsafe);
      OSD.print( "         " );
      middle_infos_y++;
      for(i=0; i<=9; i++)
      {
          OSD.setCursor( 0, middle_infos_y+i );
          OSD.print(i+36);
          OSD.print(":");
          OSD.print(serialBuf[i+36]);
          OSD.print("     ");
      }
      for(i=0; i<=9; i++)
      {
          OSD.setCursor( 10, middle_infos_y+i );
          OSD.print(i+80);
          OSD.print(": ");
          OSD.print(serialBuf[i+80]);
          OSD.print("      ");
      }
    }

/*
    if(reducedMode == 1){
      OSD.setCursor( 10, -2 );
      if (calybGyro > 0) {
        if (blink_i >= 20) {
          OSD.print( "calibrating" );
        } else {
        }
      } else {
          OSD.print( "           " );
      }
      */
      /*
      if (armed == 0) {
        OSD.setCursor( 11, -4 );
        OSD.print( "disarmed" );
      }
    }
      */
  }    
}
