// KissAlarmTask.h

#ifndef _KISSALARMMANAGER_h
#define _KISSALARMMANAGER_h

#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>

#include "IMovementSensor.h"
#include "IAlarmOutput.h"
#include "IEventListener.h"

class KissAlarmManager : Task, public virtual IEventListener
{
private:
	const uint32_t MOVEMENT_PERIOD_MILLIS = 600;
	const uint32_t TRANSITION_GRACE_PERIOD_MILLIS = 2000;

	const uint32_t ARM_MIN_PERIOD_MILLIS = 5000;
	const uint32_t ARM_MAX_WARMUP_PERIOD_MILLIS = ARM_MIN_PERIOD_MILLIS * 3;
	const uint32_t REARM_WAIT_PERIOD_MILLIS = 5000;
	const uint32_t ALARMING_DURATION_MILLIS = 5000; //3 * 60 * 1000;

	const uint32_t ARMING_CHECK_PERIOD_MILLIS = 10;
	static const uint32_t MIN_RUN_PERIOD_MILLIS = 2;
	const uint32_t ALARMING_CHECK_PERIOD_MILLIS = 1000;
	const uint32_t EARLY_WARNING_PERIOD_MILLIS = 1000 + MOVEMENT_PERIOD_MILLIS + TRANSITION_GRACE_PERIOD_MILLIS;
	const uint32_t EARLY_WARNING_SKIP_MILLIS = 3000 + EARLY_WARNING_PERIOD_MILLIS;

	IAlarmOutput* Buzzer = nullptr;

	IAlarmOutput* Light = nullptr;

	IMovementSensor* MovementDetector = nullptr;

	IInputReader* InputReader = nullptr;

	enum StateEnum : uint8_t
	{
		Disabled,
		WakingUp,
		NotArmed,
		Arming,
		ArmingFailed,
		Armed,
		ArmingEarlyWarning,
		EarlyWarning,
		Alarming,
		StateCount
	};

	StateEnum State = StateEnum::Disabled;

	uint32_t StateStartedTimestamp = 0;
	uint32_t LastWarningTimestamp = 0;

public:
	KissAlarmManager(Scheduler* scheduler)
		: Task(0, TASK_FOREVER, scheduler, false)
		, IEventListener()
	{
		pinMode(LED_BUILTIN, OUTPUT);
		digitalWrite(LED_BUILTIN, LOW);
	}

	bool Setup(IAlarmOutput* buzzer, IAlarmOutput* light, IMovementSensor* movementSensor
		, IInputReader* inputReader)
	{
		bool Success = true;

		Buzzer = buzzer;
		Light = light;
		MovementDetector = movementSensor;
		InputReader = inputReader;

		if (Buzzer == nullptr ||
			Light == nullptr ||
			MovementDetector == nullptr ||
			InputReader == nullptr)
		{
			Success = false;
		}

		if (Success)
		{
			UpdateState(StateEnum::WakingUp);

			return true;
		}
		else
		{
			UpdateState(StateEnum::Disabled);

			return false;
		}
	}

	virtual void OnEvent()
	{
		switch (State)
		{
		case StateEnum::Disabled:
			break;
		default:
			Task::enableIfNot();
			Task::forceNextIteration();
			break;
		}
	}

	void UpdateState(StateEnum state, const uint32_t nextRunDelayMillis = MIN_RUN_PERIOD_MILLIS)
	{
		if (State != state)
		{
#if defined(DEBUG_LOG) && defined(DEBUG_STATE)
			Serial.print(millis());
			Serial.print(F(" - Alarm State("));
			Serial.print(millis() - StateStartedTimestamp);
			Serial.print(F("): "));
			Serial.println(state);
#endif

			StateStartedTimestamp = millis();
			State = state;

			switch (state)
			{
			case StateEnum::Disabled:
				MovementDetector->Disable();
				InputReader->Disable();

				Light->PlayError();
				Buzzer->PlayError();
				break;
			case StateEnum::WakingUp:
				MovementDetector->Disable();
				InputReader->Disable();

				Light->Stop();
				Buzzer->Stop();
				break;
			case StateEnum::NotArmed:
				InputReader->Enable();
				MovementDetector->Disable();

				Light->PlayNotArmed();
				Buzzer->PlayNotArmed();
				break;
			case StateEnum::Arming:
				InputReader->Enable();
				MovementDetector->Enable();

				Light->PlayArming();
				Buzzer->PlayArming();
				break;
			case StateEnum::ArmingFailed:
				InputReader->Enable();
				MovementDetector->Disable();

				Light->PlayArmingFailed();
				Buzzer->PlayArmingFailed();
				break;
			case StateEnum::Armed:
				InputReader->Enable();
				MovementDetector->Enable();

				Light->PlayArmed();
				Buzzer->PlayArmed();
				break;
			case StateEnum::ArmingEarlyWarning:
				InputReader->Enable();
				MovementDetector->Enable();

				Buzzer->PlayEarlyWarning();
				break;
			case StateEnum::EarlyWarning:
				InputReader->Enable();
				MovementDetector->Enable();
				break;
			case StateEnum::Alarming:
				MovementDetector->Disable();
				InputReader->Enable();

				Light->PlayAlarm();
				Buzzer->PlayAlarm();
				break;
			case StateEnum::StateCount:
				break;
			default:
				break;
			}

			Task::enableIfNot();
			if (nextRunDelayMillis > 0)
			{
				Task::delay(nextRunDelayMillis);
			}
			else
			{
				Task::forceNextIteration();
			}
		}
	}

	bool Callback()
	{
		uint32_t StateElapsed = millis() - StateStartedTimestamp;

		switch (State)
		{
		case StateEnum::WakingUp:
			UpdateState(StateEnum::NotArmed);
			break;
		case StateEnum::NotArmed:
			if (InputReader->IsArmSignalOn())
			{
				UpdateState(StateEnum::Arming, ARM_MIN_PERIOD_MILLIS);
				LastWarningTimestamp = millis() - INT32_MAX; // Clear last warning weariness.
			}
			else
			{
				Task::disable();
			}
			break;
		case StateEnum::Arming:
			if (!InputReader->IsArmSignalOn())
			{
				UpdateState(StateEnum::NotArmed);
			}
			if (StateElapsed > ARM_MAX_WARMUP_PERIOD_MILLIS)
			{
				UpdateState(StateEnum::ArmingFailed);
			}
			else if (StateElapsed > ARM_MIN_PERIOD_MILLIS &&
				!MovementDetector->HasRecentSignificantMotion(ARM_MIN_PERIOD_MILLIS))
			{
				UpdateState(StateEnum::Armed);
			}
			else
			{
				Task::delay(ARMING_CHECK_PERIOD_MILLIS);
			}
			break;
		case StateEnum::ArmingFailed:
			if (!InputReader->IsArmSignalOn())
			{
				UpdateState(StateEnum::NotArmed);
			}
			else if (StateElapsed > REARM_WAIT_PERIOD_MILLIS)
			{
				UpdateState(StateEnum::Arming, REARM_WAIT_PERIOD_MILLIS);
			}
			else
			{
				Task::delay(ARMING_CHECK_PERIOD_MILLIS);
			}
			break;
		case StateEnum::Armed:
			if (!InputReader->IsArmSignalOn())
			{
				UpdateState(StateEnum::NotArmed);
			}
			else if (MovementDetector->HasRecentSignificantMotion(MOVEMENT_PERIOD_MILLIS))
			{
				if (millis() - LastWarningTimestamp > EARLY_WARNING_SKIP_MILLIS)
				{
					LastWarningTimestamp = millis();
					UpdateState(StateEnum::ArmingEarlyWarning, TRANSITION_GRACE_PERIOD_MILLIS);
				}
				else
				{
					UpdateState(StateEnum::Alarming);
				}
			}
			else
			{
				Task::disable();
			}
			break;
		case StateEnum::ArmingEarlyWarning:
			if (!InputReader->IsArmSignalOn())
			{
				UpdateState(StateEnum::NotArmed);
			}
			else if (StateElapsed > (TRANSITION_GRACE_PERIOD_MILLIS + MOVEMENT_PERIOD_MILLIS)
				&& MovementDetector->HasRecentSignificantMotion(MOVEMENT_PERIOD_MILLIS))
			{
				UpdateState(StateEnum::Alarming);
			}
			else if (StateElapsed > EARLY_WARNING_PERIOD_MILLIS)
			{
				UpdateState(StateEnum::EarlyWarning);
			}
			else
			{
				Task::delay(ARMING_CHECK_PERIOD_MILLIS);
			}
			break;
		case StateEnum::EarlyWarning:
			if (!InputReader->IsArmSignalOn())
			{
				UpdateState(StateEnum::NotArmed);
			}
			else if (MovementDetector->HasRecentSignificantMotion(MOVEMENT_PERIOD_MILLIS))
			{
				UpdateState(StateEnum::Alarming);
			}
			else
			{
				UpdateState(StateEnum::Armed);
			}
			break;
		case StateEnum::Alarming:
			if (!InputReader->IsArmSignalOn())
			{
				UpdateState(StateEnum::NotArmed);
			}
			else if (StateElapsed > ALARMING_DURATION_MILLIS)
			{
				LastWarningTimestamp = millis();
				UpdateState(StateEnum::ArmingEarlyWarning);
			}
			else
			{
				Task::delay(ALARMING_CHECK_PERIOD_MILLIS);
			}
			break;
		case StateEnum::Disabled:
		default:
			Task::disable();
			break;
		}

		return true;
	}

	void OnDisable()
	{
		if (Buzzer != nullptr)
		{
			Buzzer->Stop();
		}
	}

	bool OnEnable()
	{
		return State != StateEnum::Disabled;
	}
};
#endif