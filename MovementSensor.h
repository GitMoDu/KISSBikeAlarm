// MovementSensor.h

#ifndef _MOVEMENTSENSOR_h
#define _MOVEMENTSENSOR_h

#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>

#include "EventTask.h"

#include "IMovementSensor.h"

class MovementSensor : EventTask, public virtual IMovementSensor
{
private:
	const uint8_t InterruptPin;

public:
	MovementSensor(Scheduler* scheduler, const uint8_t interruptPin)
		: EventTask(scheduler)
		, IMovementSensor()
		, InterruptPin(interruptPin)
	{
	}

	virtual bool Setup(IEventListener* eventListener)
	{
		if (!EventTask::Setup(eventListener))
		{
			return false;
		}

		pinMode(InterruptPin, INPUT_PULLUP);

		return true;
	}

	bool Callback()
	{

	}

	void OnPinInterrupt()
	{
		EventListener->OnEvent();
	}
};


#endif

