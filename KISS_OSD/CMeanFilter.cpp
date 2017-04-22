#include "CMeanFilter.h"

CMeanFilter::CMeanFilter(uint8_t maxCount) :
m_maxCount(maxCount),
m_accValue(0),
m_count(0),
m_currentValue(0)
{
  #ifdef NEW_FILTER
  m_bufPos = 0;
  if(m_maxCount > m_maxBuf) m_maxCount = m_maxBuf;
  #endif
}
  
int16_t CMeanFilter::ProcessValue(const int16_t value)
{
  #ifdef NEW_FILTER
  uint8_t newPos = m_bufPos % m_maxCount;
  m_bufPos++;
  m_buf[newPos] = value;  
  m_accValue = (uint32_t)m_buf[newPos] * (uint32_t)m_maxCount;
  uint8_t count = m_maxCount;
  uint8_t j;
  uint8_t i = newPos+1;
  for(j=1; j < m_maxCount; j++)
  {
    m_accValue += (uint32_t)m_buf[i % m_maxCount] * (uint32_t)j;
    count += j;
    i++;
  }
  m_currentValue = (int16_t)((uint32_t) m_accValue / (uint32_t) count);
  #else
  
  m_accValue += (uint32_t)value;
  m_count++;
  if(m_count == m_maxCount)
  {
    m_currentValue = (int16_t)((uint32_t) m_accValue / (uint32_t) m_maxCount);
    m_count = 0;
    m_accValue = 0;
  }
  #endif
  return m_currentValue;
}
