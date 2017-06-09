#include <Arduino.h>

#ifndef CStatGeneratorh
#define CStatGeneratorh

class CStatGenerator
{
  public:
    CStatGenerator(uint8_t minThrottleThresh, uint8_t maxThrottleThresh);
    void StoreValue(int16_t value, uint8_t throttle);
    int16_t GetAverage();
    int16_t m_Max;
  private:
    uint8_t m_minThrottleThresh, m_maxThrottleThresh;
    uint32_t m_accValue;
    uint32_t m_valCount;    
};

#endif
