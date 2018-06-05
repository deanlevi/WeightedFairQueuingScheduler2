#include <fstream>
#include <string>

#include "Scheduler.h"
#include "Queue.h"

using namespace std;

SchedulerProperties Scheduler; // keeps needed parameters to handle the scheduler

/*
Input: none.
Output: none.
Description: handles the operation of the scheduler. reads from input untill have packets with time <= system time
			 and outputs packets untill needs to read more. when finish reading outputs all left packets.
			 has the complexity of the entire program =
			 O((num of existing connections + complexity of UpdateRound) * total num of packets)
			 =~ O(total num of waiting packets per packet that we read * total num of packets)
			 =~ O(total num of packets ^ 2)
*/
void HandleScheduler();

/*
Input: none.
Output: none.
Description: chooses connection to output and timing it's first packet in queue by iterating over all connections and
			 checking which one has the most recent Last parameter.
			 has complexity of O(num of connections).
*/
void ChooseConnectionToOutput();

void HandleScheduler() {
	Scheduler.ExtraPacket.Time = -1; // mark that not valid
	while (!Scheduler.FinishedReadingInputFile) {
		if (Scheduler.ExtraPacket.Time <= Scheduler.SystemTime || Scheduler.NumberOfPacketsInQueue == 0) {
			if (Scheduler.ExtraPacket.Time >= 0) { // if not first read and have extra packet from last read
				HandleNewPacket(&Scheduler.ExtraPacket, Scheduler.ExtraConnection);
			}
			ReadPacketsInCurrentTime();
		}
		ChooseConnectionToOutput();
	}
	while (Scheduler.NumberOfPacketsInQueue > 0) {
		ChooseConnectionToOutput();
	}
}

void ChooseConnectionToOutput() {
	long double FastestTimeToDeliverOnePacket; // result of GPS simulation
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
		if (ConnectionIterator->Packets.front().Time > Scheduler.SystemTime) { // can't schedule future packets.
			continue;
		}
		if (FirstConnectionUpdate) {
			FastestTimeToDeliverOnePacket = ConnectionIterator->Packets.front().Last;
			ChosenConnectionIterator = ConnectionIterator;
			FirstConnectionUpdate = false;
			ChoseConnection = true;
		}
		else {
			if (ConnectionIterator->Packets.front().Last < FastestTimeToDeliverOnePacket) {
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
	cout << Scheduler.SystemTime << ": " << ChosenPacket.InputLine << "\n";
	
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