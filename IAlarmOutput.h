// IAlarmOutput.h

#ifndef _IALARM_OUTPUT_h
#define _IALARM_OUTPUT_h

#include <stdint.h>

class IAlarmOutput
{
public:
	virtual void Buzz(const uint32_t durationMillis) {}
	virtual void Stop() {}

	virtual void PlayError() {}
	virtual void PlayArmed() {}
	virtual void PlayArming() {}
	virtual void PlayArmingFailed() {}
	virtual void PlayNotArmed() {}
	virtual void PlayEarlyWarning() {}
	virtual void PlayAlarm() {}
};
#endif