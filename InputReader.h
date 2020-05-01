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
	const uint8_t ArmInterruptPin;

	const uint32_t DebounceDuration = 100;
	const uint32_t BounceDuration = 2;

	bool DebouncedArmSignal = false;
	bool LastEmittedEvent = false;

	volatile uint32_t ArmPinLastChanged = 0;
	volatile bool LastInput = false;

	enum StateEnum : uint8_t
	{
		Disabled,
		Idle,
		Debouncing,
	};

	volatile StateEnum State = StateEnum::Disabled;

public:
	InputReader(Scheduler* scheduler, const uint8_t armInterruptPin)
		: EventTask(scheduler)
		, IInputReader()
		, ArmPin(armInterruptPin)
		, ArmInterruptPin(digitalPinToInterrupt(ArmPin))
	{
		pinMode(ArmPin, INPUT);
	}

	virtual void Enable()
	{
		if (State == StateEnum::Disabled)
		{
			ResetToIdle();
			AttachInterrupt();
		}
	}

	virtual void Disable()
	{
		State = StateEnum::Disabled;
		detachInterrupt(ArmInterruptPin);
		Task::disable();
	}

	virtual bool Setup(IEventListener* eventListener)
	{
		if (!EventTask::Setup(eventListener))
		{
			return false;
		}

		Disable();

		if (ArmInterruptPin != NOT_AN_INTERRUPT)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool Callback()
	{
		bool Input = LastInput;

		uint32_t Elapsed = millis() - ArmPinLastChanged;

		switch (State)
		{
		case StateEnum::Disabled:
			detachInterrupt(ArmInterruptPin);
			Task::disable();
			break;
		case StateEnum::Idle:
			if (Input != LastInput)
			{
				// Value has changed since the callback started.
				ResetToIdle();
			}
			else if (LastInput != LastEmittedEvent)
			{
				// Input detected fallback, this should never run but may catch edge cases.
				State = StateEnum::Debouncing;
				Task::enableIfNot();
				Task::forceNextIteration();
			}
			else
			{
				Task::disable();
			}
			break;
		case StateEnum::Debouncing:
			if (Input != LastInput || Input == LastEmittedEvent)
			{
				// Value might have changed since the callback started.
				// Value is bouncing back, wait for BounceDuration before cancelling.
				if (Elapsed >= BounceDuration)
				{
					ResetToIdle();
				}
				else
				{
					Task::delay(BounceDuration - Elapsed);
				}
			}
			else if (Elapsed >= DebounceDuration)
			{
				// Debounce duration without pin change has occured.
				UpdateDebouncedArmSignal(!LastEmittedEvent);
				ResetToIdle();
			}
			else
			{
				Task::delay(DebounceDuration - Elapsed);
			}
			break;
		default:
			break;
		}

		return true;
	}


	virtual bool IsArmSignalOn()
	{
		return !DebouncedArmSignal;
	}

	void OnArmPinInterrupt()
	{
		LastInput = digitalRead(ArmPin);

		switch (State)
		{
		case StateEnum::Disabled:
			Disable();
			break;
		case StateEnum::Idle:
			State = StateEnum::Debouncing;
			ArmPinLastChanged = millis();
			Task::enableIfNot();
			Task::forceNextIteration();
			break;
		case StateEnum::Debouncing:
			ArmPinLastChanged = millis();
			Task::enableIfNot();
			Task::forceNextIteration();
			break;
		default:
			break;
		}
	}

private:
	void AttachInterrupt();

	void UpdateDebouncedArmSignal(const bool on)
	{
		DebouncedArmSignal = on;
		LastEmittedEvent = on;

		EventListener->OnEvent();
	}

	void ResetToIdle()
	{
		ArmPinLastChanged = millis();

		if (digitalRead(ArmPin) != LastEmittedEvent)
		{
			State = StateEnum::Debouncing;
			Task::enableIfNot();
			Task::forceNextIteration();
		}
		else
		{
			State = StateEnum::Idle;
			Task::disable();
		}
	}
};
#endif