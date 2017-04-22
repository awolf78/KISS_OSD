#include <Arduino.h>
#include "Config.h"

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
  #ifdef NEW_FILTER
  static const uint8_t m_maxBuf = 10;
  int16_t m_buf[m_maxBuf];
  uint8_t m_bufPos;
  #endif
};

#endif
