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

#include <Wire.h>
#include <avr/power.h>

#include "Buzzer\AlarmBuzzer.h"
#include "Light\AlarmLight.h"
#include "MovementSensor\MovementSensor.h"
#include "Input\InputReader.h"
#include "AlarmManager.h"




// Process scheduler.
Scheduler SchedulerBase;
//

// IIC Master.
// Statically declared in Wire.h
//TwoWire Wire;
//

// Buzzer task.
AlarmBuzzer Buzzer(&SchedulerBase, 9);
//

// Light task.
AlarmLight Light(&SchedulerBase, 5);
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

	Wire.begin();
	Wire.setClock(400000);

	SetupLowPower();


	if (!Buzzer.Setup())
	{
		SetupError();
	}

	if (!Light.Setup())
	{
		SetupError();
	}

	if (!Reader.Setup(&Manager))
	{
		SetupError();
	}

	if (!Sensor.Setup(&Manager))
	{
		SetupError();
	}

	if (!Manager.Setup(&Buzzer, &Light, &Sensor, &Reader))
	{
		SetupError();
	}

#ifdef DEBUG_LOG
	Serial.println(F("Alarm Start."));
#endif

}

void(*ResetFunc) (void) = 0; // Software eset function @ address 0.

void SetupError()
{
	pinMode(LED_BUILTIN, INPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	// Wait for 1 second and try again.
	delay(1000);
	ResetFunc();
}


void loop()
{
	SchedulerBase.execute();
}


void SetupLowPower()
{
#ifndef DEBUG_LOG
	power_usart0_disable();
#endif // DEBUG_LOG

	// Unused hardware.
	power_adc_disable();
	power_spi_disable();
	power_timer1_disable();
	power_timer2_disable();

	// Unused pins. Used pins are commented.
	pinMode(A0 , INPUT);
	pinMode(A1 , INPUT);
	pinMode(A2 , INPUT);
	pinMode(A3 , INPUT);
	//pinMode(A4 , INPUT);
	//pinMode(A5 , INPUT);
	pinMode(A6 , INPUT);
	pinMode(A7 , INPUT);

	pinMode(13, INPUT);
	pinMode(12, INPUT);
	pinMode(11, INPUT);
	pinMode(10, INPUT);
	//pinMode(9, INPUT);
	pinMode(8, INPUT);
	pinMode(7, INPUT);
	pinMode(6, INPUT);
	//pinMode(5, INPUT);
	pinMode(4, INPUT);
	//pinMode(3, INPUT);
	//pinMode(2, INPUT);

#ifndef DEBUG_LOG
	pinMode(1, INPUT);
	pinMode(0, INPUT);
#endif
}