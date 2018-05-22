#pragma once

#include "Queue.h"

#define ERROR_CODE (int)(-1) // todo check

typedef struct _SchedulerProperties {
	long SystemTime = 0;
	bool FinishedReadingInputFile = false;
	long NumberOfPacketsInQueue = 0;
	int MinimumArrivalTimeInQueue;
	int NumberOfConnectionsWithPacketsWithMinimumArrivalTimeInQueue;
	bool NeedToUpdateMinimumArrivalTimeInQueue = true;
	long RoundTime = 0;
	long double RoundValue = 0;
	PacketProperties ExtraPacket;
	ConnectionProperties ExtraConnection;
	long TotalDepartures = 0;
	//int WeightedSumOfActiveLinks = 1; // todo check

}SchedulerProperties;

extern SchedulerProperties Scheduler;
void StartScheduler(char *argv[]);