// IAlarmBuzzer.h

#ifndef _IALARMBUZZER_h
#define _IALARMBUZZER_h

#include <stdint.h>

class IAlarmBuzzer
{
public:
	virtual void Buzz(const uint32_t periodMillis) {}
};
#endif