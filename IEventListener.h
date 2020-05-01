// IEventListener.h

#ifndef _IEVENTLISTENER_h
#define _IEVENTLISTENER_h

#include <stdint.h>


class IEventListener
{
public:
	virtual void OnEvent() {}
};

#endif