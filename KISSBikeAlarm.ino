/* KISS Bike Alarm
	Bike Alarm
		- Controlled by ignition key for arming.
		- Triggered by motion detection.

	Dependecies
		- MPU650: https://github.com/ElectronicCats/mpu6050

	MCU
		- ATMega328P (3.3 V) @ 8 Mhz.

	External Hardware
		- MP56050.
		- Opto-isolator for input from ignition key.
		- Step-down power supply (3.3 V).
	*/

#define DEBUG_LOG
#define DEBUG_STATE
#define DEBUG_SENSOR
#define WAIT_FOR_LOGGER


#define SERIAL_BAUD_RATE 115200



#define _TASK_OO_CALLBACKS
#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass.


#include <TaskScheduler.h>

#include "AlarmBuzzer.h"
#include "AlarmLight.h"
#include "MovementSensor.h"
#include "InputReader.h"
#include "KissAlarmManager.h"

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
AlarmLight Light(&SchedulerBase, LED_BUILTIN);
// 

// Input controls task.
InputReader Reader(&SchedulerBase, 2);
//

// IMU task, with offsets.
MovementSensor Sensor(&SchedulerBase, 3, -502, -185, 1162);
//

// Alarm task.
KissAlarmManager AlarmManager(&SchedulerBase);
//


void setup()
{
#ifdef DEBUG_LOG
	Serial.begin(SERIAL_BAUD_RATE);
#endif
	if (!Buzzer.Setup())
	{
		while (true);;
	}

	Wire.begin();
	Wire.setClock(400000);

#ifdef DEBUG_LOG
	Serial.println(F("Alarm Start."));
#endif

	if (!Reader.Setup(&AlarmManager))
	{
		Buzzer.PlayError();
		return;
	}

	if (!Sensor.Setup(&AlarmManager))
	{
		Buzzer.PlayError();
		return;
	}

	if (!AlarmManager.Setup(&Buzzer, &Light, &Sensor, &Reader))
	{
		Buzzer.PlayError();
		return;
	}
}

void loop()
{
	SchedulerBase.execute();
}