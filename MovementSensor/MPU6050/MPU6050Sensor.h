// MPU6050MPU6050::h

#ifndef _MPU6050SENSOR_h
#define _MPU6050SENSOR_h


// https://github.com/ElectronicCats/mpu6050
#include <MPU6050.h>
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif


class MPU6050Sensor : MPU6050
{
private:
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


	static const uint8_t MotionDetectionThreshold = 1;
	static const uint8_t MotionDetectionThresholdDuration = 1;


public:
	MPU6050Sensor(const int16_t xOffset,
		const int16_t yOffset,
		const int16_t zOffset,
		uint8_t address = MPU6050_DEFAULT_ADDRESS) :
		MPU6050(address),
		Calibration(xOffset, yOffset, zOffset)
	{
	}
	
	bool Setup()
	{
		if (MPU6050::testConnection())
		{
			MPU6050::reset();
			delay(30);

			MPU6050::setSleepEnabled(false);

			MPU6050::setClockSource(MPU6050_CLOCK_PLL_XGYRO);
			MPU6050::setFullScaleGyroRange(MPU6050_GYRO_FS_250);
			MPU6050::setFullScaleAccelRange(MPU6050_ACCEL_FS_2);

#if defined(DEBUG_LOG) && defined(DEBUG_SENSOR)
			// get MPU hardware revision
			MPU6050::setMemoryBank(0x10, true, true);
			MPU6050::setMemoryStartAddress(0x06);
			Serial.println(F("Checking hardware revision..."));
			Serial.print(F("Revision @ user[16][6] = "));
			Serial.println(MPU6050::readMemoryByte(), HEX);
			MPU6050::setMemoryStartAddress(0x00);
			MPU6050::setMemoryBank(0, false, false);
#endif

			MPU6050::setAccelerometerPowerOnDelay(3);

			// Disable unused features.
			MPU6050::setDMPEnabled(false);
			MPU6050::setFIFOEnabled(false);
			MPU6050::setTempSensorEnabled(false);
			MPU6050::setClockOutputEnabled(false);

			// Disable Gyro.
			MPU6050::setStandbyXGyroEnabled(true);
			MPU6050::setStandbyYGyroEnabled(true);
			MPU6050::setStandbyZGyroEnabled(true);

			// Reset and apply calibration.
			MPU6050::resetAccelerometerPath();
			if (Calibration.Provided)
			{
				MPU6050::setXAccelOffset(Calibration.xOffset);
				MPU6050::setYAccelOffset(Calibration.yOffset);
				MPU6050::setZAccelOffset(Calibration.zOffset);
			}

			// Interrupt pulses when triggered instead of remaining on until cleared.
			MPU6050::setInterruptLatch(false);

			// Disable unused interrupts.
			MPU6050::setIntFreefallEnabled(true);
			MPU6050::setIntZeroMotionEnabled(false);
			MPU6050::setIntDMPEnabled(false);
			MPU6050::setIntFIFOBufferOverflowEnabled(false);

			// Active - low, push-pull.
			MPU6050::setInterruptMode(true);
			MPU6050::setInterruptDrive(false);

			// Set thresholds.
			MPU6050::setMotionDetectionThreshold(MotionDetectionThreshold);
			MPU6050::setMotionDetectionDuration(MotionDetectionThresholdDuration);

			MPU6050::setZeroMotionDetectionThreshold(MotionDetectionThreshold);
			MPU6050::setZeroMotionDetectionDuration(MotionDetectionThresholdDuration);

			// Set sensor filter mode.
			MPU6050::setDHPFMode(MPU6050_DHPF_RESET);
			//MPU6050::setDLPFMode(MPU6050_DLPF_BW_256);

			// Enable motion detection interrupt.
			MPU6050::setIntMotionEnabled(true);

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

	void SetSleep()
	{
		MPU6050::setSleepEnabled(true);
	}

	void SetActiveMotionDetection()
	{
		MPU6050::setSleepEnabled(false);
		SetLowPowerMode();
	}

private:
	void SetLowPowerMode()
	{
		// Not working.
		//MPU6050::setWakeCycleEnabled(true);
		///**LP_WAKE_CTRL | Wake - up Frequency
		//	* ------------ - +------------------
		//	* 0 | 1.25 Hz
		//	* 1 | 2.5 Hz
		//	* 2 | 5 Hz
		//	* 3 | 10 Hz*/
		//MPU6050::setWakeFrequency(10);
	}


#if defined(DEBUG_LOG) && defined(DEBUG_SENSOR)
	void CheckSettings()
	{
		Serial.println();

		Serial.print(F(" * Sleep Mode:                "));
		Serial.println(MPU6050::getSleepEnabled() ? F("Enabled") : F("Disabled"));

		Serial.print(F(" * Motion Interrupt:     "));
		Serial.println(MPU6050::getIntMotionEnabled() ? F("Enabled") : F("Disabled"));

		Serial.print(F(" * Zero Motion Interrupt:     "));
		Serial.println(MPU6050::getIntZeroMotionEnabled() ? F("Enabled") : F("Disabled"));

		Serial.print(F(" * Free Fall Interrupt:       "));
		Serial.println(MPU6050::getIntFreefallEnabled() ? F("Enabled") : F("Disabled"));

		Serial.print(F(" * Motion Threshold:          "));
		Serial.println(MPU6050::getMotionDetectionThreshold());

		Serial.print(F(" * Motion Duration:           "));
		Serial.println(MPU6050::getMotionDetectionDuration());

		Serial.print(F(" * Zero Motion Threshold:     "));
		Serial.println(MPU6050::getZeroMotionDetectionThreshold());

		Serial.print(F(" * Zero Motion Duration:      "));
		Serial.println(MPU6050::getZeroMotionDetectionDuration());

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
		switch (MPU6050::getFullScaleAccelRange())
		{
		case 3:            Serial.println(F("+/- 16 g")); break;
		case 2:             Serial.println(F("+/- 8 g")); break;
		case 1:             Serial.println(F("+/- 4 g")); break;
		case 0:             Serial.println(F("+/- 2 g")); break;
		}

		Serial.print(F(" * Accelerometer offsets:     "));
		Serial.print(MPU6050::getXAccelOffset());
		Serial.print(F(" / "));
		Serial.print(MPU6050::getYAccelOffset());
		Serial.print(F(" / "));
		Serial.println(MPU6050::getZAccelOffset());

		Serial.print(F(" * Accelerometer power delay: "));
		switch (MPU6050::getAccelerometerPowerOnDelay())
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

