#ifndef CEventDispatcherh
#define CEventDispatcherh
#include "CEventHandler.h"
#include <Arduino.h>

class CEventDispatcher
{
public:
  CEventDispatcher();
  ~CEventDispatcher();
  void ProcessStickInputs(int16_t roll, int16_t pitch, int16_t yaw, int16_t armed);
  void AddHandler(CEventHandler* handler);
  
private:
  static const unsigned long yawDelay = 3000;
  static const unsigned long rollPitchDelay = 500;
  unsigned long startYawTime, startRollTime, startPitchTime;
  static const uint8_t MAX_HANDLERS = 15;
  uint8_t m_handlerCount;
  CEventHandler* m_handlers[MAX_HANDLERS];
  
  void CheckInput(int16_t input, unsigned long *startTime, unsigned long timedelay, CEventHandler::stickEvent eventSmall, CEventHandler::stickEvent eventBig);
  void DispatchEvent(int16_t input, unsigned long *startTime, CEventHandler::stickEvent eventSmall, CEventHandler::stickEvent eventBig);
};

#endif
