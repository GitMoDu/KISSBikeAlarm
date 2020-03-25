#define DEBUG_LOG
#define WAIT_FOR_LOGGER


#define SERIAL_BAUD_RATE 9600



#define _TASK_OO_CALLBACKS
#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass.


#include <TaskScheduler.h>

#include "AlarmBuzzer.h"

#include "KissAlarmManager.h"


// Process scheduler.
Scheduler SchedulerBase;
//

// Buzzer task.
AlarmBuzzer<13> Buzzer(&SchedulerBase);
//

// Alarm task.
KissAlarmManager AlarmManager(&SchedulerBase, &Buzzer);
//


void setup()
{

}

void loop()
{

}

