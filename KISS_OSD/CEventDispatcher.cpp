#include "CEventDispatcher.h"

CEventDispatcher::CEventDispatcher():
startYawTime(0),
startRollTime(0),
startPitchTime(0),
m_handlerCount(0)
{
}

CEventDispatcher::~CEventDispatcher()
{
}

void CEventDispatcher::ProcessStickInputs(int16_t roll, int16_t pitch, int16_t yaw, int16_t armed)
{
  if(armed == 0)
  {
    CheckInput(roll, &startRollTime, rollPitchDelay, CEventHandler::RollLeft, CEventHandler::RollRight);
    CheckInput(pitch, &startPitchTime, rollPitchDelay, CEventHandler::PitchDown, CEventHandler::PitchUp);
    CheckInput(yaw, &startYawTime, yawDelay, CEventHandler::YawLeft, CEventHandler::YawRight);
  }
}

void CEventDispatcher::AddHandler(CEventHandler* handler)
{
  if(m_handlerCount < MAX_HANDLERS)
  {
    m_handlers[m_handlerCount] = handler;
    m_handlerCount++;
  }
}


void CEventDispatcher::CheckInput(int16_t input, unsigned long *startTime, unsigned long timedelay, CEventHandler::stickEvent eventSmall, CEventHandler::stickEvent eventBig)
{
  if(input > 1750 || input < 1250)
  {
    if(*startTime == 0)
    {
      *startTime = millis();
      DispatchEvent(input, startTime, eventSmall, eventBig);
    }
    else
    {
      if((millis() - *startTime) > timedelay)
      {
        DispatchEvent(input, startTime, eventSmall, eventBig);
      }
    }
  }
  else
  {
    *startTime = 0;
  }  
}

void CEventDispatcher::DispatchEvent(int16_t input, unsigned long *startTime, CEventHandler::stickEvent eventSmall, CEventHandler::stickEvent eventBig)
{
  CEventHandler::stickEvent event = eventBig;
  if(input < 1250)
  {
    event = eventSmall;
  }
  uint8_t i;
  for(i=0; i < m_handlerCount; i++)
  {
    m_handlers[i]->OnEvent(event);
  }
  *startTime = 0;
}
