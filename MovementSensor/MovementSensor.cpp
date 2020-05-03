#include "MovementSensor.h"


MovementSensor* StaticMovementSensorReference = nullptr;

static void StaticOnSensorInterrupt()
{
	StaticMovementSensorReference->OnPinInterrupt();
}

void MovementSensor::AttachInterrupt()
{
	StaticMovementSensorReference = this;
	attachInterrupt(SensorInterruptPin, StaticOnSensorInterrupt, FALLING);
}