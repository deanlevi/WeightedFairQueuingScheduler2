#include <sstream>

#include "Queue.h"
#include "Scheduler.h"

using namespace std;

list<ConnectionProperties> ConnectionList;

void ReadPacketsInCurrentTime(istream &InputFilePointer);
void HandleNewPacket(PacketProperties *NewPacket, ConnectionProperties NewConnection);
void UpdateRound(PacketProperties *NewPacket, int Weight);
int CalcArrivals(list <ConnectionProperties> ::iterator ConnectionIterator);
int CalcDepartures(list <ConnectionProperties> ::iterator ConnectionIterator);
void CalcLastForPacketInExistingConnection(PacketProperties *NewPacket, list <ConnectionProperties> ::iterator ConnectionIterator);
void CalcLastForPacketInNewConnection(PacketProperties *NewPacket, ConnectionProperties NewConnection);

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
		if (NewPacket.Time > Scheduler.SystemTime) {
			break;
		}
		HandleNewPacket(&NewPacket, NewConnection);
		ParameterIndex = 0;
	}
	if (NewPacket.Time <= Scheduler.SystemTime) { // meaning reached end of file
		Scheduler.FinishedReadingInputFile = true;
	}
	else {
		Scheduler.ExtraPacket = NewPacket;
		Scheduler.ExtraConnection = NewConnection;
	}
}

void HandleNewPacket(PacketProperties *NewPacket, ConnectionProperties NewConnection) {
	bool FoundConnection = false;
	list <ConnectionProperties> ::iterator ConnectionIterator;
	for (ConnectionIterator = ConnectionList.begin(); ConnectionIterator != ConnectionList.end(); ++ConnectionIterator) {
		if (ConnectionIterator->SAdd == NewConnection.SAdd && ConnectionIterator->SPort == NewConnection.SPort &&
			ConnectionIterator->Dadd == NewConnection.Dadd && ConnectionIterator->DPort == NewConnection.DPort) {
			FoundConnection = true;
			/*if (!ConnectionIterator->IsLinkActive) {
				ConnectionIterator->IsLinkActive = true;
				Scheduler.WeightedSumOfActiveLinks += ConnectionIterator->Weight;
			}*/ // todo remove
			if (NewConnection.Weight != 0) { // connection received new weight
				ConnectionIterator->Weight = NewConnection.Weight;
			}
			UpdateRound(NewPacket, ConnectionIterator->Weight);
			CalcLastForPacketInExistingConnection(NewPacket, ConnectionIterator);
			ConnectionIterator->Packets.push_back(*NewPacket);
			break;
		}
	}
	if (!FoundConnection) { // add new connection
		if (NewConnection.Weight == 0) { // if got no weight put default weight
			NewConnection.Weight = 1;
		}
		UpdateRound(NewPacket, NewConnection.Weight);
		CalcLastForPacketInNewConnection(NewPacket, NewConnection);

		NewConnection.Arrivals.push_back(Scheduler.RoundValue);
		NewConnection.Departures.push_back(NewPacket->Last);
		Scheduler.TotalDepartures += 1;
		NewConnection.Packets.push_back(*NewPacket);
		ConnectionList.push_back(NewConnection);
	}
	Scheduler.NumberOfPacketsInQueue += 1;
}

void UpdateRound(PacketProperties *NewPacket, int Weight) {
	if (NewPacket->Time == 88051) {
		int i = 1; // todo remove
	}

	// calc WeightedSumOfActiveLinks
	int WeightedSumOfActiveLinks = 0, Arrivals, Departures;
	list <ConnectionProperties> :: iterator ConnectionIterator;
	for (ConnectionIterator = ConnectionList.begin(); ConnectionIterator != ConnectionList.end(); ++ConnectionIterator) {
		Arrivals = CalcArrivals(ConnectionIterator);
		Departures = CalcDepartures(ConnectionIterator);
		if (Arrivals > Departures) {
			//WeightedSumOfActiveLinks = WeightedSumOfActiveLinks + (Arrivals - Departures) * ConnectionIterator->Weight; // todo check weight
			WeightedSumOfActiveLinks += ConnectionIterator->Weight;
		}
	}
	if (WeightedSumOfActiveLinks == 0) { // if no one is active
		WeightedSumOfActiveLinks = Weight;
	}

	long x = NewPacket->Time - Scheduler.RoundTime;
	Scheduler.RoundTime = NewPacket->Time;
	Scheduler.RoundValue = Scheduler.RoundValue + (long double)x / WeightedSumOfActiveLinks;

}

int CalcArrivals(list <ConnectionProperties> ::iterator ConnectionIterator) {
	int Arrivals = 0;
	list <long double> ::iterator ArrivalsIterator;
	for (ArrivalsIterator = ConnectionIterator->Arrivals.begin(); ArrivalsIterator != ConnectionIterator->Arrivals.end(); ++ArrivalsIterator) {
		if (*ArrivalsIterator <= Scheduler.RoundValue) {
			Arrivals++;
		}
		else {
			break;
		}
	}
	return Arrivals;
}

int CalcDepartures(list <ConnectionProperties> ::iterator ConnectionIterator) {
	int Departures = 0;
	list <long double> ::iterator DeparturesIterator;
	for (DeparturesIterator = ConnectionIterator->Departures.begin(); DeparturesIterator != ConnectionIterator->Departures.end();
																											++DeparturesIterator) {
		if (*DeparturesIterator <= Scheduler.RoundValue) {
			Departures++;
		}
		else {
			break;
		}
	}
	return Departures;
}

void CalcLastForPacketInExistingConnection(PacketProperties *NewPacket, list <ConnectionProperties> ::iterator ConnectionIterator) {
	long double MaxValue = (!ConnectionIterator->Packets.empty() && ConnectionIterator->Packets.front().Last > Scheduler.RoundValue) ?
																	ConnectionIterator->Packets.front().Last : Scheduler.RoundValue;
	NewPacket->Last = MaxValue + (long double)NewPacket->Length / ConnectionIterator->Weight;
	ConnectionIterator->Arrivals.push_back(Scheduler.RoundValue);
	ConnectionIterator->Departures.push_back(NewPacket->Last);
	Scheduler.TotalDepartures += 1;
}

void CalcLastForPacketInNewConnection(PacketProperties *NewPacket, ConnectionProperties NewConnection) {
	NewPacket->Last = Scheduler.RoundValue + (long double)NewPacket->Length / NewConnection.Weight;
}