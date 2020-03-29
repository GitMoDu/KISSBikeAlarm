// KissAlarmTask.h

#ifndef _KISSALARMMANAGER_h
#define _KISSALARMMANAGER_h

#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>

#include "IMovementSensor.h"
#include "IAlarmBuzzer.h"
#include "IEventListener.h"

class KissAlarmManager : Task, public virtual IEventListener
{
private:
	IAlarmBuzzer* Buzzer = nullptr;

	IMovementSensor* Sensor = nullptr;

	IInputReader* Reader = nullptr;

	enum StateEnum : uint8_t
	{
		Disabled,
		WakingUp,
		NotArmed,
		Arming,
		Armed,
		ArmingEarlyWarning,
		EarlyWarning,
		ArmingLastWarning,
		LastWarning,
		Alarming,
		StateCount
	};

	StateEnum State = StateEnum::Disabled;

	uint32_t StateStartedTimestamp = 0;

	const uint32_t TRANSITION_GRACE_PERIOD_MILLIS = 10000;

	const uint32_t ARM_DELAY_MILLIS = 5000; // 2 * 60 * 1000;
	const uint32_t ALARMING_DURATION_MILLIS = 5 * 60 * 1000;

	const uint32_t MIN_WAIT_PERIOD_MILLIS = 100;
	const uint32_t ARM_CHECK_PERIOD_MILLIS = 60000;
	const uint32_t ALARMING_CHECK_PERIOD_MILLIS = 1000;
	const uint32_t EARLY_WARNING_PERIOD_MILLIS = 10000;
	const uint32_t LAST_WARNING_PERIOD_MILLIS = 3000;

	// Runtime helper.
	uint32_t StateElapsed = 0;

public:
	KissAlarmManager(Scheduler* scheduler)
		: Task(0, TASK_FOREVER, scheduler, false)
		, IEventListener()
	{
	}

	bool Setup(IAlarmBuzzer* buzzer, IMovementSensor* movementSensor
		, IInputReader* inputReader)
	{
		bool Success = true;

		Buzzer = buzzer;
		Sensor = movementSensor;
		Reader = inputReader;

		if (Buzzer == nullptr ||
			Sensor == nullptr ||
			Reader == nullptr)
		{
			Success = false;
		}

		if (Success)
		{
			UpdateState(StateEnum::WakingUp, 1000);

			return true;
		}
		else
		{
			UpdateState(StateEnum::Disabled, 0);

			return false;
		}
	}

	virtual void OnEvent()
	{
#ifdef DEBUG_LOG
		Serial.print(millis());
		Serial.print(F(" - Event!! State("));
		Serial.print(millis() - StateStartedTimestamp);
		Serial.print(F("): "));
		Serial.println(State);
#endif

		switch (State)
		{
		case StateEnum::NotArmed:
		case StateEnum::Armed:
		case StateEnum::EarlyWarning:
		case StateEnum::LastWarning:
		case StateEnum::Alarming:
			enableIfNot();
			forceNextIteration();
			break;
		default:
			break;
		}
	}

	void UpdateState(StateEnum state, const uint32_t nextRunDelayMillis)
	{
		if (State != state)
		{
#ifdef DEBUG_LOG
			Serial.print(millis());
			Serial.print(F(" - Alarm State("));
			Serial.print(millis() - StateStartedTimestamp);
			Serial.print(F("): "));
			Serial.println(State);
#endif

			StateStartedTimestamp = millis();

			Buzzer->Stop();

			switch (state)
			{
			case StateEnum::Disabled:
				Sensor->Disable();
				Reader->Disable();
				Buzzer->PlayError();
				break;
			case StateEnum::WakingUp:
				Sensor->Disable();
				Reader->Disable();
				break;
			case StateEnum::NotArmed:
				Reader->ArmInterrupt();
				Buzzer->PlayNotArmed();
				break;
			case StateEnum::Arming:
				Reader->Disable();
				Sensor->Disable();
				Buzzer->PlayArmed();
				break;
			case StateEnum::Armed:
				Sensor->ArmInterrupt();
				Reader->ArmInterrupt();
				break;
			case StateEnum::ArmingEarlyWarning:
				Reader->Disable();
				Sensor->Disable();
				Buzzer->PlayEarlyWarning();
				break;
			case StateEnum::EarlyWarning:
				Sensor->ArmInterrupt();
				Reader->ArmInterrupt();
				break;
			case StateEnum::ArmingLastWarning:
				Reader->Disable();
				Sensor->Disable();
				Buzzer->PlayLastWarning();
				break;
			case StateEnum::LastWarning:
				Sensor->ArmInterrupt();
				Reader->ArmInterrupt();
				break;
			case StateEnum::Alarming:
				Sensor->ArmInterrupt();
				Reader->ArmInterrupt();
				Buzzer->PlayEarlyWarning();
				break;
			case StateEnum::StateCount:
				break;
			default:
				break;
			}

			State = state;


			enableIfNot();

			if (nextRunDelayMillis > 0)
			{
				Task::delay(nextRunDelayMillis);
			}
			else
			{
				forceNextIteration();
			}
		}
	}

	void PeripheralCheck()
	{
		if (!Sensor->IsDeviceReady())
		{
			// TODO: Add to log, disabled I2C device.
		}
	}

	bool Callback()
	{
		StateElapsed = millis() - StateStartedTimestamp;

		switch (State)
		{
		case StateEnum::WakingUp:
			UpdateState(StateEnum::NotArmed, 0);
			break;
		case StateEnum::NotArmed:
			if (Reader->IsArmSignalOn())
			{
				UpdateState(StateEnum::Arming, ARM_DELAY_MILLIS);
			}
			else
			{
				Task::delay(ARM_CHECK_PERIOD_MILLIS);
			}
			break;
		case StateEnum::Arming:
			if (!Reader->IsArmSignalOn())
			{
				UpdateState(StateEnum::NotArmed, 0);
			}
			else if (StateElapsed > ARM_DELAY_MILLIS)
			{
				UpdateState(StateEnum::Armed, 0);
			}
			else
			{
				Task::delay(MIN_WAIT_PERIOD_MILLIS);
			}
			break;
		case StateEnum::Armed:
			if (!Reader->IsArmSignalOn())
			{
				UpdateState(StateEnum::NotArmed, 0);
			}
			else if (Sensor->HasRecentSignificantMotion())
			{
				UpdateState(StateEnum::ArmingEarlyWarning, TRANSITION_GRACE_PERIOD_MILLIS);
			}
			else
			{
				//TODO: Enter ultra-low power mode.
				Task::delay(ARM_CHECK_PERIOD_MILLIS);
			}
			break;
		case StateEnum::ArmingEarlyWarning:
			if (!Reader->IsArmSignalOn())
			{
				UpdateState(StateEnum::NotArmed, 0);
			}
			else if (StateElapsed > TRANSITION_GRACE_PERIOD_MILLIS)
			{
				UpdateState(StateEnum::EarlyWarning, 0);
			}
			else
			{
				Task::delay(MIN_WAIT_PERIOD_MILLIS);
			}
			break;
		case StateEnum::EarlyWarning:
			if (!Reader->IsArmSignalOn())
			{
				UpdateState(StateEnum::NotArmed, 0);
			}
			else if (!Sensor->HasRecentSignificantMotion() && StateElapsed > EARLY_WARNING_PERIOD_MILLIS)
			{
				UpdateState(StateEnum::NotArmed, 0);
			}
			else if (Sensor->HasRecentSignificantMotion()) //TODO:
			{
				UpdateState(StateEnum::ArmingLastWarning, TRANSITION_GRACE_PERIOD_MILLIS);
			}
			else
			{
				Task::delay(MIN_WAIT_PERIOD_MILLIS);
			}
			break;
		case StateEnum::ArmingLastWarning:
			if (!Reader->IsArmSignalOn())
			{
				UpdateState(StateEnum::NotArmed, 0);
			}
			else if (StateElapsed > TRANSITION_GRACE_PERIOD_MILLIS)
			{
				UpdateState(StateEnum::LastWarning, 0);
			}
			else
			{
				Task::delay(MIN_WAIT_PERIOD_MILLIS);
			}
			break;
		case StateEnum::LastWarning:
			if (!Reader->IsArmSignalOn())
			{
				UpdateState(StateEnum::NotArmed, 0);
			}
			else if (!Sensor->HasRecentSignificantMotion() && StateElapsed > EARLY_WARNING_PERIOD_MILLIS)
			{
				UpdateState(StateEnum::NotArmed, 0);
			}
			else if (Sensor->HasRecentSignificantMotion())
			{
				UpdateState(StateEnum::Alarming, 0);
			}
			else
			{
				Task::delay(MIN_WAIT_PERIOD_MILLIS);
			}
			break;
		case StateEnum::Alarming:
			if (!Reader->IsArmSignalOn())
			{
				UpdateState(StateEnum::NotArmed, 0);
			}
			else if (StateElapsed > ALARMING_DURATION_MILLIS)
			{
				UpdateState(StateEnum::ArmingLastWarning, TRANSITION_GRACE_PERIOD_MILLIS);
			}
			else
			{
				Task::delay(ALARMING_CHECK_PERIOD_MILLIS);
			}
			break;
		case StateEnum::Disabled:
		default:
			disable();
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

		if (Sensor != nullptr)
		{
			Sensor->Disable();
		}

		if (Reader != nullptr)
		{
			Reader->Disable();
		}
	}

	bool OnEnable()
	{
		return State != StateEnum::Disabled;
	}
};
#endif