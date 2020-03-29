// IAlarmBuzzer.h

#ifndef _IALARMBUZZER_h
#define _IALARMBUZZER_h

#include <stdint.h>

class IAlarmBuzzer
{
public:
	virtual void Buzz(const uint32_t durationMillis) {}
	virtual void PlayError() {}
	virtual void PlayArmed() {}
	virtual void PlayNotArmed() {}
	virtual void PlayEarlyWarning() {}
	virtual void PlayLastWarning() {}
	virtual void PlayAlarm() {}
	virtual void Stop() {}
};
#endif