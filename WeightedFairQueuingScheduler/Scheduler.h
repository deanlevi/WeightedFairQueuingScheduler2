#pragma once

#include "Queue.h"

#define ERROR_CODE (int)(-1)

typedef struct _SchedulerProperties {
	long SystemTime = 0; // current time in system
	bool FinishedReadingInputFile = false;
	long NumberOfPacketsInQueue = 0;
	int MinimumArrivalTimeInQueue; // for updating system time
	int NumberOfConnectionsWithPacketsWithMinimumArrivalTimeInQueue; // optimization to reduce searches of min arrival time in system
	bool NeedToUpdateMinimumArrivalTimeInQueue = true;
	long double RoundTime = 0; // current round time
	long double RoundValue = 0; // current round value
	PacketProperties ExtraPacket; // extra packet when reading and exceeding system time
	ConnectionProperties ExtraConnection; // extra connection when reading and exceeding system time
	long double LastOfLeftPacket; // for updating round time and value when a packet has left
	bool FirstUpdateOfLastOfLeftPacket; // for updating round time and value when a packet has left
}SchedulerProperties;

extern SchedulerProperties Scheduler;
void HandleScheduler();