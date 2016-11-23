#include "CStickInput.h"

CStickInput::CStickInput():
startYawTime(0),
startRollTime(0),
startPitchTime(0),
startYawLongTime(0)
{
  rollDelay = ROLL_PITCH_DELAY;
  pitchDelay = ROLL_PITCH_DELAY;
  yawDelay = ROLL_PITCH_DELAY;
  yawLongDelay = YAW_LONG_DELAY;
  rollCount = 0;
  pitchCount = 0;
  yawCount = 0;
}

uint8_t CStickInput::ProcessStickInputs(int16_t roll, int16_t pitch, int16_t yaw, int16_t armed)
{
  uint8_t code = 0;
  if(armed == 0)
  {
    code |= CheckInput(roll, &startRollTime, &rollDelay, &rollCount, ROLL_LEFT, ROLL_RIGHT);
    if(startRollTime == 0)
    {
      rollDelay = ROLL_PITCH_DELAY;
    }
    code |= CheckInput(pitch, &startPitchTime, &pitchDelay, &pitchCount, PITCH_DOWN, PITCH_UP);
    if(startPitchTime == 0)
    {
      pitchDelay = ROLL_PITCH_DELAY;
    }
    code |= CheckInput(yaw, &startYawTime, &yawDelay, &yawCount, YAW_LEFT, YAW_RIGHT);
    if(startYawTime == 0)
    {
      yawDelay = ROLL_PITCH_DELAY;
    }
    uint16_t temp = 0;
    code |= CheckInput(yaw, &startYawLongTime, &yawLongDelay, &temp, YAW_LONG_LEFT, YAW_LONG_RIGHT, true);
    if(startYawLongTime == 0)
    {
      yawLongDelay = YAW_LONG_DELAY;
    }    
  }
  return code;
}

uint8_t CStickInput::CheckInput(int16_t input, unsigned long *startTime, uint16_t *timedelay, uint16_t *count, uint8_t smallMask, uint8_t bigMask, boolean skip)
{
  uint8_t code = 0;
  if(input > 1750 || input < 250)
  {
    if(*startTime == 0)
    {
      *startTime = millis();
      if(!skip)
      {
        if(input > 1750)
        {
          code |= bigMask;
        }
        else
        {
          code |= smallMask;
        }
      }
    }
    else
    {
      if((millis() - *startTime) > *timedelay)
      {
        (*count)++;
        if(*timedelay > MAXSPEED && (*count) > 0)
        {
          *timedelay -= SPEEDUP;
          (*count) = 0;
        }
        if(input > 1750)
        {
          code |= bigMask;
        }
        else
        {
          code |= smallMask;
        }
        *startTime = millis();
      }
    }
  }
  else
  {
    *startTime = 0;
    *count = 0;
  }
  return code;  
}
