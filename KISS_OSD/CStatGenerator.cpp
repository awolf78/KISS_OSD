#include "CStatGenerator.h"

CStatGenerator::CStatGenerator(uint8_t minThrottleThresh, uint8_t maxThrottleThresh) :
m_minThrottleThresh(minThrottleThresh),
m_maxThrottleThresh(maxThrottleThresh),
m_accValue(0),
m_valCount(0),
m_Max(0)
{
  
}

void CStatGenerator::StoreValue(int16_t value, uint8_t throttle)
{
  if(throttle >= m_minThrottleThresh && throttle <= m_maxThrottleThresh)
  {
    m_accValue += (uint32_t)value;
    m_valCount++;
    if(value > m_Max) m_Max = value;
  }
}

int16_t CStatGenerator::GetAverage()
{
  uint32_t temp = 0;
  if(m_valCount > 0) temp = m_accValue/m_valCount;
  return (int16_t)temp;
}
