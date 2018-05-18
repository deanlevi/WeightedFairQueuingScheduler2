#include <sstream>

#include "Queue.h"
#include "Scheduler.h"

using namespace std;

list<ConnectionProperties> ConnectionList;

void ReadPacketsInCurrentTime(istream &InputFilePointer);
void HandleNewPacket(PacketProperties NewPacket, ConnectionProperties NewConnection);

void ReadPacketsInCurrentTime(istream &InputFilePointer) {
	string CurrentLine;
	PacketProperties NewPacket;
	ConnectionProperties NewConnection;
	int ParameterIndex = 0;
	while (getline(InputFilePointer, CurrentLine)) {
		NewPacket.InputLine = CurrentLine;
		stringstream LineStream(CurrentLine);
		while (LineStream) {
			switch (ParameterIndex)
			{
			case 0:
				LineStream >> NewPacket.Time;
				break;
			case 1:
				LineStream >> NewConnection.SAdd;
				break;
			case 2:
				LineStream >> NewConnection.SPort;
				break;
			case 3:
				LineStream >> NewConnection.Dadd;
				break;
			case 4:
				LineStream >> NewConnection.DPort;
				break;
			case 5:
				LineStream >> NewPacket.Length;
				break;
			default:
				break;
			}
			if (ParameterIndex == 6) {
				LineStream >> NewConnection.Weight;
				break;
			}
			ParameterIndex++;
		}
		HandleNewPacket(NewPacket, NewConnection);
		ParameterIndex = 0;
		if (NewPacket.Time > Scheduler.SystemTime) {
			break;
		}
	}
	if (NewPacket.Time <= Scheduler.SystemTime) { // meaning reached end of file
		Scheduler.FinishedReadingInputFile = true;
	}
}

void HandleNewPacket(PacketProperties NewPacket, ConnectionProperties NewConnection) {
	bool FoundConnection = false;
	list <ConnectionProperties> ::iterator ConnectionIterator;
	for (ConnectionIterator = ConnectionList.begin(); ConnectionIterator != ConnectionList.end(); ++ConnectionIterator) {
		if (ConnectionIterator->SAdd == NewConnection.SAdd && ConnectionIterator->SPort == NewConnection.SPort &&
			ConnectionIterator->Dadd == NewConnection.Dadd && ConnectionIterator->DPort == NewConnection.DPort) {
			FoundConnection = true;
			ConnectionIterator->Packets.push_back(NewPacket);
			if (NewConnection.Weight != 0) { // connection received new weight
				ConnectionIterator->Weight = NewConnection.Weight;
			}
			break;
		}
	}
	if (!FoundConnection) { // add new connection
		if (NewConnection.Weight == 0) { // if got no weight put default weight
			NewConnection.Weight = 1;
		}
		NewConnection.Packets.push_back(NewPacket);
		ConnectionList.push_back(NewConnection);
	}
	Scheduler.NumberOfPacketsInQueue += 1;
}