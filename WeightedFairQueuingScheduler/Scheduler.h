#pragma once

#define ERROR_CODE (int)(-1) // todo check

typedef struct _SchedulerProperties {
	int SystemTime = 0; // todo verify
	bool FinishedReadingInputFile = false;
}SchedulerProperties;

//extern SchedulerProperties Scheduler; // todo check
void StartScheduler(char *argv[]);