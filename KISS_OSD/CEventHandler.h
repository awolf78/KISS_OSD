#ifndef CEventHandlerh
#define CEventHandlerh

class CEventHandler
{
public:
	CEventHandler(){};
	~CEventHandler(){};
	enum stickEvent {PitchUp, PitchDown, RollLeft, RollRight, YawLeft, YawRight};
	virtual void OnEvent(CEventHandler::stickEvent event) = 0;
};

#endif