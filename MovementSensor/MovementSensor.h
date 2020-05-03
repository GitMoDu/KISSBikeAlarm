// MovementSensor.h

#ifndef _MOVEMENTSENSOR_h
#define _MOVEMENTSENSOR_h

#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>


#include "..\IMovementSensor.h"
#include "..\Event\EventTask.h"
#include "MPU6050\MPU6050Sensor.h"

class MovementSensor : EventTask
	, public virtual IMovementSensor
{
private:
	const uint8_t SensorPin;
	const uint8_t SensorInterruptPin;

	uint32_t MotionLastTriggered = 0;

	enum StateEnum : uint8_t
	{
		Disabled,
		Active,
		MotionDetectionTriggered,
	};

	volatile StateEnum State = StateEnum::Disabled;

	MPU6050Sensor Sensor;

public:
	MovementSensor(Scheduler* scheduler, const uint8_t sensorPin,
		const int16_t xOffset,
		const int16_t yOffset,
		const int16_t zOffset)
		: EventTask(scheduler)
		, IMovementSensor()
		, SensorPin(sensorPin)
		, SensorInterruptPin(digitalPinToInterrupt(sensorPin))
		, Sensor(xOffset, yOffset, zOffset)

	{
		pinMode(SensorPin, INPUT);
	}

	virtual bool Setup(IEventListener* eventListener)
	{
		if (!EventTask::Setup(eventListener))
		{
			return false;
		}

		State = StateEnum::Disabled;

		if (SensorInterruptPin != NOT_AN_INTERRUPT)
		{
			detachInterrupt(SensorInterruptPin);

			return Sensor.Setup();
		}
		else
		{
			return false;
		}

	}

	virtual bool HasRecentSignificantMotion(const uint32_t period)
	{
		return millis() - MotionLastTriggered < period;
	}

	virtual void Enable()
	{
		if (State != StateEnum::Active)
		{
			detachInterrupt(SensorInterruptPin);
			State = StateEnum::Active;

			Task::enableIfNot();
			Task::forceNextIteration();
		}
	}

	virtual void Disable()
	{
		detachInterrupt(SensorInterruptPin);
		State = StateEnum::Disabled;
		Task::enableIfNot();
		Task::forceNextIteration();
	}

	bool Callback()
	{
		uint32_t Timestamp = millis();

		switch (State)
		{
		case StateEnum::Disabled:
			Task::disable();

			Sensor.SetSleep();
			break;
		case StateEnum::Active:
			Task::disable();
			Sensor.SetActiveMotionDetection();
			AttachInterrupt();
			break;
		case StateEnum::MotionDetectionTriggered:
			State = StateEnum::Active;
			Task::enableIfNot();
			Task::forceNextIteration();

			EventListener->OnEvent();
			break;
		default:
			Task::disable();
			break;
		}

		return true;
	}

	void OnPinInterrupt()
	{
		detachInterrupt(SensorInterruptPin);

		switch (State)
		{
		case StateEnum::Disabled:
			break;
		case StateEnum::Active:
			MotionLastTriggered = millis();
			State = StateEnum::MotionDetectionTriggered;
			Task::enableIfNot();
			Task::forceNextIteration();
			break;
		case StateEnum::MotionDetectionTriggered:
			MotionLastTriggered = millis();
			break;
		default:
			break;
		}
	}

private:
	void AttachInterrupt();
};
#endif