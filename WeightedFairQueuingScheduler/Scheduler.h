#pragma once

#define ERROR_CODE (int)(-1) // todo check

typedef struct _SchedulerProperties {
	long SystemTime = 0; // todo verify
	bool FinishedReadingInputFile = false;
	long NumberOfPacketsInQueue = 0;
	int MinimumArrivalTimeInQueue;
	int NumberOfConnectionsWithPacketsWithMinimumArrivalTimeInQueue;
	bool NeedToUpdateMinimumArrivalTimeInQueue = true;
}SchedulerProperties;

extern SchedulerProperties Scheduler;
void StartScheduler(char *argv[]);