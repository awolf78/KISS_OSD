#include "CMeanFilter.h"

CMeanFilter::CMeanFilter(uint8_t maxCount) :
m_maxCount(maxCount),
m_accValue(0),
m_count(0),
m_currentValue(0)
{
}
  
int16_t CMeanFilter::ProcessValue(const int16_t value)
{
  m_accValue += (uint32_t)value;
  m_count++;
  if(m_count == m_maxCount)
  {
    m_currentValue = (int16_t)((uint32_t) m_accValue / (uint32_t) m_maxCount);
    m_count = 0;
    m_accValue = 0;
  }
  return m_currentValue;
}
