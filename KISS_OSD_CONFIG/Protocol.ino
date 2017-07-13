static uint8_t minBytes = 0;
static uint8_t minBytesSettings = 0;
static uint8_t recBytes = 0;
#ifndef KISS_OSD_CONFIG
static uint8_t serialBuf[256];
#else
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


        int8_t current_armed = serialBuf[16 + STARTCOUNT];
        if (current_armed < 0) return false;

        // switch disarmed => armed
        if (armed == 0 && current_armed > 0)
        {
          start_time = millis();
          triggerCleanScreen = true;
          armedOnce = true;
          last_Aux_Val = AuxChanVals[settings.m_DVchannel];
          DV_change_time = 0;
          #ifndef KISS_OSD_CONFIG
          statsActive = false;
          #endif
        }
        // switch armed => disarmed
        else
        {
          if (armed > 0 && current_armed == 0)
          {
            if(start_time > 0) total_time = total_time + (millis() - start_time);
            start_time = 0;
            triggerCleanScreen = true;
            if (settings.m_batWarning > 0)
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
            if (throttle < 5 && start_time == 0)
            {
              time = 0;
            }
            else
            {
              if (start_time == 0) start_time = millis();
              time = millis() - start_time;
            }
          }
        }
        armed = current_armed;

        LipoMAH =       ((serialBuf[148 + STARTCOUNT] << 8) | serialBuf[149 + STARTCOUNT]);

        uint32_t tmp32 = 0;
        uint8_t voltDev = 0;
        for (i = 0; i < 4; i++)
        {
          uint8_t i10 = i * 10;
          motorKERPM[i] = ((serialBuf[91 + i10 + STARTCOUNT] << 8) | serialBuf[92 + i10 + STARTCOUNT]) / (MAGNETPOLECOUNT / 2);
          motorCurrent[i] = ((serialBuf[87 + i10 + STARTCOUNT] << 8) | serialBuf[88 + i10 + STARTCOUNT]);
          ESCTemps[i] = ((serialBuf[83 + i10 + STARTCOUNT] << 8) | serialBuf[84 + i10 + STARTCOUNT]);
          ESCmAh[i] = ((serialBuf[89 + i10 + STARTCOUNT] << 8) | serialBuf[90 + i10 + STARTCOUNT]);
          int16_t tempVoltage = ((serialBuf[85 + i10 + STARTCOUNT] << 8) | serialBuf[86 + i10 + STARTCOUNT]);
          if (tempVoltage > 5) // the ESC's read the voltage better then the FC
          {
            ESCVoltage[i] = tempVoltage;
            tmp32 += tempVoltage;
            voltDev++;
          }
          uint8_t i2 = i * 2;
          AuxChanVals[i] = ((serialBuf[8 + i2 + STARTCOUNT] << 8) | serialBuf[9 + i2 + STARTCOUNT]);
        }

        if (voltDev != 0)
        {
          tmp32 = tmp32 / (uint32_t)voltDev;
          LipoVoltage = (int16_t)tmp32;
        }
        LipoVoltage += settings.m_voltCorrect * 10;

        current = (uint16_t)(motorCurrent[0] + motorCurrent[1] + motorCurrent[2] + motorCurrent[3]);

        #ifndef KISS_OSD_CONFIG
        failSafeState = serialBuf[41 + STARTCOUNT];
        #endif

        // Data sanity check. Return false if we get invalid data
        for (i = 0; i < 4; i++)
        {
          if (ESCTemps[i] < -50 || ESCTemps[i] > 130 ||
              motorCurrent[i] < 0 || motorCurrent[i] > 10000 ||
              motorKERPM[i] < 0 || motorKERPM[i] > 500 ||
              ESCVoltage[i] < 0 || ESCVoltage[i] > 10000 ||
              ESCmAh[i] < 0)
          {
            return false;
          }
        }
        if (LipoVoltage < 0 || LipoVoltage > 10000 ||
            throttle < 0 || throttle > 100 ||
            roll < 0 || roll > 2000 ||
            pitch < 0 || pitch > 2000 ||
            yaw < 0 || yaw > 2000 ||
            LipoMAH < 0)
        {
          return false;
        }


        if (settings.m_tempUnit == 1)
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
        if (lastTempUnit == 1 && settings.m_tempUnit == 0)
        {
          for (i = 0; i < 4; i++)
          {
            maxTemps[i] = (maxTemps[i] - 32) * 5 / 9;
          }
          MaxTemp = (MaxTemp - 32) * 5 / 9;
        }
        lastTempUnit = settings.m_tempUnit;

        
        LipoVoltage = voltageFilter.ProcessValue(LipoVoltage);
        current = ampTotalFilter.ProcessValue(current);
        for (i = 0; i < 4; i++)
        {
          motorKERPM[i] = ESCKRfilter[i].ProcessValue(motorKERPM[i]);
          motorCurrent[i] = ESCAmpfilter[i].ProcessValue(motorCurrent[i]);
        }

        #ifndef KISS_OSD_CONFIG
        if (armedOnce)
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
          tmp32 = (uint32_t)MaxAmps * 10 / (uint32_t)settings.m_batMAH[settings.m_activeBattery];
          MaxC = (int16_t) tmp32;
          tmp32 = (uint32_t)LipoVoltage * (uint32_t)current;
          MaxWatt = findMax(MaxWatt, (uint16_t) (tmp32 / 1000));
          if (MaxWatt > settings.m_maxWatts) settings.m_maxWatts = MaxWatt;
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
    while (NewSerial.available()) serialBuf[recBytes++] = NewSerial.read();
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
              if (serialBuf[73 + STARTCOUNT] > 0)
              {
                armOnYaw = false;
              }
              if (serialBuf[55 + STARTCOUNT] > 2)
              {
                dShotEnabled = true;
              }
#ifndef IMPULSERC_VTX
              if (protoVersion > 106 && serialBuf[154 + STARTCOUNT] > 0 && ((serialBuf[154 + STARTCOUNT] & 0x0F) == 0x06))
              {
                vTxPowerKnobChannel = (int8_t)(serialBuf[154 + STARTCOUNT] >> 4) - 1;
              }
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
              break;
            case FC_FILTERS:
              memcpy(&fc_filters, &serialBuf[STARTCOUNT], sizeof(fc_filters));
              fc_filters.notchFilterCenterR = (uint16_t)((uint16_t)fc_filters.notchFilterCenterR >> 8) | ((uint16_t)fc_filters.notchFilterCenterR << 8);
              fc_filters.notchFilterCutR = (uint16_t)((uint16_t)fc_filters.notchFilterCutR >> 8) | ((uint16_t)fc_filters.notchFilterCutR << 8);
              fc_filters.notchFilterCenterP = (uint16_t)((uint16_t)fc_filters.notchFilterCenterP >> 8) | ((uint16_t)fc_filters.notchFilterCenterP << 8);
              fc_filters.notchFilterCutP = (uint16_t)((uint16_t)fc_filters.notchFilterCutP >> 8) | ((uint16_t)fc_filters.notchFilterCutP << 8);
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
      serialBuf[STARTCOUNT + index++] = (byte) vTxType;
      serialBuf[STARTCOUNT + index++] = (byte)((vTxBand * 8) + vTxChannel);
      serialBuf[STARTCOUNT + index++] = (byte)((vTxLowPower & 0xFF00) >> 8);
      serialBuf[STARTCOUNT + index++] = (byte)(vTxLowPower & 0x00FF);
      serialBuf[STARTCOUNT + index++] = (byte)((vTxHighPower & 0xFF00) >> 8);
      serialBuf[STARTCOUNT + index++] = (byte)(vTxHighPower & 0x00FF);
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
