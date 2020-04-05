#define DEBUG_LOG
#define DEBUG_EVENT
#define DEBUG_STATE
#define WAIT_FOR_LOGGER


#define SERIAL_BAUD_RATE 115200



#define _TASK_OO_CALLBACKS
#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass.


#include <TaskScheduler.h>

#include "AlarmBuzzer.h"
#include "MovementSensor.h"
#include "InputReader.h"
#include "KissAlarmManager.h"


// Process scheduler.
Scheduler SchedulerBase;
//

// Buzzer task.
AlarmBuzzer Buzzer(&SchedulerBase, 11);
//

// Input controls task.
InputReader Reader(&SchedulerBase, 2);
//

// IMU task.
MovementSensor Sensor(&SchedulerBase, 3);
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

	if (!AlarmManager.Setup(&Buzzer, &Sensor, &Reader))
	{
		Buzzer.PlayError();
		return;
	}



#ifdef DEBUG_LOG
	Serial.println(F("Alarm Started."));
#endif
}

void loop()
{
	SchedulerBase.execute();
}

