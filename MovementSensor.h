// MovementSensor.h

#ifndef _MOVEMENTSENSOR_h
#define _MOVEMENTSENSOR_h

#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>

// https://github.com/ElectronicCats/mpu6050
#include <MPU6050.h>
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

#include "EventTask.h"

#include "IMovementSensor.h"

class MovementSensor : EventTask
	, public virtual IMovementSensor
{
private:
	const uint8_t SensorPin;
	const uint8_t SensorInterruptPin;


	const uint8_t MotionDetectionThreshold = 1;
	const uint8_t MotionDetectionThresholdDuration = 1;

	uint32_t MotionLastTriggered = 0;

	const struct CalibrationProvider
	{
		const int16_t xOffset;
		const int16_t yOffset;
		const int16_t zOffset;
		const bool Provided;

		CalibrationProvider() :
			xOffset(0), yOffset(0), zOffset(0),
			Provided(false)
		{}

		CalibrationProvider(const int16_t x, const int16_t y, const int16_t z) :
			xOffset(x), yOffset(y), zOffset(z),
			Provided(true)
		{}
	} Calibration;

	enum StateEnum : uint8_t
	{
		Disabled,
		Active,
		MotionDetectionTriggered,
	};

	volatile StateEnum State = StateEnum::Disabled;

	MPU6050 Sensor;

public:
	MovementSensor(Scheduler* scheduler, const uint8_t sensorPin,
		const int16_t xOffset,
		const int16_t yOffset,
		const int16_t zOffset)
		: EventTask(scheduler)
		, IMovementSensor()
		, SensorPin(sensorPin)
		, SensorInterruptPin(digitalPinToInterrupt(sensorPin))
		, Calibration(xOffset, yOffset, zOffset)
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

			return SetupSensor();
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
			disable();

			Sensor.setSleepEnabled(true);
			break;
		case StateEnum::Active:
			disable();

			Sensor.setSleepEnabled(false);
			SetThresholds();
			SetLowPowerMode();
			
			AttachInterrupt();
			break;
		case StateEnum::MotionDetectionTriggered:
			State = StateEnum::Active;
			Task::enableIfNot();
			Task::forceNextIteration();

			EventListener->OnEvent();
			break;
		default:
			disable();
			break;
		}

		return true;
	}

	void OnPinInterrupt()
	{
		switch (State)
		{
		case StateEnum::Disabled:
			detachInterrupt(SensorInterruptPin);
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

	bool SetupSensor()
	{
		if (Sensor.testConnection())
		{
			Sensor.reset();
			delay(30);

			Sensor.setClockSource(MPU6050_CLOCK_PLL_XGYRO);
			Sensor.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
			Sensor.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);

#if defined(DEBUG_LOG) && defined(DEBUG_SENSOR)
			// get MPU hardware revision
			Sensor.setMemoryBank(0x10, true, true);
			Sensor.setMemoryStartAddress(0x06);
			Serial.println(F("Checking hardware revision..."));
			Serial.print(F("Revision @ user[16][6] = "));
			Serial.println(Sensor.readMemoryByte(), HEX);
			Sensor.setMemoryStartAddress(0x00);
			Sensor.setMemoryBank(0, false, false);
#endif

			Sensor.setAccelerometerPowerOnDelay(3);

			// Disable unused features.
			Sensor.setDMPEnabled(false);
			Sensor.setFIFOEnabled(false);
			Sensor.setTempSensorEnabled(false);
			Sensor.setClockOutputEnabled(false);

			// Disable Gyro.
			Sensor.setStandbyXGyroEnabled(true);
			Sensor.setStandbyYGyroEnabled(true);
			Sensor.setStandbyZGyroEnabled(true);

			// Reset and apply calibration.
			Sensor.resetAccelerometerPath();
			if (Calibration.Provided)
			{
				Sensor.setXAccelOffset(Calibration.xOffset);
				Sensor.setYAccelOffset(Calibration.yOffset);
				Sensor.setZAccelOffset(Calibration.zOffset);
			}

			// Interrupt pulses when triggered instead of remaining on until cleared.
			Sensor.setInterruptLatch(false);

			// Disable unused interrupts.
			Sensor.setIntFreefallEnabled(false);
			Sensor.setIntZeroMotionEnabled(false);
			Sensor.setIntDMPEnabled(false);
			Sensor.setIntFIFOBufferOverflowEnabled(false);

			// Active - low, push-pull.
			Sensor.setInterruptMode(true);
			Sensor.setInterruptDrive(false);

			// Set thresholds.
			SetThresholds();

			// Set sensor filter mode.
			Sensor.setDHPFMode(MPU6050_DHPF_RESET);
			//Sensor.setDLPFMode(MPU6050_DLPF_BW_256);

			// Enable motion detection interrupt.
			Sensor.setIntMotionEnabled(true);

#if defined(DEBUG_LOG) && defined(DEBUG_SENSOR)
			CheckSettings();
#endif
			return true;
		}
		else
		{
			return false;
		}
	}

	void SetThresholds()
	{
		Sensor.setMotionDetectionThreshold(MotionDetectionThreshold);
		Sensor.setMotionDetectionDuration(MotionDetectionThresholdDuration);
	}

	void SetLowPowerMode()
	{
		// Not working.
		//Sensor.setWakeCycleEnabled(true);
		///**LP_WAKE_CTRL | Wake - up Frequency
		//	* ------------ - +------------------
		//	* 0 | 1.25 Hz
		//	* 1 | 2.5 Hz
		//	* 2 | 5 Hz
		//	* 3 | 10 Hz*/
		//Sensor.setWakeFrequency(10);
	}

#if defined(DEBUG_LOG) && defined(DEBUG_SENSOR)
	void CheckSettings()
	{
		Serial.println();

		Serial.print(F(" * Sleep Mode:                "));
		Serial.println(Sensor.getSleepEnabled() ? F("Enabled") : F("Disabled"));

		Serial.print(F(" * Motion Interrupt:     "));
		Serial.println(Sensor.getIntMotionEnabled() ? F("Enabled") : F("Disabled"));

		Serial.print(F(" * Zero Motion Interrupt:     "));
		Serial.println(Sensor.getIntZeroMotionEnabled() ? F("Enabled") : F("Disabled"));

		Serial.print(F(" * Free Fall Interrupt:       "));
		Serial.println(Sensor.getIntFreefallEnabled() ? F("Enabled") : F("Disabled"));

		Serial.print(F(" * Motion Threshold:          "));
		Serial.println(Sensor.getMotionDetectionThreshold());

		Serial.print(F(" * Motion Duration:           "));
		Serial.println(Sensor.getMotionDetectionDuration());

		Serial.print(F(" * Zero Motion Threshold:     "));
		Serial.println(Sensor.getZeroMotionDetectionThreshold());

		Serial.print(F(" * Zero Motion Duration:      "));
		Serial.println(Sensor.getZeroMotionDetectionDuration());

		/*	Serial.print(" * Clock Source:              ");
			switch (getClockSource())
			{
			case MPU6050_CLOCK_KEEP_RESET:     Serial.println("Stops the clock and keeps the timing generator in reset"); break;
			case MPU6050_CLOCK_PLL_EXT19M: Serial.println("PLL with external 19.2MHz reference"); break;
			case MPU6050_CLOCK_PLL_EXT32K: Serial.println("PLL with external 32.768kHz reference"); break;
			case MPU6050_CLOCK_PLL_ZGYRO:      Serial.println("PLL with Z axis gyroscope reference"); break;
			case MPU6050_CLOCK_PLL_YGYRO:      Serial.println("PLL with Y axis gyroscope reference"); break;
			case MPU6050_CLOCK_PLL_XGYRO:      Serial.println("PLL with X axis gyroscope reference"); break;
			case MPU6050_CLOCK_INTERNAL:  Serial.println("Internal 8MHz oscillator"); break;
			}*/

		Serial.print(F(" * Accelerometer:             "));
		switch (Sensor.getFullScaleAccelRange())
		{
		case 3:            Serial.println(F("+/- 16 g")); break;
		case 2:             Serial.println(F("+/- 8 g")); break;
		case 1:             Serial.println(F("+/- 4 g")); break;
		case 0:             Serial.println(F("+/- 2 g")); break;
		}

		Serial.print(F(" * Accelerometer offsets:     "));
		Serial.print(Sensor.getXAccelOffset());
		Serial.print(F(" / "));
		Serial.print(Sensor.getYAccelOffset());
		Serial.print(F(" / "));
		Serial.println(Sensor.getZAccelOffset());

		Serial.print(F(" * Accelerometer power delay: "));
		switch (Sensor.getAccelerometerPowerOnDelay())
		{
		case 3:            Serial.println(F("3ms")); break;
		case 2:            Serial.println(F("2ms")); break;
		case 1:            Serial.println(F("1ms")); break;
		case 0:             Serial.println(F("0ms")); break;
		}

		Serial.println();
	}
#endif
};
#endif