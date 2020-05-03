// AlarmConstants.h

#ifndef _ALARMCONSTANTS_h
#define _ALARMCONSTANTS_h

#include <stdint.h>

static const uint32_t MOVEMENT_PERIOD_MILLIS = 600;
static const uint32_t TRANSITION_GRACE_PERIOD_MILLIS = 2000;

static const uint32_t ARM_PERIOD_MILLIS = 8000;

static const uint32_t REARM_WAIT_PERIOD_MILLIS = 2000;

static const uint32_t ALARMING_DURATION_MILLIS = 5000; //3 * 60 * 1000;

static const uint32_t MIN_RUN_PERIOD_MILLIS = 2;
static const uint32_t EARLY_WARNING_PERIOD_MILLIS = 1000 + MOVEMENT_PERIOD_MILLIS + TRANSITION_GRACE_PERIOD_MILLIS;
static const uint32_t EARLY_WARNING_SKIP_MILLIS = 3000 + EARLY_WARNING_PERIOD_MILLIS;

// Output Constants.
static const uint32_t ARMED_FLASH_PERIOD_MILLIS = 2500;
#endif