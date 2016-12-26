#include <Arduino.h>

#ifndef CMeanFilter_h
#define CMeanFilter_h


class CMeanFilter
{
  public:
  CMeanFilter(uint16_t maxCount);
  int16_t ProcessValue(const int16_t value);
  private:
  uint32_t m_accValue;
  uint16_t m_maxCount;
  uint16_t m_count;
  int16_t m_currentValue;
};

#endif
