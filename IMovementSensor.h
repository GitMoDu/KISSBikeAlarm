// IMovementSensor.h

#ifndef _IMOVEMENTSENSOR_h
#define _IMOVEMENTSENSOR_h


#include <stdint.h>

class IMovementSensor
{
public:
	virtual void Disable() {}
	virtual void Enable() {}
	virtual bool HasRecentSignificantMotion(const uint32_t period) { return false;  }
};

#endif

