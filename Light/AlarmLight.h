// AlarmLight.h

#ifndef _ALARMLIGHT_h
#define _ALARMLIGHT_h

#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>

#include "..\IAlarmOutput.h"

#include "..\AlarmConstants.h"


// use the cRGB struct hsv method
#define USE_HSV
#include <WS2812.h> // https://github.com/cpldcpu/light_ws2812

class AlarmLight : Task, public virtual IAlarmOutput
{
private:
	const uint8_t DrivePin;

	static const uint8_t LedCount = 1;
	static const uint32_t AnimationPeriod = 10;

	static const uint8_t Brightness = 255;
	static const uint8_t ArmedBrightness = 230;

	WS2812 LED;

	cRGB Value;

	enum LightEnum : uint8_t
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

	LightEnum Current = LightEnum::None;

	uint32_t CurrentStartedMillis = 0;

#ifdef DEBUG_LOG
	bool Debugged = false;
#endif

public:
	AlarmLight(Scheduler* scheduler, const uint8_t drivePin)
		: Task(AnimationPeriod, TASK_FOREVER, scheduler, false)
		, IAlarmOutput()
		, DrivePin(drivePin)
		, LED(LedCount)
	{
		LED.setOutput(DrivePin);
		Stop();
	}

	bool Setup()
	{
		Stop();

		return true;
	}

	bool OnEnable()
	{
		return true;
	}

	bool Callback()
	{
		const uint32_t Elapsed = millis() - CurrentStartedMillis;

		// Set default animation period wait.
		// Animations may override the value.
		Task::delay(AnimationPeriod);

		switch (Current)
		{
		case LightEnum::None:
			Value.SetHSV(0, 0, 0);
			Task::disable();
			break;
		case LightEnum::Error:
			UpdateError(Elapsed);
			break;
		case LightEnum::NotArmed:
			UpdateNotArmed(Elapsed);
			break;
		case LightEnum::Arming:
			UpdateArming(Elapsed);
			break;
		case LightEnum::ArmingFailed:
			UpdateError(Elapsed);
			break;
		case LightEnum::Armed:
			UpdateArmed(Elapsed);
			break;
		case LightEnum::EarlyWarning:
			// Early warning is same as alarm.
			UpdateAlarm(Elapsed);
			break;
		case LightEnum::Alarm:
			UpdateAlarm(Elapsed);
			break;
		default:
			Stop();
			return;
		}

		UpdateLED();

		return true;
	}

	virtual void PlayError()
	{
		CurrentStartedMillis = millis();
		Current = LightEnum::Error;
		Task::enable();
	}

	virtual void PlayArmed()
	{
		CurrentStartedMillis = millis();
		Current = LightEnum::Armed;
		Task::enable();
	}

	virtual void PlayNotArmed()
	{
		CurrentStartedMillis = millis();
		Current = LightEnum::NotArmed;
		Task::enable();
	}

	virtual void PlayArming()
	{
		CurrentStartedMillis = millis();
		Current = LightEnum::Arming;
		Task::enable();
	}

	virtual void PlayArmingFailed()
	{
		CurrentStartedMillis = millis();
		Current = LightEnum::ArmingFailed;
		Task::enable();
	}

	virtual void PlayEarlyWarning()
	{
		CurrentStartedMillis = millis();
		Current = LightEnum::EarlyWarning;
		Task::enable();
	}

	virtual void PlayAlarm()
	{
		CurrentStartedMillis = millis();
		Current = LightEnum::Alarm;
		Task::enable();
	}

	virtual void Stop()
	{
		Current = LightEnum::None;
		Task::enableIfNot();
		Task::forceNextIteration();
	}

private:
	void UpdateLED()
	{
		LED.set_crgb_at(0, Value);

		// Only one LED, takes about 40 us @ 16 MHz.
		noInterrupts();
		LED.sync();
		interrupts();
	}

	void UpdateError(const uint32_t elapsed)
	{
		const uint32_t ErrorFlashPeriod = 700;
		const uint32_t Progress = elapsed % ErrorFlashPeriod;

		if (Progress > (ErrorFlashPeriod / 2))
		{
			Value.SetHSV(0, 255, Brightness);
			Task::delay(ErrorFlashPeriod - Progress);
		}
		else
		{
			Value.SetHSV(35, 255, Brightness);
			Task::delay((ErrorFlashPeriod / 2) - Progress);
		}
	}

	void UpdateAlarm(const uint32_t elapsed)
	{
		const uint32_t AlarmFlashPeriod = 100;
		const uint32_t Progress = elapsed % AlarmFlashPeriod;

		if (Progress > (AlarmFlashPeriod / 2))
		{
			Value.SetHSV(0, 255, Brightness);
			Task::delay(AlarmFlashPeriod - Progress);
		}
		else
		{
			Value.SetHSV(45, 255, Brightness);
			Task::delay((AlarmFlashPeriod / 2) - Progress);
		}
	}

	void UpdateNotArmed(const uint32_t elapsed)
	{
		const uint32_t TotalDuration = ARMED_FLASH_PERIOD_MILLIS;
		const uint32_t HuePeriod = 50;
		const uint8_t PresenceBrightness = 1;

		if (elapsed > TotalDuration)
		{
			Value.r = 0;
			Value.g = 0;
			Value.b = PresenceBrightness;
			Task::disable();
		}
		else
		{
			const int16_t NotArmedHue = map(elapsed % HuePeriod, 0, HuePeriod, 0, 360);
			Value.SetHSV(NotArmedHue, 255, (uint8_t)map(elapsed, 0, TotalDuration, Brightness, 0));
		}
	}

	void UpdateArmed(const uint32_t elapsed)
	{
		const uint32_t ArmedFlashPeriod = 3000;
		const uint32_t FlashDuration = 90;

		const uint32_t Progress = elapsed % ArmedFlashPeriod;

		if (Progress <= FlashDuration)
		{
			Value.r = 0;
			Value.g = 0;
			Value.b = map(Progress, 0, FlashDuration, ArmedBrightness, 0);
		}
		else
		{
			Value.r = 0;
			Value.g = 0;
			Value.b = 0;
			Task::delay(ArmedFlashPeriod - Progress);
		}
	}

	void UpdateArming(const uint32_t elapsed)
	{
		const uint32_t TotalDuration = ARM_PERIOD_MILLIS - 1;
		const uint32_t HeadDuration = 2000;
		const uint32_t TailDuration = 3000;

		if (elapsed > TotalDuration)
		{
			Stop();
		}

		if (elapsed < HeadDuration)
		{
			Value.r = Brightness;
			Value.g = 0;
			Value.b = 0;
		}
		else if (elapsed > (TotalDuration - HeadDuration))
		{
			Value.r = 0;
			Value.g = 0;
			Value.b = map(elapsed, (TotalDuration - HeadDuration), TotalDuration, Brightness, 0);
		}
		else
		{
			Value.r = map(elapsed, HeadDuration, (TotalDuration - HeadDuration), Brightness, 0);
			Value.g = 0;
			Value.b = Brightness - Value.r;
		}
	}
};

#endif