// AlarmLight.h

#ifndef _ALARMLIGHT_h
#define _ALARMLIGHT_h

#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>

#include "IAlarmOutput.h"

class AlarmLight : Task, public virtual IAlarmOutput
{
private:
	const uint8_t DrivePin;

	enum LightEnum : uint8_t
	{
		None,
		Generic,
		Error,
		NotArmed,
		Arming,
		ArmingFailed,
		Armed,
		Alarm
	};

	LightEnum Current = LightEnum::None;

	uint32_t CurrentStartedMillis = 0;

#ifdef DEBUG_LOG
	bool Debugged = false;
#endif

public:
	AlarmLight(Scheduler* scheduler, const uint8_t drivePin)
		: Task(50, TASK_FOREVER, scheduler, false)
		, IAlarmOutput()
		, DrivePin(drivePin)
	{
	}

	bool Setup()
	{
		return true;
	}

	bool OnEnable()
	{
		return true;
	}

	void OnDisable()
	{
		StopPlaying();
	}


	bool Callback()
	{
		uint32_t Elapsed = millis() - CurrentStartedMillis;

		switch (Current)
		{
		case LightEnum::None:
			//TODO: Clear lights.
			disable();
			break;
		case LightEnum::Error:
			break;
		case LightEnum::NotArmed:
			break;
		case LightEnum::Arming:
			break;
		case LightEnum::ArmingFailed:
			break;
		case LightEnum::Armed:
			break;
		case LightEnum::Alarm:
			break;
		default:
			disable();
			break;
		}
	}

	virtual void PlayError()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = LightEnum::Error;
		enable();
	}

	virtual void PlayArmed()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = LightEnum::Armed;
		enable();
	}

	virtual void PlayNotArmed()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = LightEnum::NotArmed;
		enable();
	}

	virtual void PlayArming()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = LightEnum::Arming;
		enable();
	}

	virtual void PlayArmingFailed()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = LightEnum::ArmingFailed;
		enable();
	}

	virtual void PlayAlarm()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = LightEnum::Alarm;
		enable();
	}

	virtual void Stop()
	{
		StopPlaying();
	}

private:
	void StopPlaying()
	{
		Current = LightEnum::None;
		Task::enableIfNot();
		Task::forceNextIteration();
	}
};

#endif