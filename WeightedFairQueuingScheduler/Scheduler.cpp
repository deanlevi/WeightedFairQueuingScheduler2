#include <fstream>
#include <string>

#include "Scheduler.h"
#include "Queue.h"

using namespace std;

SchedulerProperties Scheduler;

void StartScheduler(char *argv[]);
void ChooseConnectionToOutput();

void StartScheduler(char *argv[]) {
	ofstream OutputFile; // todo remove
	OutputFile.open("MyOutputMedium.txt"); // todo remove
	OutputFile.close(); // todo remove

	ifstream InputFilePointer;
	InputFilePointer.open(argv[1]); // todo check if move to Scheduler struct
	if (!InputFilePointer.is_open()) {
		// todo put error
	}
	while (!Scheduler.FinishedReadingInputFile) {
		ReadPacketsInCurrentTime(InputFilePointer);
		ChooseConnectionToOutput();
		// todo calc packet to output
		// todo print packet to output
	}
	InputFilePointer.close();
	while (Scheduler.NumberOfPacketsInQueue > 0) {
		ChooseConnectionToOutput();
	}
}

void ChooseConnectionToOutput() {
	double FastestTimeToDeliverOnePacket;
	bool FirstConnectionUpdate = true, ChoseConnection = false, FirstMinimumArrivalTimeUpdate = true;
	list <ConnectionProperties> ::iterator ConnectionIterator, ChosenConnectionIterator;
	for (ConnectionIterator = ConnectionList.begin(); ConnectionIterator != ConnectionList.end(); ++ConnectionIterator) {
		if (ConnectionIterator->Packets.empty()) {
			continue;
		}
		// handle minimum arrival time in queue
		if (Scheduler.NeedToUpdateMinimumArrivalTimeInQueue) {
			if (FirstMinimumArrivalTimeUpdate) {
				Scheduler.MinimumArrivalTimeInQueue = ConnectionIterator->Packets.front().Time;
				Scheduler.NumberOfConnectionsWithPacketsWithMinimumArrivalTimeInQueue = 1;
				FirstMinimumArrivalTimeUpdate = false;
			}
			else {
				if (ConnectionIterator->Packets.front().Time == Scheduler.MinimumArrivalTimeInQueue) {
					Scheduler.NumberOfConnectionsWithPacketsWithMinimumArrivalTimeInQueue += 1;
				}
				if (ConnectionIterator->Packets.front().Time < Scheduler.MinimumArrivalTimeInQueue) {
					Scheduler.MinimumArrivalTimeInQueue = ConnectionIterator->Packets.front().Time;
					Scheduler.NumberOfConnectionsWithPacketsWithMinimumArrivalTimeInQueue = 1;
				}
			}
		}

		// handle chosen connection for output
		if (ConnectionIterator->Packets.front().Time > Scheduler.SystemTime) { // can't schedule future packets
			continue;
		}
		if (FirstConnectionUpdate) {
			FastestTimeToDeliverOnePacket = (double) ConnectionIterator->Packets.front().Length / ConnectionIterator->Weight;
			ChosenConnectionIterator = ConnectionIterator;
			FirstConnectionUpdate = false;
			ChoseConnection = true;
		}
		else {
			if ((ConnectionIterator->Packets.front().Length / ConnectionIterator->Weight) < FastestTimeToDeliverOnePacket) {
				FastestTimeToDeliverOnePacket = (double) ConnectionIterator->Packets.front().Length / ConnectionIterator->Weight;
				ChosenConnectionIterator = ConnectionIterator;
			}
		}
	}
	if (Scheduler.NeedToUpdateMinimumArrivalTimeInQueue && (Scheduler.MinimumArrivalTimeInQueue > Scheduler.SystemTime)) {
		Scheduler.SystemTime = Scheduler.MinimumArrivalTimeInQueue;
		Scheduler.NeedToUpdateMinimumArrivalTimeInQueue = false;
	}
	if (!ChoseConnection) {
		return;
	}
	PacketProperties ChosenPacket = ChosenConnectionIterator->Packets.front();
	//cout << "Time: " << Scheduler.SystemTime << ", Sadd: " << ChosenConnectionIterator->SAdd << ", Sport: " <<
		//ChosenConnectionIterator->SPort << ", Dadd: " << ChosenConnectionIterator->Dadd << ", Dport: " << ChosenConnectionIterator->DPort <<
		//", Length: " << ChosenPacket.Length << "\n"; // todo check print
	cout << Scheduler.SystemTime << ": " << ChosenPacket.InputLine << "\n";
	
	ofstream OutputFile; // todo remove
	OutputFile.open("MyOutputMedium.txt", ios::app); // todo remove
	OutputFile << Scheduler.SystemTime << ": " << ChosenPacket.InputLine << "\n"; // todo remove
	OutputFile.close(); // todo remove
	if (Scheduler.SystemTime == 312530) {
		int i = 1; // todo remove
	}


	Scheduler.SystemTime += ChosenPacket.Length;
	if (ChosenPacket.Time == Scheduler.MinimumArrivalTimeInQueue) {
		Scheduler.NumberOfConnectionsWithPacketsWithMinimumArrivalTimeInQueue -= 1;
		if (Scheduler.NumberOfConnectionsWithPacketsWithMinimumArrivalTimeInQueue == 0) {
			Scheduler.NeedToUpdateMinimumArrivalTimeInQueue = true;
		}
	}
	ChosenConnectionIterator->Packets.pop_front(); // delete selected packet
	Scheduler.NumberOfPacketsInQueue -= 1;
}