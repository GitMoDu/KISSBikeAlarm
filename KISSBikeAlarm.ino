/* KISS Bike Alarm
	Bike Alarm
		- Controlled by ignition key for arming.
		- Triggered by motion detection.

	Dependecies
		- Task Scheduler: https://github.com/arkhipenko/TaskScheduler
		- MPU650: https://github.com/ElectronicCats/mpu6050

	MCU
		- ATMega328P (3.3 V) @ 8 Mhz.

	External Hardware
		- MP56050.
		- Opto-isolator for input from ignition key.
	*/

	//#define DEBUG_LOG
	//#define DEBUG_STATE
	//#define DEBUG_SENSOR
	//#define WAIT_FOR_LOGGER


#define SERIAL_BAUD_RATE 115200



#define _TASK_OO_CALLBACKS
#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass.


#include <TaskScheduler.h>

#include "Buzzer\AlarmBuzzer.h"
#include "Light\AlarmLight.h"
#include "MovementSensor\MovementSensor.h"
#include "Input\InputReader.h"
#include "AlarmManager.h"

#include <Wire.h>


// Process scheduler.
Scheduler SchedulerBase;
//

// IIC Master.
// Statically declared in Wire.h
//TwoWire Wire;
//

// Buzzer task.
AlarmBuzzer Buzzer(&SchedulerBase, 11);
//

// Light task.
AlarmLight Light(&SchedulerBase, 7);
// 

// Input controls task.
InputReader Reader(&SchedulerBase, 2);
//

// IMU task, with offsets.
MovementSensor Sensor(&SchedulerBase, 3, -502, -185, 1162);
//

// Alarm task.
AlarmManager Manager(&SchedulerBase);
//


void setup()
{
#ifdef DEBUG_LOG
	Serial.begin(SERIAL_BAUD_RATE);
#endif
	if (!Buzzer.Setup())
	{
		return;
	}

	if (!Light.Setup())
	{
		return;
	}

	Wire.begin();
	Wire.setClock(400000);

#ifdef DEBUG_LOG
	Serial.println(F("Alarm Start."));
#endif

	if (!Reader.Setup(&Manager))
	{
		Buzzer.PlayError();
		return;
	}

	if (!Sensor.Setup(&Manager))
	{
		Buzzer.PlayError();
		return;
	}

	if (!Manager.Setup(&Buzzer, &Light, &Sensor, &Reader))
	{
		Buzzer.PlayError();
		return;
	}
}

void loop()
{
	SchedulerBase.execute();
}