// AlarmBuzzer.h

#ifndef _ALARMBUZZER_h
#define _ALARMBUZZER_h

#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>


#include <TimerOne.h> // https://github.com/PaulStoffregen/TimerOne

#include "..\IAlarmOutput.h"

class AlarmBuzzer : Task, public virtual IAlarmOutput
{
private:
	const uint8_t DrivePin;

	static const uint32_t BuzzerUpdatePeriodMillis = 2;
	static const uint32_t BuzzerCarriedPeriodMicros = 80;

	static const uint32_t ChirpPeriodDefault = 160;
	static const uint32_t ChirpDurationDefault = 15;
	static const uint32_t ChirpEntropy = 4;

	static const uint32_t AlarmPeriod = 1400;
	static const uint32_t AlarmBeeps = 3;
	static const uint32_t AlarmPausePeriod = 150;

	static const uint32_t ArmChirpCount = 2;
	static const uint32_t NotArmedChirpCount = 3;
	static const uint32_t NotArnedChirpPeriod = 60;
	static const uint32_t NotArmedChirpDuration = 10;

	static const uint32_t ArmingChirpCount = 10;
	static const uint32_t ArmingFailedChirpCount = 1;
	static const uint32_t ArmingChirpPeriod = 1000;
	static const uint32_t ArmingChirpDuration = 2;

	static const uint16_t AlarmLevel = 1023;
	static const uint16_t ArmedLevel = 100;
	static const uint16_t NotArmedLevel = 50;
	static const uint16_t ArmingLevel = 20;

	enum SoundEnum : uint8_t
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

	SoundEnum Current = SoundEnum::None;

	uint32_t CurrentStartedMillis = 0;
	uint32_t GenericBuzzDurationMillis = 0;

public:
	AlarmBuzzer(Scheduler* scheduler, const uint8_t drivePin)
		: Task(BuzzerUpdatePeriodMillis, TASK_FOREVER, scheduler, false)
		, IAlarmOutput()
		, DrivePin(drivePin)
	{
		pinMode(DrivePin, OUTPUT);
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
			Task::disable();
			break;
		case SoundEnum::Generic:
			StopPlaying();
			break;
		case SoundEnum::Error:
			StopPlaying();
			break;
		case SoundEnum::NotArmed:
			UpdateChirp(Elapsed, NotArmedChirpCount, NotArmedLevel, NotArnedChirpPeriod, NotArmedChirpDuration);
			break;
		case SoundEnum::Arming:
			UpdateChirp(Elapsed, ArmingChirpCount, ArmingLevel, ArmingChirpPeriod, ArmingChirpDuration);
			break;
		case SoundEnum::ArmingFailed:
			UpdateChirp(Elapsed, ArmingFailedChirpCount, NotArmedLevel);
			break;
		case SoundEnum::Armed:
			UpdateChirp(Elapsed, ArmChirpCount, NotArmedLevel);
			break;
		case SoundEnum::Alarm:
			UpdateAlarm(Elapsed);
			break;
		default:
			Task::disable();
			break;
		}

		return true;
	}

	void OnDisable()
	{
		StopPlaying();
	}

	// Interface implementations.
	virtual void Buzz(const uint32_t durationMillis)
	{
		PreparePlay(SoundEnum::Generic);
#ifdef DEBUG_LOG
		Serial.println(F("Buzz: Bzzzz"));
#endif
	}

	virtual void PlayError()
	{
		PreparePlay(SoundEnum::Error);

#ifdef DEBUG_LOG
		Serial.println(F("Buzz: Error"));
#endif
	}

	virtual void PlayArmed()
	{
		PreparePlay(SoundEnum::Armed);

#ifdef DEBUG_LOG
		Serial.println(F("Buzz: Armed"));
#endif
	}

	virtual void PlayNotArmed()
	{
		PreparePlay(SoundEnum::NotArmed);

#ifdef DEBUG_LOG
		Serial.println(F("Buzz: Not Armed"));
#endif
	}

	virtual void PlayEarlyWarning()
	{
		PreparePlay(SoundEnum::Alarm);

#ifdef DEBUG_LOG
		Serial.println(F("Buzz: Early Warning"));
#endif
	}

	virtual void PlayArming()
	{
		if (PreparePlay(SoundEnum::Arming))
		{
			// Skip first beat.
			Task::delay(ArmingChirpPeriod);
		}
	}

	virtual void PlayArmingFailed()
	{
		PreparePlay(SoundEnum::ArmingFailed);
	}

	virtual void PlayAlarm()
	{
		PreparePlay(SoundEnum::Alarm);
	}

	virtual void Stop()
	{
		StopPlaying();
	}

private:

	bool PreparePlay(SoundEnum newPlay)
	{
		if (Current != newPlay)
		{
			Current = newPlay;
			Timer1.pwm(DrivePin, 0);

			power_timer1_enable();
			Timer1.initialize(BuzzerCarriedPeriodMicros);
			Timer1.pwm(DrivePin, 0);

			Task::enableIfNot();
			Task::forceNextIteration();

			CurrentStartedMillis = millis();

			return true;
		}
		else
		{
			Task::enableIfNot();

			return false;
		}
	}

	void UpdateChirp(const uint32_t elapsed, const uint8_t chirpCount, const uint16_t level, const uint32_t chirpPeriod = ChirpPeriodDefault, const uint32_t chirpDuration = ChirpDurationDefault)
	{
		if (elapsed < chirpPeriod * chirpCount)
		{
			uint32_t Progress = elapsed % (chirpPeriod / chirpCount);

			if (Progress > chirpDuration)
			{
				Timer1.pwm(DrivePin, 0);
				Task::delay(chirpPeriod - Progress);
			}
			else
			{				
				Timer1.pwm(DrivePin, ((elapsed % ChirpEntropy) * level) / ChirpEntropy);
				Task::delay(1);
			}
		}
		else
		{
			StopPlaying();
		}
	}

	void UpdateAlarm(const uint32_t elapsed)
	{
		uint32_t Progress = AlarmPeriod - (elapsed % AlarmPeriod);

		if (Progress > (AlarmPeriod - AlarmPausePeriod))
		{
			Timer1.pwm(DrivePin, 0);
			Task::delay(AlarmPeriod - Progress);
		}
		else
		{
			Progress = Progress % ((AlarmPeriod - AlarmPausePeriod) / AlarmBeeps);

			Timer1.pwm(DrivePin, (Progress * AlarmLevel) / (AlarmPeriod - AlarmPausePeriod));
		}
	}

	void StopPlaying()
	{
		Current = SoundEnum::None;
		power_timer1_disable();

		pinMode(DrivePin, OUTPUT);
		digitalWrite(DrivePin, LOW);
		Task::disable();
	}
};

#endif