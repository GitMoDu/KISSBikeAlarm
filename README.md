# KissBikeAlarm

Bike Alarm
		- Controlled by ignition key for arming.
		- Triggered by motion detection.

	Dependecies
		- Task Scheduler: https://github.com/arkhipenko/TaskScheduler
		- TimerOne: https://github.com/PaulStoffregen/TimerOne
		- Light WS2812: https://github.com/cpldcpu/light_ws2812
		- Accelerometer: https://github.com/ElectronicCats/mpu6050

	MCU
		- ATMega328P (3.3 V) @ 8 Mhz.


Schematic
![](https://raw.githubusercontent.com/GitMoDu/KISSBikeAlarm/master/Media/Schematic.png)

Generic step-down power supply (3.3 V).

Opto-isolator 4N35 for input from ignition key. 

Movement sensor.