#include <sstream>

#include "Queue.h"
#include "Scheduler.h"

using namespace std;

list<ConnectionProperties> ConnectionList; // keeps all the connections in the test

/*
Input: InputFilePointer - pointer to input file. // todo
Output: none.
Description: read new packets, parse and update their patameters and insert them to right connection in ConnectionList.
			 stops reading when reads a packet with time > system time and saves the extra packet.
			 with inner functions has complexity of O((num of existing connections + total num of read packets) * num of new read packets)
			 = O(total num of read packets * num of new read packets).
*/
void ReadPacketsInCurrentTime(istream &InputFilePointer);

/*
Input: NewPacket - pointer to the new received packet. NewConnection - paramterets for parsed connection of packet.
Output: none.
Description: for each new packet - searches existing connection, calculating Round and Last for packet and insert packet to ConnectionList.
			 if no connection is found a new connection is created.
			 without inner functions has complexity of O(num of existing connections).
			 with inner functions has complexity of O(num of existing connections + total num of read packets)
			 = O(total num of read packets).
*/
void HandleNewPacket(PacketProperties *NewPacket, ConnectionProperties NewConnection);

/*
Input: NewPacket - pointer to the new received packet. Weight - weight of the packet's connection.
Output: none.
Description: update Round of packet. at first, calculating weights assuming no packet has left. then calculating weights for new round value
			 with an indication of "Last" of last packet that left if exists (we save the indication anyway in order to not go over the
			 ConnectionList again) and number of departures for new round. finally, if we see that packets have left, we calculate the
			 new x and use the new weight in order to calculate the right round value.
			 without inner functions has complexity of O(1). with inner functions complexity of O(total num of read packets).
*/
void UpdateRound(PacketProperties *NewPacket, int Weight);

/*
Input: Weight - weight of the packet's connection, FindLastPacketThatLeft - tells if we need to check the "Last" of last packet that left.
Output: NumberOfDepartures - pointer of total number of departures to update.
Description: calculate and returns the sum of weights in current round value. check arrivals and departures for each connection and if
			 arrivals > departures then link (connection) is active and we sum its weight.
			 without inner functions has complexity of O(num of existing connections).
			 with inner functions complexity of O(total num of read packets).
*/
int CalcWeightedSumOfActiveLinks(int Weight, int *NumberOfDepartures, bool FindLastPacketThatLeft);

/*
Input: ConnectionIterator - iterator to current checked connection.
Output: total number of arrivals in current connection.
Description: go over the arrivals list of the connection and sum it's arrivals up to current virtual time (= up to round value).
			 has complexity of O(num of packets in checked connection).
*/
int CalcArrivals(list <ConnectionProperties> ::iterator ConnectionIterator);

/*
Input: ConnectionIterator - iterator to current checked connection.
Output: total number of departures in current connection.
Description: go over the departures list of the connection and sum it's departures up to current virtual time (= up to round value).
			 has complexity of O(num of packets in checked connection).
*/
int CalcDepartures(list <ConnectionProperties> ::iterator ConnectionIterator);

/*
Input: NewPacket - pointer to the new received packet, ConnectionIterator - iterator to current checked connection.
Output: none.
Description: update Last for current packet in existing connection.
			 has complexity of O(1)
*/
void CalcLastForPacketInExistingConnection(PacketProperties *NewPacket, list <ConnectionProperties> ::iterator ConnectionIterator);

/*
Input: NewPacket - pointer to the new received packet, NewConnection - paramterets for parsed connection of packet.
Output: none.
Description: update Last for current packet in new connection.
			 has complexity of O(1)
*/
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
		NewConnection.Packets.push_back(*NewPacket);
		ConnectionList.push_back(NewConnection);
	}
	Scheduler.NumberOfPacketsInQueue += 1;
}

void UpdateRound(PacketProperties *NewPacket, int Weight) {
	if (NewPacket->Time == 88051) {
		int i = 1; // todo remove
	}
	Scheduler.LastOfLeftPacket = 0;
	int OldNumberOfDepartures = 0, NewNumberOfDepartures = 0;
	long double OldRoundValue = Scheduler.RoundValue;
	int OldWeightedSumOfActiveLinks = CalcWeightedSumOfActiveLinks(Weight, &OldNumberOfDepartures, false);

	long x = NewPacket->Time - Scheduler.RoundTime;
	Scheduler.RoundTime = NewPacket->Time;
	Scheduler.RoundValue = Scheduler.RoundValue + (long double)x / OldWeightedSumOfActiveLinks;

	int NewWeightedSumOfActiveLinks = CalcWeightedSumOfActiveLinks(Weight, &NewNumberOfDepartures, true);
	if (NewNumberOfDepartures > OldNumberOfDepartures) { // a packet has left - need to recalculate round
		Scheduler.RoundValue = OldRoundValue;
		x = (Scheduler.LastOfLeftPacket - Scheduler.RoundValue) * OldWeightedSumOfActiveLinks;
		Scheduler.RoundValue = Scheduler.LastOfLeftPacket + (long double)x / NewWeightedSumOfActiveLinks;
	}
}

int CalcWeightedSumOfActiveLinks(int Weight, int *NumberOfDepartures, bool FindLastPacketThatLeft) {
	int WeightedSumOfActiveLinks = 0, Arrivals, Departures;
	list <ConnectionProperties> ::iterator ConnectionIterator;
	for (ConnectionIterator = ConnectionList.begin(); ConnectionIterator != ConnectionList.end(); ++ConnectionIterator) {
		Arrivals = CalcArrivals(ConnectionIterator);
		Departures = CalcDepartures(ConnectionIterator);
		*NumberOfDepartures = *NumberOfDepartures + Departures;
		if (Arrivals > Departures) {
			//WeightedSumOfActiveLinks = WeightedSumOfActiveLinks + (Arrivals - Departures) * ConnectionIterator->Weight; // todo check weight
			WeightedSumOfActiveLinks += ConnectionIterator->Weight;
		}
		if (FindLastPacketThatLeft) {
			if (ConnectionIterator->TempDepartures < Departures) {
				if (ConnectionIterator->Departures.back() > Scheduler.LastOfLeftPacket) {
					Scheduler.LastOfLeftPacket = ConnectionIterator->Departures.back();
				}
			}
		}
		else {
			ConnectionIterator->TempDepartures = Departures;
		}
	}
	if (WeightedSumOfActiveLinks == 0) { // if no one is active
		WeightedSumOfActiveLinks = Weight;
	}
	return WeightedSumOfActiveLinks;
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
}

void CalcLastForPacketInNewConnection(PacketProperties *NewPacket, ConnectionProperties NewConnection) {
	NewPacket->Last = Scheduler.RoundValue + (long double)NewPacket->Length / NewConnection.Weight;
}