// IMovementSensor.h

#ifndef _IMOVEMENTSENSOR_h
#define _IMOVEMENTSENSOR_h


#include <stdint.h>

class IMovementSensor
{
public:
	virtual void Disable() {}
	virtual void ArmInterrupt() {}
	virtual bool HasRecentSignificantMotion() { return false;  }
	virtual bool IsDeviceReady() { return false; }
};

#endif

