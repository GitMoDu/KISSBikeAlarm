# KissBikeAlarm

Arduino based, simple movement detection alarm.

	Bike Alarm
		- Controlled by ignition key for arming.
		- Triggered by motion detection.

	Dependecies
		- MPU650: https://github.com/ElectronicCats/mpu6050. Only used for testing, to be replaced with accelerometer library.

	MCU
		- ATMega328P (3.3 V) @ 8 Mhz.

	External Hardware
		- MP56050 (Only used for testing).
		- Opto-isolator 4N35 for input from ignition key. 
		- Generic step-down power supply (3.3 V).

Schematic
![](https://raw.githubusercontent.com/GitMoDu/KISSBikeAlarm/master/Media/Schematic.png)
