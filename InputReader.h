// InputReader.h

#ifndef _INPUTREADER_h
#define _INPUTREADER_h


#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>

#include <Arduino.h>

#include "IInputReader.h"

#include "EventTask.h"

class InputReader : EventTask, public virtual IInputReader
{
private:
	const uint8_t ArmPin;

	const uint32_t DebounceDuration = 200;
	const uint32_t WaitDuration = 10;

	bool DebouncedArmSignal;

	uint32_t ArmPinLastChanged = 0;
	bool EventLastEmitted = false;

	bool PendingEvent = false;

public:
	InputReader(Scheduler* scheduler, const uint8_t armInterruptPin)
		: EventTask(scheduler)
		, IInputReader()
		, ArmPin(armInterruptPin)
	{
	}

	virtual void ArmInterrupt();

	bool Callback()
	{
		if (PendingEvent)
		{
			if (millis() - ArmPinLastChanged > DebounceDuration)
			{
				PendingEvent = false;
				DebouncedArmSignal = digitalRead(ArmPin);
				if (EventLastEmitted != DebouncedArmSignal)
				{
					EventLastEmitted = DebouncedArmSignal;
					EventListener->OnEvent();
				}
			}
			else
			{
				Task::delay(WaitDuration);
			}

			return true;
		}
		else
		{
			Task::disable();
			return false;
		}
	}

	virtual bool Setup(IEventListener* eventListener)
	{
		if (!EventTask::Setup(eventListener))
		{
			return false;
		}

		//TODO: Validate interrupt pins.

		pinMode(ArmPin, INPUT);

		detachInterrupt(digitalPinToInterrupt(ArmPin));

		return true;
	}

	virtual bool IsArmSignalOn()
	{
		return !DebouncedArmSignal;
	}

	virtual void Disable()
	{
		detachInterrupt(digitalPinToInterrupt(ArmPin));
	}

	void OnDisarmPinInterrupt()
	{
		ArmPinLastChanged = millis();
		PendingEvent = true;
		enableIfNot();
		Task::delay(WaitDuration);
	}
};
#endif