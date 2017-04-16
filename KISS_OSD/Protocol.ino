static uint8_t serialBuf[256];
static uint8_t minBytes = 0;
static uint8_t minBytesSettings = 0;
static uint8_t recBytes = 0;
const uint8_t filterCount = 5;
const uint8_t filterCount2 = 3;
//#ifndef IMPULSERC_VTX
CMeanFilter voltageFilter(filterCount), ampTotalFilter(filterCount);
CMeanFilter ESCKRfilter[4] = { CMeanFilter(filterCount2), CMeanFilter(filterCount2), CMeanFilter(filterCount2), CMeanFilter(filterCount2) }; 
CMeanFilter ESCAmpfilter[4] = { CMeanFilter(filterCount2), CMeanFilter(filterCount2), CMeanFilter(filterCount2), CMeanFilter(filterCount2) };
//#endif

boolean ReadTelemetry()
{
  uint16_t i = 0;
  minBytes = 100;
  recBytes = 0;
   
  while(recBytes < minBytes && micros()-LastLoopTime < 20000)
  {
    const uint8_t STARTCOUNT = 2;
    if(NewSerial.available()) serialBuf[recBytes++] = NewSerial.read();
    if(recBytes == 1 && serialBuf[0] != 5)recBytes = 0; // check for start byte, reset if its wrong
    if(recBytes == 2) 
    {
      minBytes = serialBuf[1]+STARTCOUNT+1; // got the transmission length
      if (minBytes<150 || minBytes>180)  
      {
         recBytes = 0;
         minBytes = 100;
      }
    }
    if(recBytes == minBytes)
    {
       uint32_t checksum = 0;
       for(i=2;i<minBytes;i++){
          checksum += serialBuf[i];
       }
       checksum = (uint32_t)checksum/(minBytes-3);       
       
       if(checksum == serialBuf[recBytes-1])
       {
        
         throttle = ((serialBuf[STARTCOUNT]<<8) | serialBuf[1+STARTCOUNT])/10;
         roll = 1000 + ((serialBuf[2+STARTCOUNT]<<8) | serialBuf[3+STARTCOUNT]);
         pitch = 1000 + ((serialBuf[4+STARTCOUNT]<<8) | serialBuf[5+STARTCOUNT]);
         yaw = 1000 + ((serialBuf[6+STARTCOUNT]<<8) | serialBuf[7+STARTCOUNT]);
         LipoVoltage = ((serialBuf[17+STARTCOUNT]<<8) | serialBuf[18+STARTCOUNT]);
         angleX = ((serialBuf[31+STARTCOUNT]<<8) | serialBuf[32+STARTCOUNT])/100;
         angleY = ((serialBuf[33+STARTCOUNT]<<8) | serialBuf[34+STARTCOUNT])/100;


         int8_t current_armed = serialBuf[16+STARTCOUNT];
         if(current_armed < 0) return false;
         
         // switch disarmed => armed
         if (armed == 0 && current_armed > 0) 
         {
           if(settings.m_airTimer == 0)
           {
              start_time = millis();
           }
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
             settings.UpdateMaxWatt(MaxWatt);
           } 
           else if (armed > 0) 
           {
             if(throttle < 5 && settings.m_airTimer == 1 && start_time == 0) 
             {
              time = 0;
             }
             else
             {
              if(settings.m_airTimer == 1 && start_time == 0) start_time = millis();
              time = millis() - start_time;
             }             
           }
         }
         armed = current_armed;
         
         uint32_t tmp32 = 0;
         uint8_t voltDev = 0;
         if(((serialBuf[85+STARTCOUNT]<<8) | serialBuf[86+STARTCOUNT]) > 5){ // the ESC's read the voltage better then the FC
           ESCVoltage[0] = ((serialBuf[85+STARTCOUNT]<<8) | serialBuf[86+STARTCOUNT]);
           tmp32 += ESCVoltage[0];
           voltDev++;
         }
         if(((serialBuf[95+STARTCOUNT]<<8) | serialBuf[96+STARTCOUNT]) > 5){ 
           ESCVoltage[1] = ((serialBuf[95+STARTCOUNT]<<8) | serialBuf[96+STARTCOUNT]);
           tmp32 += ESCVoltage[1];
           voltDev++;
         }
         if(((serialBuf[105+STARTCOUNT]<<8) | serialBuf[106+STARTCOUNT]) > 5){
           ESCVoltage[2] = ((serialBuf[105+STARTCOUNT]<<8) | serialBuf[106+STARTCOUNT]);
           tmp32 += ESCVoltage[2];
           voltDev++;
         }
         if(((serialBuf[115+STARTCOUNT]<<8) | serialBuf[116+STARTCOUNT]) > 5){ 
           ESCVoltage[3] = ((serialBuf[115+STARTCOUNT]<<8) | serialBuf[116+STARTCOUNT]);
           tmp32 += ESCVoltage[3];
           voltDev++;
         }
         
         if(voltDev!=0) 
         {
          tmp32 = tmp32/(uint32_t)voltDev;  
          LipoVoltage = (int16_t)tmp32;
         }
         LipoVoltage += settings.m_voltCorrect * 10;
         
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

         ESCmAh[0] = ((serialBuf[89+STARTCOUNT]<<8) | serialBuf[90+STARTCOUNT]); 
         ESCmAh[1] = ((serialBuf[99+STARTCOUNT]<<8) | serialBuf[100+STARTCOUNT]);
         ESCmAh[2] = ((serialBuf[109+STARTCOUNT]<<8) | serialBuf[110+STARTCOUNT]);
         ESCmAh[3] = ((serialBuf[119+STARTCOUNT]<<8) | serialBuf[120+STARTCOUNT]);        

         AuxChanVals[0] = ((serialBuf[8+STARTCOUNT]<<8) | serialBuf[9+STARTCOUNT]);
         AuxChanVals[1] = ((serialBuf[10+STARTCOUNT]<<8) | serialBuf[11+STARTCOUNT]);
         AuxChanVals[2] = ((serialBuf[12+STARTCOUNT]<<8) | serialBuf[13+STARTCOUNT]);
         AuxChanVals[3] = ((serialBuf[14+STARTCOUNT]<<8) | serialBuf[15+STARTCOUNT]);
         
         current = (uint16_t)(motorCurrent[0]+motorCurrent[1]+motorCurrent[2]+motorCurrent[3])/10;
         
         uint8_t i;
         
         // Data sanity check. Return false if we get invalid data
         for(i=0; i<4; i++)
         {
           if(ESCTemps[i] < -50 || ESCTemps[i] > 100 ||
              motorCurrent[i] < 0 || motorCurrent[i] > 10000 ||
              motorKERPM[i] < 0 || motorKERPM[i] > 500 ||
              ESCVoltage[i] < 0 || ESCVoltage[i] > 10000 ||
              ESCmAh[i] < 0)
           {
             return false;
           }
         }
         if(LipoVoltage < 0 || LipoVoltage > 10000 ||
            throttle < 0 || throttle > 100 ||
            roll < 0 || roll > 2000 ||
            pitch < 0 || pitch > 2000 ||
            yaw < 0 || yaw > 2000 ||
            LipoMAH < 0)
         {
           return false;
         }
            
         
         if(settings.m_tempUnit == 1)
         {
           for(i=0; i<4; i++)
           {
             ESCTemps[i] = 9 * ESCTemps[i] / 5 + 32;
             if(lastTempUnit == 0)
             {
               maxTemps[i] = 9 * maxTemps[i] / 5 + 32;
             }
           }
           if(lastTempUnit == 0)
           {
             MaxTemp = 9 * MaxTemp / 5 + 32;
           }
         }
         if(lastTempUnit == 1 && settings.m_tempUnit == 0)
         {
           for(i=0; i<4; i++)
           {
             maxTemps[i] = (maxTemps[i] - 32) * 5 / 9;
           }
           MaxTemp = (MaxTemp - 32) * 5 / 9;
         }
         lastTempUnit = settings.m_tempUnit;
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
           tmp32 = (uint32_t)MaxAmps * 100 / (uint32_t)settings.m_batMAH[settings.m_activeBattery];
           MaxC = (int16_t) tmp32;
           tmp32 = (uint32_t)LipoVoltage * (uint32_t)(current * 10);
           MaxWatt = findMax(MaxWatt, (uint16_t) (tmp32 / 1000));
           if(MaxWatt > settings.m_maxWatts) settings.m_maxWatts = MaxWatt;
           for(i=0; i<4; i++)
           {
             maxKERPM[i] = findMax(maxKERPM[i], motorKERPM[i]);
             maxCurrent[i] = findMax(maxCurrent[i], motorCurrent[i]);
             maxTemps[i] = findMax(maxTemps[i], ESCTemps[i]);
             minVoltage[i] = findMin(minVoltage[i], ESCVoltage[i]);
           }
         }
         LipoVoltageRT = LipoVoltage;
         //#ifndef IMPULSERC_VTX
         LipoVoltage = voltageFilter.ProcessValue(LipoVoltage);
         //#endif
         currentRT = current;
         //#ifndef IMPULSERC_VTX
         current = ampTotalFilter.ProcessValue(current);
         //#endif
         for(i=0; i<4; i++)
         {
           motorKERPMRT[i] = motorKERPM[i];
           //#ifndef IMPULSERC_VTX
           motorKERPM[i] = ESCKRfilter[i].ProcessValue(motorKERPM[i]);
           motorCurrent[i] = ESCAmpfilter[i].ProcessValue(motorCurrent[i]);
           //#endif
         }         
      }
      else
      {
        return false;
      }
    }
  }
  if(recBytes != minBytes)
  {
    return false;
  }
  return true;
}

uint8_t getCheckSum(uint8_t *buf, uint8_t startIndex, uint8_t stopIndex)
{
   double checksum = 0.0;
   double dataCount = 0.0;
   uint8_t i;       
   for(i=startIndex;i<stopIndex;i++)
   {
      checksum += buf[i];
      dataCount++;
   }
   return (uint8_t)floor(checksum/dataCount);  
}

#ifndef KISS_OSD_CONFIG

extern unsigned long _StartupTime;
//static uint8_t serialBuf2[256];
#define serialBuf2 serialBuf

#ifdef NEW_FC_SETTINGS
void ReadFCSettings(boolean skipValues, uint8_t sMode)
{
  uint8_t i = 0;
  minBytes = 100;
  recBytes = 0;
   
  while(recBytes < minBytes && micros()-LastLoopTime < 20000)
  {
    uint8_t STARTCOUNT = 2;
    while(NewSerial.available()) serialBuf2[recBytes++] = NewSerial.read();
    if(getSettingModes[sMode] == 0x30)
    {
      if(recBytes == 1 && serialBuf2[0] != 5) recBytes = 0; // check for start byte, reset if its wrong
      if(recBytes == 2) minBytes = serialBuf2[1]+STARTCOUNT+1; // got the transmission length
    }
    else
    {
      if(recBytes == 1 && serialBuf2[0] != getSettingModes[sMode]) recBytes = 0;
      if(recBytes == 2) minBytes = serialBuf2[1] + 3; // got the transmission length
    }
    if(recBytes == minBytes)
    {
       if(millis()-_StartupTime < 5000)
       {
         return; //throwing away garbage data sent by FC during startup phase
       }

       uint8_t stopByte = minBytes;
       if(getSettingModes[sMode] > 0x30) stopByte--;
       uint8_t checksum2 = getCheckSum(serialBuf2, STARTCOUNT, stopByte);       

       uint8_t index = 0;
       if(checksum2 == serialBuf2[recBytes-1])
       {        
         if(!skipValues)
         {
           switch(sMode)
           {
              case FC_SETTINGS:
                 protoVersion = serialBuf2[92+STARTCOUNT];
                 if(serialBuf2[73+STARTCOUNT] > 0)
                 {
                   armOnYaw = false;
                 }
                 if(serialBuf2[55+STARTCOUNT] > 2)
                 {
                   dShotEnabled = true;
                 }                       
              break;
              case FC_RATES:
                 rcrate_roll = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 rate_roll = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 rccurve_roll = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 rcrate_pitch = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 rate_pitch = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 rccurve_pitch = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 rcrate_yaw = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 rate_yaw = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 rccurve_yaw = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
              break;
              case FC_PIDS:
                 p_roll = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 i_roll = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 d_roll = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 p_pitch = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 i_pitch = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 d_pitch = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 p_yaw = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 i_yaw = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 d_yaw = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
              break;
              case FC_VTX:
                 vTxType = serialBuf2[index+STARTCOUNT];
                 index++;
                 vTxChannel = serialBuf2[index+STARTCOUNT];            
                 vTxBand = vTxChannel / 8;
                 oldvTxBand = vTxBand;
                 vTxChannel %= 8;
                 oldvTxChannel = vTxChannel;                 
                 index++;             
                 vTxLowPower = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 oldvTxLowPower = vTxLowPower;
                 index += 2;
                 vTxHighPower = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 oldvTxHighPower = vTxHighPower;
              break;
              case FC_FILTERS:
                 lpf_frq = serialBuf2[index+STARTCOUNT];
                 index++;
                 yawFilterCut = (int16_t)serialBuf2[index+STARTCOUNT];
                 index++;
                 notchFilterEnabledR = serialBuf2[index+STARTCOUNT];
                 index++;
                 notchFilterCenterR = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 notchFilterCutR = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 notchFilterEnabledP = serialBuf2[index+STARTCOUNT];
                 index++;
                 notchFilterCenterP = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 notchFilterCutP = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);                 
              break;
              case FC_TPA:
                 p_tpa = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 i_tpa = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 d_tpa = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
                 index += 2;
                 customTPAEnabled = serialBuf2[index+STARTCOUNT];
                 index++;
                 ctpa_bp1 = serialBuf2[index+STARTCOUNT];
                 index++;
                 ctpa_bp2 = serialBuf2[index+STARTCOUNT];
                 index++;
                 ctpa_infl0 = serialBuf2[index+STARTCOUNT];
                 index++;
                 ctpa_infl1 = serialBuf2[index+STARTCOUNT];
                 index++;
                 ctpa_infl2 = serialBuf2[index+STARTCOUNT];
                 index++;
                 ctpa_infl3 = serialBuf2[index+STARTCOUNT];
              break;
           }
         }
         fcSettingsReceived = true;
       }
     }
  }
}

void SendFCSettings(uint8_t sMode)
{
  const uint8_t STARTCOUNT = 1;
  uint8_t index = 0;
  switch(sMode)
  {
    case FC_SETTINGS:
      return;
    break;
    case FC_RATES:
      serialBuf2[STARTCOUNT+index++] = (byte)((rcrate_roll & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(rcrate_roll & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((rate_roll & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(rate_roll & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((rccurve_roll & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(rccurve_roll & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((rcrate_pitch & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(rcrate_pitch & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((rate_pitch & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(rate_pitch & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((rccurve_pitch & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(rccurve_pitch & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((rcrate_yaw & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(rcrate_yaw & 0x00FF);            
      serialBuf2[STARTCOUNT+index++] = (byte)((rate_yaw & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(rate_yaw & 0x00FF);            
      serialBuf2[STARTCOUNT+index++] = (byte)((rccurve_yaw & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(rccurve_yaw & 0x00FF);
    break;
    case FC_PIDS:
      serialBuf2[STARTCOUNT+index++] = (byte)((p_roll & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(p_roll & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((i_roll & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(i_roll & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((d_roll & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(d_roll & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((p_pitch & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(p_pitch & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((i_pitch & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(i_pitch & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((d_pitch & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(d_pitch & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((p_yaw & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(p_yaw & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((i_yaw & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(i_yaw & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((d_yaw & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(d_yaw & 0x00FF);
    break;  
    case FC_VTX:
      serialBuf2[STARTCOUNT+index++] = (byte) vTxType;
      serialBuf2[STARTCOUNT+index++] = (byte)((vTxBand*8) + vTxChannel);
      serialBuf2[STARTCOUNT+index++] = (byte)((vTxLowPower & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(vTxLowPower & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((vTxHighPower & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(vTxHighPower & 0x00FF);
    break;
    case FC_FILTERS:
      serialBuf2[STARTCOUNT+index++] = lpf_frq;
      serialBuf2[STARTCOUNT+index++] = (byte) yawFilterCut;
      serialBuf2[STARTCOUNT+index++] = (byte) notchFilterEnabledR;
      serialBuf2[STARTCOUNT+index++] = (byte)((notchFilterCenterR & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(notchFilterCenterR & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((notchFilterCutR & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(notchFilterCutR & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte) notchFilterEnabledP;
      serialBuf2[STARTCOUNT+index++] = (byte)((notchFilterCenterP & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(notchFilterCenterP & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((notchFilterCutP & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(notchFilterCutP & 0x00FF);      
    break;
    case FC_TPA:
      serialBuf2[STARTCOUNT+index++] = (byte)((p_tpa & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(p_tpa & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((i_tpa & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(i_tpa & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((d_tpa & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(d_tpa & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)customTPAEnabled;
      serialBuf2[STARTCOUNT+index++] = (byte)ctpa_bp1;
      serialBuf2[STARTCOUNT+index++] = (byte)ctpa_bp2;
      serialBuf2[STARTCOUNT+index++] = (byte)ctpa_infl0;
      serialBuf2[STARTCOUNT+index++] = (byte)ctpa_infl1;
      serialBuf2[STARTCOUNT+index++] = (byte)ctpa_infl2;
      serialBuf2[STARTCOUNT+index++] = (byte)ctpa_infl3;
    break;
  }
  
  serialBuf2[0] = index;
  serialBuf2[STARTCOUNT+index] = getCheckSum(serialBuf2, 1, STARTCOUNT+index);
  
  NewSerial.write(setSettingModes[sMode]); //Set settings
  uint8_t i; 
  for(i=0;i<(STARTCOUNT+index+1);i++)
  {
    NewSerial.write(serialBuf2[i]);
  }
}

#else
static boolean shiftedSettings = false;

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
       if(millis()-_StartupTime < 5000)
       {
         return; //throwing away garbage data sent by FC during startup phase
       }
       
       double checksum = 0.0;
       double dataCount = 0.0;
       for(i=2;i<minBytes;i++){
          checksum += serialBuf2[i];
          dataCount++;
       }
       uint8_t checksum2 = (uint8_t)floor(checksum/dataCount);
       
       #ifdef PROTODEBUG
       checkCalced = checksum;
       bufminus1 = serialBuf2[recBytes-1];
       #endif
       
       if(checksum2 == serialBuf2[minBytes-1] && minBytes > 119)// || (checksum-1) == serialBuf2[recBytes-1])
       //if(minBytes > 119)
       {
         minBytesSettings = minBytes;         
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
           
           if(serialBuf2[55+STARTCOUNT] > 2)
           {
             dShotEnabled = true;
           }

           index = 56;
           minCommand = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT])+1000;
           index += 4;
           minThrottle = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT])+1000;
           
           index = 73;
           protoVersion = serialBuf2[92+STARTCOUNT];
           if(protoVersion < 104)
           {
             if(serialBuf2[index+STARTCOUNT] == 1 || serialBuf2[index+STARTCOUNT] == 12 || serialBuf2[index+STARTCOUNT] == 13
             || serialBuf2[index+1+STARTCOUNT] == 1 || serialBuf2[index+1+STARTCOUNT] == 12 || serialBuf2[index+1+STARTCOUNT] == 13
             || serialBuf2[index+2+STARTCOUNT] == 1 || serialBuf2[index+2+STARTCOUNT] == 12 || serialBuf2[index+2+STARTCOUNT] == 13
             || serialBuf2[index+3+STARTCOUNT] == 1 || serialBuf2[index+3+STARTCOUNT] == 12 || serialBuf2[index+3+STARTCOUNT] == 13)
             {
               armOnYaw = false;
             }
           }
           else
           {
             if(serialBuf2[index+STARTCOUNT] > 0)
             {
               armOnYaw = false;
             }
             index = 138;
             notchFilterEnabledR = serialBuf2[index+STARTCOUNT];
             index++;
             notchFilterCenterR = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
             index += 2;
             notchFilterCutR = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
             index += 2;
             notchFilterEnabledP = serialBuf2[index+STARTCOUNT];
             index++;
             notchFilterCenterP = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
             index += 2;
             notchFilterCutP = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
             index += 2;
             uint8_t yawFilterCut2 = serialBuf2[index+STARTCOUNT];
             yawFilterCut = (int16_t)yawFilterCut2;             
           }
           
           index = 79;
           lpf_frq = serialBuf2[index+STARTCOUNT];
           
           index = 93;
           p_tpa = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           i_tpa = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
           index += 2;
           d_tpa = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);

           if(p_roll < 0 || i_roll < 0 || d_roll < 0 || 
              p_pitch < 0 || i_pitch < 0 || d_pitch < 0 ||
              p_yaw < 0 || i_yaw < 0 || d_yaw < 0 ||
              rcrate_roll < 0 || rcrate_pitch < 0 || rcrate_yaw < 0 ||
              rate_roll < 0 || rate_pitch < 0 || rate_yaw < 0 ||
              p_tpa < 0 || i_tpa < 0 || d_tpa < 0)
           {
              fcSettingsReceived = false;
              return;
           }
            
           if(protoVersion >= 106)
           {
             index = 120;
             vTxChannel = serialBuf2[index+STARTCOUNT];            
             vTxBand = vTxChannel / 8;
             oldvTxBand = vTxBand;
             vTxChannel %= 8;
             oldvTxChannel = vTxChannel;
             index = 149;
             vTxType = serialBuf2[index+STARTCOUNT];
             index++;             
             vTxLowPower = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
             oldvTxLowPower = vTxLowPower;
             index += 2;
             vTxHighPower = ((serialBuf2[index+STARTCOUNT]<<8) | serialBuf2[index+1+STARTCOUNT]);
             oldvTxHighPower = vTxHighPower;
           }

           //TODO: Shift after obj.lipoConnected = data.getUint8(154, 0); (for newer version)
           shiftedSettings = false; 
         }
         fcSettingsReceived = true;
       }       
    }
  }
}

static const uint8_t minVersionAllowed = 108;
static const uint8_t maxVersionAllowed = 108;

boolean SendFCSettings()
{
  if(fcSettingsReceived && protoVersion >= minVersionAllowed && protoVersion <= maxVersionAllowed)
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

    index = 56;
    serialBuf2[STARTCOUNT+index++] = (byte)(((minCommand-1000) & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)((minCommand-1000) & 0x00FF);
    index += 2;
    serialBuf2[STARTCOUNT+index++] = (byte)(((minThrottle-1000) & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)((minThrottle-1000) & 0x00FF);
    
    index = 79;
    serialBuf2[STARTCOUNT+index] = lpf_frq;
    
    //Need to shift data - but only once :)
    uint16_t i;
    if(!shiftedSettings)
    {
      for(i=93; i < 101; i++)
      {
        serialBuf2[STARTCOUNT+i-13] = serialBuf2[STARTCOUNT+i]; // skipping serial number
      }
      for(i=0; i<4; i++)
      {
        serialBuf2[STARTCOUNT+88+i] = 0; //controller activation
      }
      serialBuf2[STARTCOUNT+92] = serialBuf2[STARTCOUNT+101]; // data.setUint8(92, obj.BoardRotation, 0); obj.BoardRotation = data.getUint8(101);
      for(i=103; i < (256-STARTCOUNT); i++)
      {
        serialBuf2[STARTCOUNT+i-10] = serialBuf2[STARTCOUNT+i]; // skipped obj.isActive = data.getUint8(102);
      }
      shiftedSettings = true;
    }
    
    index = 80;
    serialBuf2[STARTCOUNT+index++] = (byte)((p_tpa & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(p_tpa & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((i_tpa & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(i_tpa & 0x00FF);
    serialBuf2[STARTCOUNT+index++] = (byte)((d_tpa & 0xFF00) >> 8);
    serialBuf2[STARTCOUNT+index++] = (byte)(d_tpa & 0x00FF);
    
    if(protoVersion > 104)
    {
      index = 128;
      serialBuf2[STARTCOUNT+index++] = (byte) notchFilterEnabledR;
      serialBuf2[STARTCOUNT+index++] = (byte)((notchFilterCenterR & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(notchFilterCenterR & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((notchFilterCutR & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(notchFilterCutR & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte) notchFilterEnabledP;
      serialBuf2[STARTCOUNT+index++] = (byte)((notchFilterCenterP & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(notchFilterCenterP & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((notchFilterCutP & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(notchFilterCutP & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte) yawFilterCut;
    }
    
    if(protoVersion > 106)
    {
      index = 110;
      serialBuf2[STARTCOUNT+index++] = (byte)((vTxBand*8) + vTxChannel);
      index = 140;
      serialBuf2[STARTCOUNT+index++] = (byte)((vTxLowPower & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(vTxLowPower & 0x00FF);
      serialBuf2[STARTCOUNT+index++] = (byte)((vTxHighPower & 0xFF00) >> 8);
      serialBuf2[STARTCOUNT+index++] = (byte)(vTxHighPower & 0x00FF);
    }
    
    double checksum = 0.0;
    double dataCount = 0.0;
    for(i=2;i<minBytesSettings;i++)
    {
     checksum += serialBuf2[i];
     dataCount++;
    }
    checksum = checksum/dataCount;
    serialBuf2[minBytesSettings-1] = floor(checksum);
    
    NewSerial.write(0x10); //Set settings
    for(i=0;i<minBytesSettings;i++)
    {
      NewSerial.write(serialBuf2[i]);
    }
    return true;
  }
  return false;
}
#endif
#endif
