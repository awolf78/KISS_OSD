static uint8_t minBytes = 100;
static uint8_t minBytesSettings = 0;
static uint8_t recBytes = 0;
#ifdef KISS_OSD_CONFIG
static const uint8_t protoVersion = 108;
#endif
#ifdef NEW_FILTER
const uint8_t filterCount = 10;
const uint8_t filterCount2 = 10;
#else
const uint8_t filterCount = 5;
const uint8_t filterCount2 = 5;
#endif
CMeanFilter voltageFilter(filterCount), ampTotalFilter(filterCount);
CMeanFilter ESCKRfilter[4] = { CMeanFilter(filterCount2), CMeanFilter(filterCount2), CMeanFilter(filterCount2), CMeanFilter(filterCount2) };
CMeanFilter ESCAmpfilter[4] = { CMeanFilter(filterCount2), CMeanFilter(filterCount2), CMeanFilter(filterCount2), CMeanFilter(filterCount2) };
static boolean airTimerStarted = false;
static uint8_t current_armed = 0;
static uint32_t tmp32 = 0;

inline void GenerateStats()
{
  uint8_t i;
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
  tmp32 = (uint32_t)MaxAmps * 10 / (uint32_t)settings.s.m_batMAH[settings.s.m_activeBattery];
  MaxC = (uint8_t) tmp32;
  tmp32 = (uint32_t)LipoVoltage * (uint32_t)current;
  MaxWatt = findMax(MaxWatt, (uint16_t) (tmp32 / 1000));
  for (i = 0; i < 4; i++)
  {
    maxKERPM[i] = findMax(maxKERPM[i], motorKERPM[i]);
    maxCurrent[i] = findMax(maxCurrent[i], motorCurrent[i]);
    maxTemps[i] = findMax(maxTemps[i], ESCTemps[i]);
    minVoltage[i] = findMin(minVoltage[i], ESCVoltage[i]);
  }
  #if defined(ADVANCED_STATS) || defined(ADVANCED_ESC_STATS)
  if(armed > 0)
  {
  #endif
  #ifdef ADVANCED_STATS
  for(i=0; i<STAT_GENERATOR_SIZE; i++)
  {
    statGenerators[i].StoreValue(current, throttle);
  }
  #endif
  #ifdef ADVANCED_ESC_STATS
  for(i=0; i<4; i++)
  {
    ESCstatGenerators[i].StoreValue(motorCurrent[i], throttle);
  }
  #endif
  #if defined(ADVANCED_STATS) || defined(ADVANCED_ESC_STATS)
  }
  #endif    
}

inline void ProcessConversionAndFilters()
{
  uint8_t i;
  if (settings.s.m_tempUnit == 1)
  {
    for (i = 0; i < 4; i++)
    {
      ESCTemps[i] = 9 * ESCTemps[i] / 5 + 32;
      if (lastTempUnit == 0)
      {
        maxTemps[i] = 9 * maxTemps[i] / 5 + 32;
      }
    }
    if (lastTempUnit == 0)
    {
      MaxTemp = 9 * MaxTemp / 5 + 32;
    }
  }
  if (lastTempUnit == 1 && settings.s.m_tempUnit == 0)
  {
    for (i = 0; i < 4; i++)
    {
      maxTemps[i] = (maxTemps[i] - 32) * 5 / 9;
    }
    MaxTemp = (MaxTemp - 32) * 5 / 9;
  }
  lastTempUnit = settings.s.m_tempUnit;

  
  LipoVoltage = voltageFilter.ProcessValue(LipoVoltage);
  current = ampTotalFilter.ProcessValue(current);
  for (i = 0; i < 4; i++)
  {
    motorKERPM[i] = ESCKRfilter[i].ProcessValue(motorKERPM[i]);
    motorCurrent[i] = ESCAmpfilter[i].ProcessValue(motorCurrent[i]);
  }
}

inline void ArmDisarmEvents()
{
  // switch disarmed => armed
  #ifdef ARMING_STATUS
  if(armed != current_armed) 
  {
    armingStatusChangeTime = millis();
  }
  #endif
  if (armed == 0 && current_armed > 0)
  {
    triggerCleanScreen = true;
    if(current_armed == 1) armedOnce = true;
    last_Aux_Val = AuxChanVals[settings.s.m_DVchannel];
    DV_change_time = 0;
    #ifndef KISS_OSD_CONFIG
    statsActive = false;
    #endif
    #ifdef RC_SPLIT_CONTROL
    if(settings.s.m_RCSplitControl > 0) newRCsplitState = true;
    #endif
  }
  // switch armed => disarmed
  else
  {
    if (armed > 0 && current_armed == 0)
    {
      if(settings.s.m_timerMode < 2) airTimerStarted = false;
      if(start_time > 0)
      {
        if(settings.s.m_timerMode < 2)
        {
          total_time = total_time + (millis() - start_time);
          start_time = 0;
        }
        else total_time = millis() - start_time;
      }             
      triggerCleanScreen = true;
      if (settings.s.m_batWarning > 0)
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
      #ifdef RC_SPLIT_CONTROL
      newRCsplitState = false;
      #endif
    }
    else if (armed > 0)
    {
      if (throttle < 5 && !airTimerStarted)
      {
        time = 0;
      }
      else
      {
        airTimerStarted = true;
        if (start_time == 0) start_time = millis();
        time = millis() - start_time;
      }
      if(settings.s.m_timerMode > 0) time += total_time;
    }
  }
  if(settings.s.m_timerMode == 2 && airTimerStarted) time = millis() - start_time;
  armed = current_armed;
}

uint8_t kissProtocolCRC8(const uint8_t *data, uint8_t startIndex, uint8_t stopIndex) 
{
  uint8_t crc = 0;
  for (uint8_t i = startIndex; i < stopIndex; i++) 
  {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) 
    {
      if ((crc & 0x80) != 0) 
      {
        crc = (uint8_t) ((crc << 1) ^ 0xD5);
      } 
      else 
      {
        crc <<= 1;
      }
    }
  }
  return crc;
}

#ifdef SERIAL_SETTINGS
static uint8_t zeroCount = 0, oneCount = 0;

inline void SerialSettings()
{
  if(recBytes == 1)
  {
    if(serialBuf[0] == 0) zeroCount++;
    else zeroCount = 0;
    if(serialBuf[0] == 1) oneCount++;
    else oneCount = 0;
    if(zeroCount == 5)
    {
      settings.WriteSettings(true);
      uint16_t i;
      for(i=0; i<5; i++) NewSerial.write((byte)0);
      serialBuf[0] = (uint8_t)(sizeof(settings.s)+2);
      serialBuf[1] = settings.m_settingVersion;
      for(i=0; i<(sizeof(settings.s)+2); i++) NewSerial.write(serialBuf[i]);
      NewSerial.write(kissProtocolCRC8(serialBuf, 0, sizeof(settings.s)+2));
      zeroCount = 0;
    }
    if(oneCount == 5)
    {
      unsigned long startSettingTime = micros();
      minBytes = 100;
      recBytes = 0;
      while(recBytes < minBytes && micros() - startSettingTime < 20000)
      {
        if(NewSerial.available()) serialBuf[recBytes++] = NewSerial.read();
        if(recBytes == 1) 
        { 
          minBytes = serialBuf[0]; 
        }
        if(recBytes == minBytes)
        {
          uint8_t settingSize = minBytes-1;
          if(kissProtocolCRC8(serialBuf, 0, settingSize) == serialBuf[minBytes-1])
          {
            if(settingSize > sizeof(settings.s)) settingSize = (uint8_t)sizeof(settings.s);
            settings.ReadSettings(true, settingSize);
            settings.WriteSettings();
          }
        }
      }
      recBytes = 0;
      oneCount = 0;
    }
  }
}
#endif


#ifndef BF32_MODE
boolean ReadTelemetry()
{
  minBytes = 100;
  recBytes = 0;

  while (recBytes < minBytes && micros() - LastLoopTime < 20000)
  {
    const uint8_t STARTCOUNT = 2;
    if (NewSerial.available()) serialBuf[recBytes++] = NewSerial.read();
    if (recBytes == 1 && serialBuf[0] != 5)recBytes = 0; // check for start byte, reset if its wrong
    if (recBytes == 2)
    {
      minBytes = serialBuf[1] + STARTCOUNT + 1; // got the transmission length
      if (minBytes < 150 || minBytes > 180)
      {
        recBytes = 0;
        minBytes = 100;
      }
    }
    if (recBytes == minBytes)
    {
      uint32_t checksum = 0;
      uint8_t i;
      for (i = STARTCOUNT; i < minBytes; i++) {
        checksum += serialBuf[i];
      }
      checksum = (uint32_t)checksum / (minBytes - 3);

      if ((checksum == serialBuf[recBytes - 1] && protoVersion < 109) || (kissProtocolCRC8(serialBuf, STARTCOUNT, minBytes-1) == serialBuf[recBytes - 1] && protoVersion > 108))
      {

        throttle = ((serialBuf[STARTCOUNT] << 8) | serialBuf[1 + STARTCOUNT]) / 10;
        roll = 1000 + ((serialBuf[2 + STARTCOUNT] << 8) | serialBuf[3 + STARTCOUNT]);
        pitch = 1000 + ((serialBuf[4 + STARTCOUNT] << 8) | serialBuf[5 + STARTCOUNT]);
        yaw = 1000 + ((serialBuf[6 + STARTCOUNT] << 8) | serialBuf[7 + STARTCOUNT]);
        LipoVoltage = ((serialBuf[17 + STARTCOUNT] << 8) | serialBuf[18 + STARTCOUNT]);
        #ifdef CROSSHAIR_ANGLE
        angleY = ((serialBuf[33 + STARTCOUNT] << 8) | serialBuf[34 + STARTCOUNT]) / 100;
        #endif

        current_armed = serialBuf[16 + STARTCOUNT];
        if(serialBuf[65 + STARTCOUNT] > 1) current_armed = serialBuf[65 + STARTCOUNT];
        if (current_armed < 0) return false;

        ArmDisarmEvents();

        #ifdef MAH_CORRECTION
        LipoMAH = 0;
        #else
        uint16_t newMAH = ((serialBuf[148 + STARTCOUNT] << 8) | serialBuf[149 + STARTCOUNT]);
        if(newMAH > LipoMAH) LipoMAH = newMAH; 
        #endif
        
        uint8_t voltDev = 0;
        uint32_t temp = 0;
        current = 0;
        tmp32 = 0;
        for (i = 0; i < 4; i++)
        {
          uint8_t i10 = i * 10;
          motorKERPM[i] = ((serialBuf[91 + i10 + STARTCOUNT] << 8) | serialBuf[92 + i10 + STARTCOUNT]) / (MAGNETPOLECOUNT / 2);
          motorCurrent[i] = ((serialBuf[87 + i10 + STARTCOUNT] << 8) | serialBuf[88 + i10 + STARTCOUNT]);
          ESCTemps[i] = ((serialBuf[83 + i10 + STARTCOUNT] << 8) | serialBuf[84 + i10 + STARTCOUNT]);
          ESCmAh[i] = ((serialBuf[89 + i10 + STARTCOUNT] << 8) | serialBuf[90 + i10 + STARTCOUNT]);
          #ifdef MAH_CORRECTION
          temp = (uint32_t)ESCmAh[i] * (uint32_t)settings.s.m_ESCCorrection[i];
          temp /= (uint32_t)100;
          ESCmAh[i] = (uint16_t)temp;
          LipoMAH += ESCmAh[i];
          temp = (uint32_t)motorCurrent[i] * (uint32_t)settings.s.m_ESCCorrection[i];
          temp /= (uint32_t)100;
          motorCurrent[i] = (uint16_t)temp;
          #endif
          current += (uint16_t)motorCurrent[i];
          uint16_t tempVoltage = ((serialBuf[85 + i10 + STARTCOUNT] << 8) | serialBuf[86 + i10 + STARTCOUNT]);
          if (tempVoltage > 5) // the ESC's read the voltage better then the FC
          {
            ESCVoltage[i] = tempVoltage;
            tmp32 += (uint32_t)tempVoltage;
            voltDev++;
          }
          uint8_t i2 = i * 2;
          AuxChanVals[i] = ((serialBuf[8 + i2 + STARTCOUNT] << 8) | serialBuf[9 + i2 + STARTCOUNT]);
        }

        if (voltDev != 0)
        {
          tmp32 = tmp32 / (uint32_t)voltDev;
          LipoVoltage = (uint16_t)tmp32;
        }
        LipoVoltage += settings.s.m_voltCorrect * 10;

        #ifndef KISS_OSD_CONFIG
        failSafeState = serialBuf[41 + STARTCOUNT];
        #endif

        // Data sanity check. Return false if we get invalid data
        for (i = 0; i < 4; i++)
        {
          if (ESCTemps[i] > 160 ||
              motorCurrent[i] > 10000 ||
              motorKERPM[i] > 700 ||
              ESCVoltage[i] > 10000)
          {
            return false;
          }
        }
        if (LipoVoltage > 10000 ||
            throttle > 100 ||
            roll > 2000 ||
            pitch > 2000 ||
            yaw > 2000)
        {
          return false;
        }


        ProcessConversionAndFilters();

        #ifndef KISS_OSD_CONFIG
        if (armedOnce)
        {
          GenerateStats();
        }
        #endif
      }
      else
      {
        return false;
      }
    }
  }
  if (recBytes != minBytes)
  {
    return false;
  }
  return true;
}

uint8_t getCheckSum(uint8_t *buf, uint8_t startIndex, uint8_t stopIndex)
{
  uint32_t checksum = 0;
  uint32_t dataCount = 0;
  uint8_t i;
  for (i = startIndex; i < stopIndex; i++)
  {
    checksum += buf[i];
    dataCount++;
  }
  return (uint8_t)(checksum / dataCount);
}



#ifndef KISS_OSD_CONFIG
extern unsigned long _StartupTime;

void ReadFCSettings(boolean skipValues, uint8_t sMode)
{
  uint8_t i = 0;
  minBytes = 100;
  recBytes = 0;

  while (recBytes < minBytes && micros() - LastLoopTime < 20000)
  {
    uint8_t STARTCOUNT = 2;
    if (NewSerial.available()) serialBuf[recBytes++] = NewSerial.read();
    #ifdef SERIAL_SETTINGS
    SerialSettings();
    #endif
    if (getSettingModes[sMode] == 0x30)
    {
      if (recBytes == 1 && serialBuf[0] != 5) recBytes = 0; // check for start byte, reset if its wrong
      if (recBytes == 2) minBytes = serialBuf[1] + STARTCOUNT + 1; // got the transmission length
    }
    else
    {
      if (recBytes == 1 && serialBuf[0] != getSettingModes[sMode]) recBytes = 0;
      if (recBytes == 2) minBytes = serialBuf[1] + 3; // got the transmission length
    }
    if (recBytes == minBytes)
    {
      if (millis() - _StartupTime < 5000)
      {
        return; //throwing away garbage data sent by FC during startup phase
      }

      uint8_t stopByte = minBytes-1;
      if (getSettingModes[sMode] == 0x30) 
      {
        protoVersion = serialBuf[92 + STARTCOUNT];
      }
      
      uint8_t index = 0;
      uint8_t i;
      
      if ((getCheckSum(serialBuf, STARTCOUNT, stopByte) == serialBuf[recBytes - 1] && protoVersion < 109) || (kissProtocolCRC8(serialBuf, STARTCOUNT, stopByte) == serialBuf[recBytes - 1] && protoVersion > 108))
      {
        if (!skipValues)
        {
          switch (sMode)
          {
            case FC_SETTINGS:
              if (protoVersion >= 104 && serialBuf[73 + STARTCOUNT] > 0)
              {
                armOnYaw = false;
              }
              if(protoVersion < 104)
              {
                if(serialBuf[73+STARTCOUNT] == 1 || serialBuf[73+STARTCOUNT] == 12 || serialBuf[73+STARTCOUNT] == 13
                || serialBuf[73+1+STARTCOUNT] == 1 || serialBuf[73+1+STARTCOUNT] == 12 || serialBuf[73+1+STARTCOUNT] == 13
                || serialBuf[73+2+STARTCOUNT] == 1 || serialBuf[73+2+STARTCOUNT] == 12 || serialBuf[73+2+STARTCOUNT] == 13
                || serialBuf[73+3+STARTCOUNT] == 1 || serialBuf[73+3+STARTCOUNT] == 12 || serialBuf[73+3+STARTCOUNT] == 13)
                {
                   armOnYaw = false;
                }
              }
              if (serialBuf[55 + STARTCOUNT] > 2)
              {
                dShotEnabled = true;
              }
              if (protoVersion < 107) menuDisabled = true;
#ifndef IMPULSERC_VTX
#ifdef VTX_POWER_KNOB
              if (protoVersion > 106 && serialBuf[154 + STARTCOUNT] > 0 && ((serialBuf[154 + STARTCOUNT] & 0x0F) == 0x06))
              {
                vTxPowerKnobChannel = (int8_t)(serialBuf[154 + STARTCOUNT] >> 4) - 1;
              }
#endif
#endif
              break;
            case FC_RATES:
              for (i = 0; i < 3; i++)
              {
                rcrate[i] = ((serialBuf[index + STARTCOUNT + i * 6] << 8) | serialBuf[index + STARTCOUNT + i * 6 + 1]);
                rate[i] = ((serialBuf[index + STARTCOUNT + i * 6 + 2] << 8) | serialBuf[index + STARTCOUNT + i * 6 + 3]);
                rccurve[i] = ((serialBuf[index + STARTCOUNT + i * 6 + 4] << 8) | serialBuf[index + STARTCOUNT + i * 6 + 5]);
              }
              break;
            case FC_PIDS:
              for (i = 0; i < 3; i++)
              {
                pid_p[i] = ((serialBuf[index + STARTCOUNT + i * 6] << 8) | serialBuf[index + STARTCOUNT + i * 6 + 1]);
                pid_i[i] = ((serialBuf[index + STARTCOUNT + i * 6 + 2] << 8) | serialBuf[index + STARTCOUNT + i * 6 + 3]);
                pid_d[i] = ((serialBuf[index + STARTCOUNT + i * 6 + 4] << 8) | serialBuf[index + STARTCOUNT + i * 6 + 5]);
              }
              break;
            case FC_VTX:
              #ifndef IMPULSERC_VTX
              vTxType = serialBuf[index + STARTCOUNT];
              index++;
              vTxChannel = serialBuf[index + STARTCOUNT];
              vTxBand = vTxChannel / 8;
              oldvTxBand = vTxBand;
              vTxChannel %= 8;
              oldvTxChannel = vTxChannel;
              index++;
              vTxLowPower = ((serialBuf[index + STARTCOUNT] << 8) | serialBuf[index + 1 + STARTCOUNT]);
              oldvTxLowPower = vTxLowPower;
              index += 2;
              vTxHighPower = ((serialBuf[index + STARTCOUNT] << 8) | serialBuf[index + 1 + STARTCOUNT]);
              oldvTxHighPower = vTxHighPower;
              #endif
              break;
            case FC_FILTERS:
              memcpy(&fc_filters, &serialBuf[STARTCOUNT], sizeof(fc_filters));
              fc_filters.notchFilterCenterR = (uint16_t)((uint16_t)fc_filters.notchFilterCenterR >> 8) | ((uint16_t)fc_filters.notchFilterCenterR << 8);
              fc_filters.notchFilterCutR = (uint16_t)((uint16_t)fc_filters.notchFilterCutR >> 8) | ((uint16_t)fc_filters.notchFilterCutR << 8);
              fc_filters.notchFilterCenterP = (uint16_t)((uint16_t)fc_filters.notchFilterCenterP >> 8) | ((uint16_t)fc_filters.notchFilterCenterP << 8);
              fc_filters.notchFilterCutP = (uint16_t)((uint16_t)fc_filters.notchFilterCutP >> 8) | ((uint16_t)fc_filters.notchFilterCutP << 8);
              if((stopByte-STARTCOUNT) > 12) moreLPFfilters = true;
              break;
            case FC_TPA:
              memcpy(&fc_tpa, &serialBuf[STARTCOUNT], sizeof(fc_tpa));
              for (i = 0; i < 3; i++)
              {
                fc_tpa.tpa[i] = (uint16_t)((uint16_t)fc_tpa.tpa[i] >> 8) | ((uint16_t)fc_tpa.tpa[i] << 8);
              }
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
  uint8_t i;
  switch (sMode)
  {
    case FC_SETTINGS:
      return;
    case FC_RATES:
      for (i = 0; i < 3; i++)
      {
        serialBuf[STARTCOUNT + i * 6] = (byte)((rcrate[i] & 0xFF00) >> 8);
        serialBuf[STARTCOUNT + i * 6 + 1] = (byte)(rcrate[i] & 0x00FF);
        serialBuf[STARTCOUNT + i * 6 + 2] = (byte)((rate[i] & 0xFF00) >> 8);
        serialBuf[STARTCOUNT + i * 6 + 3] = (byte)(rate[i] & 0x00FF);
        serialBuf[STARTCOUNT + i * 6 + 4] = (byte)((rccurve[i] & 0xFF00) >> 8);
        serialBuf[STARTCOUNT + i * 6 + 5] = (byte)(rccurve[i] & 0x00FF);
      }
      index = 18;
      break;
    case FC_PIDS:
      for (i = 0; i < 3; i++)
      {
        serialBuf[STARTCOUNT + i * 6] = (byte)((pid_p[i] & 0xFF00) >> 8);
        serialBuf[STARTCOUNT + i * 6 + 1] = (byte)(pid_p[i] & 0x00FF);
        serialBuf[STARTCOUNT + i * 6 + 2] = (byte)((pid_i[i] & 0xFF00) >> 8);
        serialBuf[STARTCOUNT + i * 6 + 3] = (byte)(pid_i[i] & 0x00FF);
        serialBuf[STARTCOUNT + i * 6 + 4] = (byte)((pid_d[i] & 0xFF00) >> 8);
        serialBuf[STARTCOUNT + i * 6 + 5] = (byte)(pid_d[i] & 0x00FF);
      }
      index = 18;
      break;
    case FC_VTX:
      #ifndef IMPULSERC_VTX
      serialBuf[STARTCOUNT + index++] = (byte) vTxType;
      serialBuf[STARTCOUNT + index++] = (byte)((vTxBand * 8) + vTxChannel);
      serialBuf[STARTCOUNT + index++] = (byte)((vTxLowPower & 0xFF00) >> 8);
      serialBuf[STARTCOUNT + index++] = (byte)(vTxLowPower & 0x00FF);
      serialBuf[STARTCOUNT + index++] = (byte)((vTxHighPower & 0xFF00) >> 8);
      serialBuf[STARTCOUNT + index++] = (byte)(vTxHighPower & 0x00FF);
      #else
      return;
      #endif
      break;
    case FC_FILTERS:      
      serialBuf[STARTCOUNT + index++] = fc_filters.lpf_frq;
      serialBuf[STARTCOUNT + index++] = (byte) fc_filters.yawFilterCut;
      serialBuf[STARTCOUNT + index++] = (byte) fc_filters.notchFilterEnabledR;
      serialBuf[STARTCOUNT + index++] = (byte)((fc_filters.notchFilterCenterR & 0xFF00) >> 8);
      serialBuf[STARTCOUNT + index++] = (byte)(fc_filters.notchFilterCenterR & 0x00FF);
      serialBuf[STARTCOUNT + index++] = (byte)((fc_filters.notchFilterCutR & 0xFF00) >> 8);
      serialBuf[STARTCOUNT + index++] = (byte)(fc_filters.notchFilterCutR & 0x00FF);
      serialBuf[STARTCOUNT + index++] = (byte) fc_filters.notchFilterEnabledP;
      serialBuf[STARTCOUNT + index++] = (byte)((fc_filters.notchFilterCenterP & 0xFF00) >> 8);
      serialBuf[STARTCOUNT + index++] = (byte)(fc_filters.notchFilterCenterP & 0x00FF);
      serialBuf[STARTCOUNT + index++] = (byte)((fc_filters.notchFilterCutP & 0xFF00) >> 8);
      serialBuf[STARTCOUNT + index++] = (byte)(fc_filters.notchFilterCutP & 0x00FF);
      serialBuf[STARTCOUNT + index++] = fc_filters.yawLpF;
      serialBuf[STARTCOUNT + index++] = fc_filters.DLpF;
      break;
    case FC_TPA:
      for (i = 0; i < 3; i++)
      {
        serialBuf[STARTCOUNT + i * 2] = (byte)((fc_tpa.tpa[i] & 0xFF00) >> 8);
        serialBuf[STARTCOUNT + i * 2 + 1] = (byte)(fc_tpa.tpa[i] & 0x00FF);
      }
      index = 6;
      serialBuf[STARTCOUNT + index++] = (byte)fc_tpa.customTPAEnabled;
      serialBuf[STARTCOUNT + index++] = (byte)fc_tpa.ctpa_bp1;
      serialBuf[STARTCOUNT + index++] = (byte)fc_tpa.ctpa_bp2;
      for (i = 0; i < 4; i++)
      {
        serialBuf[STARTCOUNT + i + index] = (byte)fc_tpa.ctpa_infl[i];
      }
      index += 4;
      break;
  }

  serialBuf[0] = index;
  if(protoVersion > 108) serialBuf[STARTCOUNT + index] = kissProtocolCRC8(serialBuf, STARTCOUNT, STARTCOUNT + index);
  else serialBuf[STARTCOUNT + index] = getCheckSum(serialBuf, STARTCOUNT, STARTCOUNT + index);

  NewSerial.write(setSettingModes[sMode]); //Set settings
  for (i = 0; i < (STARTCOUNT + index + 1); i++)
  {
    NewSerial.write(serialBuf[i]);
  }
}
#endif


#else

#define MSP_API_VERSION           1    //out message
#define MSP_MODE_RANGES          34    //out message         Returns all mode ranges
#define MSP_VTX_CONFIG           88    //out message         Get vtx settings - betaflight
#define MSP_SET_VTX_CONFIG       89    //in message          Set vtx settings - betaflight
#define MSP_FILTER_CONFIG        92
#define MSP_SET_FILTER_CONFIG    93
#define MSP_PID_ADVANCED         94
#define MSP_SET_PID_ADVANCED     95

#define MSP_IDENT                100   //out message         multitype + multiwii version + protocol version + capability variable
#define MSP_STATUS               101   //out message         cycletime & errors_count & sensor present & box activation & current setting number
#define MSP_RAW_IMU              102   //out message         9 DOF
#define MSP_SERVO                103   //out message         8 servos
#define MSP_MOTOR                104   //out message         8 motors
#define MSP_RC                   105   //out message         8 rc chan and more
#define MSP_RAW_GPS              106   //out message         fix, numsat, lat, lon, alt, speed, ground course
#define MSP_COMP_GPS             107   //out message         distance home, direction home
#define MSP_ATTITUDE             108   //out message         2 angles 1 heading
#define MSP_ALTITUDE             109   //out message         altitude, variometer
#define MSP_ANALOG               110   //out message         vbat, powermetersum, rssi if available on RX
#define MSP_RC_TUNING            111   //out message         rc rate, rc expo, rollpitch rate, yaw rate, dyn throttle PID
#define MSP_PID                  112   //out message         P I D coeff (9 are used currently)
#define MSP_BOX                  113   //out message         BOX setup (number is dependant of your setup)
#define MSP_MISC                 114   //out message         powermeter trig
#define MSP_MOTOR_PINS           115   //out message         which pins are in use for motors & servos, for GUI 
#define MSP_BOXNAMES             116   //out message         the aux switch names
#define MSP_PIDNAMES             117   //out message         the PID names
#define MSP_BOXIDS               119   //out message         get the permanent IDs associated to BOXes
#define MSP_NAV_STATUS           121   //out message         Returns navigation status
#define MSP_VOLTAGE_METERS       128   //out message         Voltage (per meter)
#define MSP_CURRENT_METERS       129   //out message         Amperage (per meter)

#define MSP_CELLS                130   //out message         FrSky SPort Telemtry

#define MSP_EXTRA_ESC_DATA       134    //out message         Extra ESC data from 32-Bit ESCs (Temperature, RPM)

#define MSP_STATUS_EX            150    //out message         cycletime, errors_count, CPU load, sensor present etc

#define MSP_SET_RAW_RC           200   //in message          8 rc chan
#define MSP_SET_RAW_GPS          201   //in message          fix, numsat, lat, lon, alt, speed
#define MSP_SET_PID              202   //in message          P I D coeff (9 are used currently)
#define MSP_SET_BOX              203   //in message          BOX setup (number is dependant of your setup)
#define MSP_SET_RC_TUNING        204   //in message          rc rate, rc expo, rollpitch rate, yaw rate, dyn throttle PID
#define MSP_ACC_CALIBRATION      205   //in message          no param
#define MSP_MAG_CALIBRATION      206   //in message          no param
#define MSP_SET_MISC             207   //in message          powermeter trig + 8 free for future use
#define MSP_RESET_CONF           208   //in message          no param
#define MSP_SET_WP               209   //in message          sets a given WP (WP#,lat, lon, alt, flags)
#define MSP_SELECT_SETTING       210   //in message          Select Setting Number (0-2)
#define MSP_SET_HEAD             211   //in message          define a new heading hold direction

#define MSP_BIND                 240   //in message          no param

#define MSP_ALARMS               242   //in message          poll for alert text

#define MSP_EEPROM_WRITE         250   //in message          no param

#define MSP_DEBUGMSG             253   //out message         debug string buffer
#define MSP_DEBUG                254   //out message         debug1,debug2,debug3,debug4

// Cleanflight/Betaflight specific
#define MSP_PID_CONTROLLER       59    //in message          no param
#define MSP_SET_PID_CONTROLLER   60    //out message         sets a given pid controller

// Cleanflight specific
#define MSP_LOOP_TIME            73    //out message         Returns FC cycle time i.e looptime 
#define MSP_SET_LOOP_TIME        74    //in message          Sets FC cycle time i.e looptime parameter

// Baseflight specific
#define MSP_CONFIG               66    //out message         baseflight-specific settings that aren't covered elsewhere
#define MSP_SET_CONFIG           67    //in message          baseflight-specific settings save

#define MSP_CAMERA_CONTROL       98

static uint32_t armBox = 0, failSafeBox = 0;
#ifdef ARMING_STATUS
static uint32_t dShotReverseBox = 0;
#endif

static const char mspHeader[] = { '$', 'M', '>' };

void mspRequest(uint8_t mspCommand)
{
  for(uint8_t i=0; i<2; i++) NewSerial.write(mspHeader[i]);
  NewSerial.write('<');
  uint8_t txChecksum = 0;
  NewSerial.write((uint8_t)0);
  NewSerial.write(mspCommand);
  txChecksum ^= mspCommand;
  NewSerial.write(txChecksum);
}

#ifndef KISS_OSD_CONFIG
extern void ReadFCSettings(boolean skipValues, uint8_t sMode, boolean notReceived = true);
#endif

boolean ReadTelemetry()
{
  minBytes = 100;
  recBytes = 0;
  uint8_t mspCmd;

  while (recBytes < minBytes && micros() - LastLoopTime < 20000)
  {
    const uint8_t STARTCOUNT = 5;
    if (NewSerial.available()) serialBuf[recBytes++] = NewSerial.read();
    if (recBytes == 1 && serialBuf[0] != mspHeader[0]) recBytes = 0; // check for MSP header, reset if its wrong
    if (recBytes == 2 && serialBuf[1] != mspHeader[1]) recBytes = 0; // check for MSP header, reset if its wrong
    if (recBytes == 3 && serialBuf[2] != mspHeader[2]) recBytes = 0; // check for MSP header, reset if its wrong
    if (recBytes == 4) minBytes = serialBuf[3] + STARTCOUNT + 1; // got the transmission length
    if (recBytes == 5) mspCmd = serialBuf[4]; // MSP command
    if (recBytes == minBytes)
    {
      uint8_t checksum = serialBuf[3];
      uint8_t i;
      for (i = STARTCOUNT-1; i < minBytes; i++) {
        checksum ^= serialBuf[i];
      }

      if (checksum == 0)
      {
        uint32_t temp = 0;
        uint8_t kissMotorPos = 0;
        switch(mspCmd)
        {
          case MSP_RC:
            roll = (constrain(((serialBuf[1 + STARTCOUNT] << 8) | serialBuf[STARTCOUNT]), 1000, 2000) - 1000)*2;
            pitch = (constrain(((serialBuf[3 + STARTCOUNT] << 8) | serialBuf[2 + STARTCOUNT]), 1000, 2000) - 1000)*2;
            yaw = (constrain(((serialBuf[5 + STARTCOUNT] << 8) | serialBuf[4 + STARTCOUNT]), 1000, 2000) - 1000)*2;            
            throttle = (constrain(((serialBuf[7 + STARTCOUNT] << 8) | serialBuf[6 + STARTCOUNT]), 1000, 2000) - 1000) / 10;
            for(i=0; i<4; i++) AuxChanVals[i] = (constrain(((serialBuf[9 + i*2 + STARTCOUNT] << 8) | serialBuf[8 + i*2 + STARTCOUNT]), 1000, 2000) - 1000)*2-1000;
          break;
          case MSP_ANALOG:
            LipoVoltage = serialBuf[STARTCOUNT]*10; //8-bit WTF???
            LipoMAH = ((serialBuf[2 + STARTCOUNT] << 8) | serialBuf[1 + STARTCOUNT]);
            rssiVal = ((serialBuf[4 + STARTCOUNT] << 8) | serialBuf[3 + STARTCOUNT]);
            current = ((serialBuf[6 + STARTCOUNT] << 8) | serialBuf[5 + STARTCOUNT]);
          break;
          case MSP_BOXIDS:
            armBox = 0;
            failSafeBox = 0;
            #ifdef ARMING_STATUS
            dShotReverseBox = 0;
            #endif
            temp = 1;
            for(i=0; (i + STARTCOUNT)<(minBytes-1); i++)
            {
              if(serialBuf[i + STARTCOUNT] == 0) armBox |= temp;
              if(serialBuf[i + STARTCOUNT] == 27) failSafeBox |= temp;
              #ifdef ARMING_STATUS
              if(serialBuf[i + STARTCOUNT] == 35) dShotReverseBox |= temp;
              #endif
              temp <<= 1;
            }
          break;
          case MSP_STATUS_EX:            
            temp = ((serialBuf[9 + STARTCOUNT] << 24) | (serialBuf[8 + STARTCOUNT] << 16) | (serialBuf[7 + STARTCOUNT] << 8) | serialBuf[6 + STARTCOUNT]);
            current_armed = (temp & armBox) != 0;
            #ifdef ARMING_STATUS
            if(temp & dShotReverseBox) current_armed = 3;
            #endif
            #ifndef KISS_OSD_CONFIG
            if(pidProfileChanged)
            {
              if(pidProfile == serialBuf[10 + STARTCOUNT])
              {
                mspRequest(MSP_PID);
                mspRequest(MSP_PID_ADVANCED);
                pidProfileChanged = false;
              }
            }
            else pidProfile = serialBuf[10 + STARTCOUNT];
            if(rateProfileChanged)
            {
              if(rateProfile == serialBuf[14 + STARTCOUNT])
              {
                mspRequest(MSP_RC_TUNING);
                rateProfileChanged = false;
              }
            }
            else rateProfile = serialBuf[14 + STARTCOUNT];
            failSafeState = 0;
            if(temp & failSafeBox) failSafeState = 10;
            #endif            
            ArmDisarmEvents();
          break;
          case MSP_VOLTAGE_METERS:
            if(minBytes > 9)
            {
              uint16_t voltSum = 0;
              uint8_t ESCfound = 0;
              uint8_t index = STARTCOUNT;
              while((index+1) < minBytes)
              {
                if(serialBuf[index] > 59 && serialBuf[index] <64)
                {
                  kissMotorPos = ((serialBuf[index]-60)+2)%4;
                  ESCVoltage[kissMotorPos] = serialBuf[index+1]*10;
                  if(ESCVoltage[kissMotorPos] > 5)
                  {
                    voltSum += ESCVoltage[kissMotorPos];              
                    ESCfound++;
                  }                  
                }
                index += 2;
              }
              if(ESCfound > 0) LipoVoltage = voltSum / (uint16_t)ESCfound;
            }
          break;
          case MSP_CURRENT_METERS:
            if(minBytes > 14)
            {
              #ifdef MAH_CORRECTION
              uint16_t oldcurrent = current;
              uint16_t oldLipoMah = LipoMAH;
              current = 0;
              LipoMAH = 0;
              #endif
              uint8_t index = STARTCOUNT;
              while((index+4) < minBytes)
              {
                if(serialBuf[index] > 59 && serialBuf[index] <64)
                {
                  uint8_t bfMotorPos = serialBuf[index]-60;
                  kissMotorPos = (bfMotorPos+2)%4;
                  ESCmAh[kissMotorPos] = ((serialBuf[index+2] << 8) | serialBuf[index+1]);
                  motorCurrent[kissMotorPos] = constrain(((serialBuf[index+4] << 8) | serialBuf[index+3]) / 10, 0, 10000);
                  #ifdef MAH_CORRECTION
                  temp = (uint32_t)motorCurrent[kissMotorPos] * (uint32_t)settings.s.m_ESCCorrection[bfMotorPos];
                  temp /= (uint32_t)100;
                  motorCurrent[kissMotorPos] = (uint16_t)temp;
                  temp = (uint32_t)ESCmAh[kissMotorPos] * (uint32_t)settings.s.m_ESCCorrection[bfMotorPos];
                  temp /= (uint32_t)100;
                  ESCmAh[kissMotorPos] = (uint16_t)temp;
                  LipoMAH += ESCmAh[kissMotorPos];
                  current += motorCurrent[kissMotorPos]; 
                  #endif
                }
                index += 5;
              }
              #ifdef MAH_CORRECTION
              if(current == 0) current = oldcurrent;
              if(LipoMAH == 0) LipoMAH = oldLipoMah;
              #endif
            }
          break;
          //MUST BE LAST AT ALL TIMES!!!
          case MSP_EXTRA_ESC_DATA:
            for(i=0; (STARTCOUNT+i*3+2)<minBytes && i<4; i++)
            {
              kissMotorPos = (i+2)%4;
              ESCTemps[kissMotorPos] = serialBuf[STARTCOUNT+1+i*3];
              motorKERPM[kissMotorPos] = ((serialBuf[STARTCOUNT+1+i*3+2] << 8) | serialBuf[STARTCOUNT+1+i*3+1])/ (MAGNETPOLECOUNT/2);
            }
          break;
          #ifndef KISS_OSD_CONFIG
          case MSP_PID:
            ReadFCSettings(false,MSP_PID,false);
          break;
          case MSP_RC_TUNING:
            ReadFCSettings(false,MSP_RC_TUNING,false);
          break;
          case MSP_PID_ADVANCED:
            ReadFCSettings(false,MSP_PID_ADVANCED,false);
          break;
          #endif
          default:
          return true;
        }
        
        #ifdef CROSSHAIR_ANGLE
        angleY = 0; //FIXME
        #endif

        LipoVoltage += settings.s.m_voltCorrect * 10;

        if(telemetryMSP == MAX_TELEMETRY_MSPS-1) 
        {
      
          ProcessConversionAndFilters();
  
          #ifndef KISS_OSD_CONFIG
          if (armedOnce)
          {
            GenerateStats();
          }
          #endif
        }
      }
      else
      {
        return false;
      }
    }
  }
  if (recBytes != minBytes)
  {
    return false;
  }
  return true;
}

#ifndef KISS_OSD_CONFIG
void ReadFCSettings(boolean skipValues, uint8_t sMode, boolean notReceived = true)
{ 
  if(notReceived)
  {
    recBytes = 0;
    minBytes = 100;
  }
  uint8_t mspCmd;
  if(!notReceived) mspCmd = sMode;

  while((recBytes < minBytes && micros() - LastLoopTime < 20000) || !notReceived)
  {
    const uint8_t STARTCOUNT = 5;
    if(notReceived)
    {
      if (NewSerial.available()) serialBuf[recBytes++] = NewSerial.read();
      #ifdef SERIAL_SETTINGS
      SerialSettings();
      #endif
      if (recBytes == 1 && serialBuf[0] != mspHeader[0]) recBytes = 0; // check for MSP header, reset if its wrong
      if (recBytes == 2 && serialBuf[1] != mspHeader[1]) recBytes = 0; // check for MSP header, reset if its wrong
      if (recBytes == 3 && serialBuf[2] != mspHeader[2]) recBytes = 0; // check for MSP header, reset if its wrong
      if (recBytes == 4) minBytes = serialBuf[3] + STARTCOUNT + 1; // got the transmission length
      if (recBytes == 5) mspCmd = serialBuf[4]; // MSP command
    }

    if (recBytes == minBytes)
    {
      uint8_t checksum = serialBuf[3];
      uint8_t i;
      for (i = STARTCOUNT-1; i < minBytes; i++) {
        checksum ^= serialBuf[i];
      }

      if (checksum == 0)
      {
        fcSettingsReceived = true;
        if(!skipValues)
        {
          uint8_t copyLength;
          switch(mspCmd)
          {
            case MSP_API_VERSION:
              protoVersion = serialBuf[STARTCOUNT + 2];
              if(protoVersion < 36) menuDisabled = true;
            break;
            case MSP_PID:
              if(!rateProfileChanged)
              {
                for(i=0; i<10; i++)
                {
                  pid_p[i] = serialBuf[STARTCOUNT + i * 3];
                  pid_i[i] = serialBuf[STARTCOUNT + i * 3 + 1];
                  pid_d[i] = serialBuf[STARTCOUNT + i * 3 + 2];
                }
              }
            break;
            case MSP_RC_TUNING:
              if(!pidProfileChanged) memcpy(&bf32_rates, &serialBuf[STARTCOUNT], sizeof(bf32_rates));
            break;
            case MSP_FILTER_CONFIG:
              memcpy(&bf32_filters, &serialBuf[STARTCOUNT], sizeof(bf32_filters));
            break;
            case MSP_VTX_CONFIG:
              #ifndef IMPULSERC_VTX
              vTxType = serialBuf[STARTCOUNT];
              vTxBand = serialBuf[STARTCOUNT + 1];
              vTxChannel = serialBuf[STARTCOUNT + 2];
              oldvTxBand = vTxBand;
              oldvTxChannel = vTxChannel;
              vTx_powerIDX = serialBuf[STARTCOUNT + 3];
              oldvTx_powerIDX = vTx_powerIDX;
              vTx_pitmode = serialBuf[STARTCOUNT + 4];
              #endif
            break;
            case MSP_MODE_RANGES:
              for(i=STARTCOUNT; (i+4)<minBytes; i+=4)
              {
                if(serialBuf[i] == 0) armOnYaw = false;
              }
            break;
            case MSP_PID_ADVANCED:
              if(serialBuf[3] > 55)
              {
                fcSettingsReceived = false;
                return;
              }
              serialBuf[200] = serialBuf[3];
              memcpy(&serialBuf[201], &serialBuf[STARTCOUNT], serialBuf[3]);
              setpointRelaxRatio = serialBuf[STARTCOUNT + 8];
              dtermSetpointWeight = serialBuf[STARTCOUNT + 9];
            break;
          }
        }
      }
    }
    if(!notReceived) return;
  }
}

#ifdef CAMERA_CONTROL
typedef enum {
    CAMERA_CONTROL_KEY_ENTER,
    CAMERA_CONTROL_KEY_LEFT,
    CAMERA_CONTROL_KEY_UP,
    CAMERA_CONTROL_KEY_RIGHT,
    CAMERA_CONTROL_KEY_DOWN,
    CAMERA_CONTROL_KEYS_COUNT
} cameraControlKey_e;
#endif

void SendFCSettings(uint8_t mspCmd)
{
  uint8_t checksum;
  uint8_t transLength;
  uint8_t i;
  
  switch(mspCmd)
  {
    case MSP_SET_PID:
      transLength = 30;
      for(i=0; i<10; i++)
      {
        serialBuf[i * 3] = pid_p[i];
        serialBuf[i * 3 + 1] = pid_i[i];
        serialBuf[i * 3 + 2] = pid_d[i];
      }
    break;
    case MSP_SET_RC_TUNING:
      transLength = sizeof(bf32_rates);
      memcpy(&serialBuf[0], &bf32_rates, sizeof(bf32_rates));
    break;
    case MSP_SET_FILTER_CONFIG:
      transLength = sizeof(bf32_filters);
      memcpy(&serialBuf[0], &bf32_filters, sizeof(bf32_filters));
    break;
    case MSP_SET_VTX_CONFIG:
      #ifndef IMPULSERC_VTX
      transLength = 5;
      serialBuf[0] = vTxType;
      serialBuf[1] = vTxBand;
      serialBuf[2] = vTxChannel;
      serialBuf[3] = vTx_powerIDX;
      serialBuf[4] = vTx_pitmode;
      #else
      return;
      #endif
    break;
    case MSP_SELECT_SETTING:
      transLength = 1;
      if(pidProfileChanged) serialBuf[0] = pidProfile;
      else if(rateProfileChanged)
           {
             serialBuf[0] = 0;
             serialBuf[0] |= (1 << 7) | rateProfile;
           }
    break;
    case MSP_SET_PID_ADVANCED:
      transLength = serialBuf[200];
      memcpy(&serialBuf[0], &serialBuf[201], transLength);
      serialBuf[8] = setpointRelaxRatio;
      serialBuf[9] = dtermSetpointWeight;
    break;
    #ifdef CAMERA_CONTROL
    case MSP_CAMERA_CONTROL:
      transLength = 1;
      serialBuf[0] = 100;
      if(code & inputChecker.PITCH_UP) serialBuf[0] = CAMERA_CONTROL_KEY_UP;
      if(code & inputChecker.PITCH_DOWN) serialBuf[0] = CAMERA_CONTROL_KEY_DOWN;
      if(code & inputChecker.ROLL_LEFT) serialBuf[0] = CAMERA_CONTROL_KEY_LEFT;
      if(code & inputChecker.ROLL_RIGHT) serialBuf[0] = CAMERA_CONTROL_KEY_RIGHT;
      if(code & inputChecker.YAW_RIGHT) serialBuf[0] = CAMERA_CONTROL_KEY_ENTER;
      if(serialBuf[0] == 100) return;
    break;
    #endif
    default:
    return;
  }
  for(i=0; i<2; i++) NewSerial.write(mspHeader[i]);
  NewSerial.write('<');
  NewSerial.write(transLength);
  checksum = transLength;
  NewSerial.write(mspCmd);
  checksum ^= mspCmd;
  for(i=0; i<transLength; i++)
  {
    NewSerial.write(serialBuf[i]);
    checksum ^= serialBuf[i];
  }
  NewSerial.write(checksum);
}
#endif
#endif
