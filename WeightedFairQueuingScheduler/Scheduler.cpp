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
	Scheduler.ExtraPacket.Time = -1; // mark that not valid
	while (!Scheduler.FinishedReadingInputFile) {
		if (Scheduler.ExtraPacket.Time <= Scheduler.SystemTime || Scheduler.NumberOfPacketsInQueue == 0) {
			if (Scheduler.ExtraPacket.Time >= 0) {
				HandleNewPacket(&Scheduler.ExtraPacket, Scheduler.ExtraConnection);
			}
			ReadPacketsInCurrentTime(InputFilePointer);
		}
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
	long double FastestTimeToDeliverOnePacket;
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
			//FastestTimeToDeliverOnePacket = (double) ConnectionIterator->Packets.front().Length / ConnectionIterator->Weight; // todo
			FastestTimeToDeliverOnePacket = ConnectionIterator->Packets.front().Last;
			ChosenConnectionIterator = ConnectionIterator;
			FirstConnectionUpdate = false;
			ChoseConnection = true;
		}
		else {
			//if ((ConnectionIterator->Packets.front().Length / ConnectionIterator->Weight) < FastestTimeToDeliverOnePacket) {
			if (ConnectionIterator->Packets.front().Last < FastestTimeToDeliverOnePacket) {
				//FastestTimeToDeliverOnePacket = (double) ConnectionIterator->Packets.front().Length / ConnectionIterator->Weight; // todo
				FastestTimeToDeliverOnePacket = ConnectionIterator->Packets.front().Last;
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
	if (Scheduler.SystemTime == 313552) { // todo remove
		int i = 1; // todo remove
	} // todo remove


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