// InputReader.h

#ifndef _INPUTREADER_h
#define _INPUTREADER_h


#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>

#include <Arduino.h>

#include "IInputReader.h"

#include "EventTask.h"

class InputReader : EventSource, public virtual IInputReader
{
private:
	const uint8_t PowerPin;
	const uint8_t ArmPin;

public:
	InputReader(const uint8_t armInterruptPin, const uint8_t powerPin)
		: EventSource()
		, IInputReader()
		, PowerPin(powerPin)
		, ArmPin(armInterruptPin)
	{
	}

	virtual void ArmInterrupt();

	virtual bool Setup(IEventListener* eventListener)
	{
		if (!EventSource::Setup(eventListener))
		{
			return false;
		}

		//TODO: Validate interrupt pins.

		pinMode(PowerPin, INPUT_PULLUP);
		pinMode(ArmPin, INPUT);

		detachInterrupt(digitalPinToInterrupt(ArmPin));

		return true;
	}

	virtual bool IsMainPowerOn()
	{
		return digitalRead(PowerPin);
	}

	virtual bool IsArmSignalOn()
	{
		return !digitalRead(ArmPin);
	}

	virtual void Disable()
	{
		detachInterrupt(digitalPinToInterrupt(ArmPin));

		ClearLog();
	}

	void OnDisarmPinInterrupt()
	{
		ArmPinLastChanged = millis();
		EventListener->OnEvent();
	}

private:
	void ClearLog()
	{
		//TODO:
	}
};
#endif