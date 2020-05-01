// IInputReader.h

#ifndef _IINPUTREADER_h
#define _IINPUTREADER_h

#include <stdint.h>

class IInputReader
{
public:
	virtual void Disable() {}
	virtual void Enable() {}

	virtual bool IsArmSignalOn() { return false; }
};
#endif

