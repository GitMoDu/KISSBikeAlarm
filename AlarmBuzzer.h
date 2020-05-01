// AlarmBuzzer.h

#ifndef _ALARMBUZZER_h
#define _ALARMBUZZER_h

#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>

#include "IAlarmOutput.h"

class AlarmBuzzer : Task, public virtual IAlarmOutput
{
private:
	const uint8_t DrivePin;

	enum SoundEnum : uint8_t
	{
		None,
		Generic,
		Error,
		NotArmed,
		Arming,
		ArmingFailed,
		Armed,
		EarlyWarning,
		Alarm
	};

	SoundEnum Current = SoundEnum::None;

	uint32_t CurrentStartedMillis = 0;
	uint32_t GenericBuzzDurationMillis = 0;

public:
	AlarmBuzzer(Scheduler* scheduler, const uint8_t drivePin)
		: Task(50, TASK_FOREVER, scheduler, false)
		, IAlarmOutput()
		, DrivePin(drivePin)
	{
	}

	bool Setup()
	{
		return true;
	}

	bool Callback()
	{
		uint32_t Elapsed = millis() - CurrentStartedMillis;

		switch (Current)
		{
		case SoundEnum::None:
			disable();
			break;
		case SoundEnum::Generic:
			break;
		case SoundEnum::Error:
			break;
		case SoundEnum::NotArmed:
			break;
		case SoundEnum::Arming:
			break;
		case SoundEnum::ArmingFailed:
			break;
		case SoundEnum::Armed:
			break;
		case SoundEnum::EarlyWarning:
			break;
		case SoundEnum::Alarm:
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

#ifdef DEBUG_LOG
		Serial.println(F("Buzz: Bzzzz"));
#endif
	}

	virtual void PlayError()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = SoundEnum::Error;
		enable();

#ifdef DEBUG_LOG
		Serial.println(F("Buzz: Error"));
#endif
	}

	virtual void PlayArmed()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = SoundEnum::Armed;
		enable();

#ifdef DEBUG_LOG
		Serial.println(F("Buzz: Armed"));
#endif
	}

	virtual void PlayNotArmed()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = SoundEnum::NotArmed;
		enable();

#ifdef DEBUG_LOG
		Serial.println(F("Buzz: Not Armed"));
#endif
	}

	virtual void PlayEarlyWarning()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = SoundEnum::EarlyWarning;
		enable();

#ifdef DEBUG_LOG
		Serial.println(F("Buzz: Early Warning"));
#endif
	}

	virtual void PlayArming()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = SoundEnum::Arming;
		enable();

#ifdef DEBUG_LOG
		Serial.println(F("Buzz: Arming"));
#endif
	}

	virtual void PlayArmingFailed()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = SoundEnum::ArmingFailed;
		enable();

#ifdef DEBUG_LOG
		Serial.println(F("Buzz: Arming Failed"));
#endif
	}

	virtual void PlayAlarm()
	{
		StopPlaying();
		CurrentStartedMillis = millis();
		Current = SoundEnum::Alarm;
		enable();

#ifdef DEBUG_LOG
		Serial.println(F("Buzz: AUUUUUGAAAAAAAAAAA"));
#endif
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
		disable();
	}
};

#endif