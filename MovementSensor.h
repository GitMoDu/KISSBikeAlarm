// MovementSensor.h

#ifndef _MOVEMENTSENSOR_h
#define _MOVEMENTSENSOR_h

#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>

#include "EventTask.h"

#include "IMovementSensor.h"

class MovementSensor : EventTask, public virtual IMovementSensor
{
public:
	MovementSensor(Scheduler* scheduler)
		: EventTask(scheduler)
		, IMovementSensor()
	{
	}

	virtual bool Setup(IEventListener* eventListener)
	{
		if (!EventTask::Setup(eventListener))
		{
			return false;
		}

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

