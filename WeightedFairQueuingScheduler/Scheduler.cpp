#include <fstream>
#include <sstream>
#include <string>

#include "Scheduler.h"
#include "Queue.h"

using namespace std;

SchedulerProperties Scheduler;

void StartScheduler(char *argv[]);
void ReadPacketsInCurrentTime(istream &InputFilePointer);

void StartScheduler(char *argv[]) {
	ifstream InputFilePointer;
	InputFilePointer.open(argv[1]); // todo check if move to Scheduler struct
	if (!InputFilePointer.is_open()) {
		// todo put error
	}
	while (!Scheduler.FinishedReadingInputFile) {
		ReadPacketsInCurrentTime(InputFilePointer);
		// todo calc packet to output
		// todo print packet to output
	}
}

void ReadPacketsInCurrentTime(istream &InputFilePointer) {
	string CurrentLine;
	QueueProperties NewPacketToInsert;
	int ParameterIndex = 0;
	while (getline(InputFilePointer, CurrentLine)) {
		stringstream LineStream(CurrentLine);
		while (LineStream) {
			switch (ParameterIndex)
			{
			case 0:
				LineStream >> NewPacketToInsert.Time;
				break;
			case 1:
				LineStream >> NewPacketToInsert.SAdd;
				break;
			case 2:
				LineStream >> NewPacketToInsert.SPort;
				break;
			case 3:
				LineStream >> NewPacketToInsert.Dadd;
				break;
			case 4:
				LineStream >> NewPacketToInsert.DPort;
				break;
			case 5:
				LineStream >> NewPacketToInsert.Length;
				break;
			case 6:
				LineStream >> NewPacketToInsert.Weight; // will be updated if exists
				break;
			default:
				LineStream >> ParameterIndex; // just so LineStream will finish
				break;
			}
			ParameterIndex++;
		}
		Queue.push_back(NewPacketToInsert);
		ParameterIndex = 0;
		if (NewPacketToInsert.Time > Scheduler.SystemTime) {
			break;
		}
	}
	if (NewPacketToInsert.Time <= Scheduler.SystemTime) { // meaning reached end of file
		Scheduler.FinishedReadingInputFile = true;
	}
}