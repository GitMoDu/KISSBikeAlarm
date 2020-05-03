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
	enum StateEnum : uint8_t
	{
		Disabled,
		Active,
	};

	const uint8_t ArmPin;
	const uint8_t ArmInterruptPin;

	const uint32_t DebounceDuration = 100;

	bool DebouncedArmSignal = false;
	bool LastEmittedEvent = false;

	volatile StateEnum State = StateEnum::Disabled;
	volatile uint32_t ArmPinLastChanged = 0;
	volatile bool InterruptPending = false;

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
		if (State != StateEnum::Active)
		{
			DebouncedArmSignal = digitalRead(ArmPin);
			LastEmittedEvent = !DebouncedArmSignal;
			ResetToIdle();
			AttachInterrupt();
		}
	}

	virtual void Disable()
	{
		if (State != StateEnum::Disabled)
		{
			State = StateEnum::Disabled;
			detachInterrupt(ArmInterruptPin);
			Task::disable();
		}
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
		switch (State)
		{
		case StateEnum::Disabled:
			detachInterrupt(ArmInterruptPin);
			Task::disable();
			break;
		case StateEnum::Active:
			if (InterruptPending)
			{
				InterruptPending = false;
			}

			if (millis() - ArmPinLastChanged >= DebounceDuration)
			{
				// Debounce duration without pin change has occured.
				UpdateDebouncedArmSignal(digitalRead(ArmPin));
				ResetToIdle();
			}
			else if (InterruptPending)
			{
				// If interrupt fired while processing, start again.
				Task::enableIfNot();
				Task::forceNextIteration();
			}
			else
			{
				// Sleep the debounce period.
				Task::delay(DebounceDuration - (millis() - ArmPinLastChanged));
			}
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
		noInterrupts();
		switch (State)
		{
		case StateEnum::Disabled:
			Disable();
			break;
		case StateEnum::Active:
			ArmPinLastChanged = millis();
			InterruptPending = true;
			Task::enableIfNot();
			Task::forceNextIteration();
			break;
		default:
			break;
		}
		interrupts();
	}

private:
	void AttachInterrupt();

	void UpdateDebouncedArmSignal(const bool on)
	{
		DebouncedArmSignal = on;

		// Only fire event if value has changed.
		if (LastEmittedEvent != DebouncedArmSignal)
		{
			LastEmittedEvent = DebouncedArmSignal;
			EventListener->OnEvent();
		}
	}

	void ResetToIdle()
	{
		// Last minute check, before we sleep.
		if (digitalRead(ArmPin) != LastEmittedEvent)
		{
			State = StateEnum::Active;
			Task::enableIfNot();
			Task::forceNextIteration();
		}
		else
		{
			State = StateEnum::Active;
			Task::disable();
		}
	}
};
#endif