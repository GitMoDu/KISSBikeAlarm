// EventTask.h

#ifndef _EVENTTASK_h
#define _EVENTTASK_h

#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>

#include "..\IEventListener.h"



class EventSource
{
protected:
	IEventListener* EventListener = nullptr;

public:
	EventSource()
	{
	}

	virtual bool Setup(IEventListener* eventListener)
	{
		EventListener = eventListener;

		return EventListener != nullptr;
	}
};

class EventTask : public Task
{
protected:
	IEventListener* EventListener = nullptr;

public:
	EventTask(Scheduler* scheduler)
		: Task(0, TASK_FOREVER, scheduler, false)
	{
	}

	virtual bool Setup(IEventListener* eventListener)
	{
		EventListener = eventListener;

		return EventListener != nullptr;
	}
};


#endif