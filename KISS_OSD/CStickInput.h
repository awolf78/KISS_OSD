#ifndef CStickInputh
#define CStickInputh
#include <Arduino.h>

class CStickInput
{
public:
  CStickInput();
  uint8_t ProcessStickInputs(int16_t roll, int16_t pitch, int16_t yaw, int16_t armed);
  static const uint8_t YAW_LEFT = 0x01; // hex for 0000 0001
  static const uint8_t YAW_RIGHT = 0x02; // hex for 0000 0010
  static const uint8_t ROLL_LEFT = 0x04; // hex for 0000 0100
  static const uint8_t ROLL_RIGHT = 0x08; // hex for 0000 1000
  static const uint8_t PITCH_UP = 0x10; // hex for 0001 0000
  static const uint8_t PITCH_DOWN = 0x20; // hex for 0010 0000
  static const uint8_t YAW_LONG_LEFT = 0x40; // hex for 0100 0000
  static const uint8_t YAW_LONG_RIGHT = 0x80; // hex for 1000 0000
  
  
private:
  static const uint16_t YAW_LONG_DELAY = 2000;
  static const uint16_t ROLL_PITCH_DELAY = 1250;
  static const uint16_t SPEEDUP = 1000;
  static const uint16_t MAXSPEED = 250;
  uint16_t rollDelay, pitchDelay, yawDelay, yawLongDelay;
  uint16_t rollCount, pitchCount, yawCount;
  unsigned long startYawTime, startRollTime, startPitchTime, startYawLongTime;
  
  uint8_t CheckInput(int16_t input, unsigned long *startTime, uint16_t *timedelay, uint16_t *count, uint8_t smallMask, uint8_t bigMask, boolean skip = false);
  uint8_t CheckInputInternal(int16_t input, uint8_t smallMask, uint8_t bigMask);
};

#endif
