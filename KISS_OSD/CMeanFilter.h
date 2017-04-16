#include <Arduino.h>

#ifndef CMeanFilter_h
#define CMeanFilter_h


class CMeanFilter
{
  public:
  CMeanFilter(uint8_t maxCount);
  int16_t ProcessValue(const int16_t value);
  private:
  uint32_t m_accValue;
  uint8_t m_maxCount;
  uint8_t m_count;
  int16_t m_currentValue;
};

#endif
