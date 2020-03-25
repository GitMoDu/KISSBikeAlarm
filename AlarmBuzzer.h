// AlarmBuzzer.h

#ifndef _ALARMBUZZER_h
#define _ALARMBUZZER_h

#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>

#include "IAlarmBuzzer.h"

//template<const uint8_t DrivePinA, const uint8_t DrivePinB>
template<const uint8_t DrivePin>
class AlarmBuzzer : Task, public IAlarmBuzzer
{
public:
	AlarmBuzzer(Scheduler* scheduler)
		: Task(0, TASK_FOREVER, scheduler, false)
	{
	}

	virtual void Buzz(const uint32_t periodMillis)
	{
	}

	bool Callback()
	{

	}
};

#endif