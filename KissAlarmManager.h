// KissAlarmTask.h

#ifndef _KISSALARMMANAGER_h
#define _KISSALARMMANAGER_h

#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>


class KissAlarmManager : Task
{
private: 
	IAlarmBuzzer* Buzzer = nullptr;

	enum StateEnum : uint8_t 
	{
		Disabled = 0
	};

	StateEnum State = StateEnum::Disabled;

public:
	KissAlarmManager(Scheduler* scheduler, IAlarmBuzzer* buzzer)
		: Task(0, TASK_FOREVER, scheduler, false)
	{
		Buzzer = buzzer;
	}

	bool Callback()
	{
		return true;
	}

	void OnDisable()
	{

	}
};
#endif

