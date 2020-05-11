// AlarmManager.h

#ifndef _ALARMMANAGER_h
#define _ALARMMANAGER_h

#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>

#include "IMovementSensor.h"
#include "IAlarmOutput.h"
#include "IEventListener.h"

#include "AlarmConstants.h"


class AlarmManager : Task, public virtual IEventListener
{
private:
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
	AlarmManager(Scheduler* scheduler)
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
				InputReader->Enable();

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

				Light->PlayEarlyWarning();
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
				UpdateState(StateEnum::Arming);
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
			else if (StateElapsed >= ARM_PERIOD_MILLIS)
			{
				if (MovementDetector->HasRecentSignificantMotion(ARM_PERIOD_MILLIS - TRANSITION_GRACE_PERIOD_MILLIS))
				{
					UpdateState(StateEnum::ArmingFailed);
				}
				else
				{
					UpdateState(StateEnum::Armed);
				}
			}
			else
			{
				Task::delay(ARM_PERIOD_MILLIS - StateElapsed);
			}
			break;
		case StateEnum::ArmingFailed:
			if (!InputReader->IsArmSignalOn())
			{
				UpdateState(StateEnum::NotArmed);
			}
			else if (StateElapsed > REARM_WAIT_PERIOD_MILLIS)
			{
				UpdateState(StateEnum::Arming);
			}
			else
			{
				Task::delay(REARM_WAIT_PERIOD_MILLIS - StateElapsed);
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
				Task::delay(EARLY_WARNING_PERIOD_MILLIS - StateElapsed);
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
				Task::delay(ALARMING_DURATION_MILLIS - StateElapsed);
			}
			break;
		case StateEnum::Disabled:
			Task::disable();
			break;
		default:
			Task::disable();
			break;
		}

		return true;
	}

	bool OnEnable()
	{
		return State != StateEnum::Disabled;
	}
};
#endif