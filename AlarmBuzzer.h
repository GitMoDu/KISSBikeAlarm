// AlarmBuzzer.h

#ifndef _ALARMBUZZER_h
#define _ALARMBUZZER_h

#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>

#include "IAlarmBuzzer.h"

class AlarmBuzzer : Task, public virtual IAlarmBuzzer
{
private:
	const uint8_t DrivePin;

	enum SoundEnum : uint8_t
	{
		None,
		Generic,
		Error,
		NotArmed,
		Armed,
		EarlyWarning,
		LastWarning,
		Alarm
	};

	SoundEnum Current = SoundEnum::None;

	uint32_t CurrentStartedMillis = 0;
	uint32_t GenericBuzzDurationMillis = 0;

#ifdef DEBUG_LOG
	bool Debugged = false;
#endif

public:
	AlarmBuzzer(Scheduler* scheduler, const uint8_t drivePin)
		: Task(50, TASK_FOREVER, scheduler, false)
		, IAlarmBuzzer()
		, DrivePin(drivePin)
	{
	}

	bool Setup()
	{
		return true;
	}

	bool Callback()
	{
		switch (Current)
		{
		case SoundEnum::None:
			disable();
			break;
		case SoundEnum::Generic:
#ifdef DEBUG_LOG
			if (!Debugged)
			{
				Debugged = true;
				Serial.println(F("Buzz: Bzzzz"));
			}
#endif
			break;
		case SoundEnum::Error:
#ifdef DEBUG_LOG
			if (!Debugged)
			{
				Debugged = true;
				Serial.println(F("Buzz: Error"));
			}
#endif
			break;
		case SoundEnum::NotArmed:
#ifdef DEBUG_LOG
			if (!Debugged)
			{
				Debugged = true;
				Serial.println(F("Buzz: NotArmed"));
			}
#endif
			break;
		case SoundEnum::Armed:
#ifdef DEBUG_LOG
			if (!Debugged)
			{
				Debugged = true;
				Serial.println(F("Buzz: Armed"));
			}
#endif
			break;
		case SoundEnum::EarlyWarning:
#ifdef DEBUG_LOG
			if (!Debugged)
			{
				Debugged = true;
				Serial.println(F("Buzz: Early Warning"));
			}
#endif
			break;
		case SoundEnum::LastWarning:
#ifdef DEBUG_LOG
			if (!Debugged)
			{
				Debugged = true;
				Serial.println(F("Buzz: Last Warning"));
			}
#endif
			break;
		case SoundEnum::Alarm:
#ifdef DEBUG_LOG
			if (!Debugged)
			{
				Debugged = true;
				Serial.println(F("Buzz: Alarming!!!"));
			}
#endif
			break;
		default:
			disable();
			break;
		}
	}

	bool OnEnable()
	{
		pinMode(DrivePin, OUTPUT);
		digitalWrite(DrivePin, LOW);

		return true;
	}

	void OnDisable()
	{
		StopPlaying();
	}

	// Interface implementations.
	virtual void Buzz(const uint32_t durationMillis)
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = SoundEnum::Generic;
		GenericBuzzDurationMillis = durationMillis;
		enable();
	}

	virtual void PlayError()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = SoundEnum::Error;
		enable();
	}

	virtual void PlayArmed()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = SoundEnum::Armed;
		enable();
	}

	virtual void PlayNotArmed()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = SoundEnum::NotArmed;
		enable();
	}

	virtual void PlayEarlyWarning()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = SoundEnum::EarlyWarning;
		enable();
	}

	virtual void PlayLastWarning()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = SoundEnum::LastWarning;
		enable();
	}

	virtual void PlayAlarm()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = SoundEnum::NotArmed;
		enable();
	}

	virtual void Stop()
	{
		StopPlaying();
	}

private:
	void StopPlaying()
	{
		Current = SoundEnum::None;
		digitalWrite(DrivePin, LOW);
#ifdef DEBUG_LOG	
		Debugged = false;
#endif
	}
};

#endif