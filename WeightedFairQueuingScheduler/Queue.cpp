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
Input: NewPacket - pointer to the new received packet.
Output: none.
Description: update Round of packet. at first, calculating weights assuming no packet has left. then calculating weights for new round value
			 with an indication of "Last" of first packet that left if exists (we save the indication anyway in order to not go over the
			 ConnectionList again) and number of departures for new round. finally, if we see that packets have left, we calculate the
			 new x and use the new weight in order to calculate the right round value. we do the process again to see if more packets
			 have left untill converging to the actual round value.
			 without inner functions has complexity of O(num of left packet).
			 with inner functions complexity of O(total num of read packets * num of left packet)
			 =~ (if not many packets have left) =~ O(total num of read packets).
*/
void UpdateRound(PacketProperties *NewPacket);

/*
Input: Weight - weight of the packet's connection, FindLastOfFirstPacketThatLeft - if we need to check the "Last" of first packet that left.
Output: NumberOfDepartures - pointer of total number of departures to update.
Description: calculate and returns the sum of weights in current round value. check arrivals and departures for each connection and if
			 arrivals > departures then link (connection) is active and we sum its weight.
			 without inner functions has complexity of O(num of existing connections).
			 with inner functions complexity of O(total num of read packets).
*/
int CalcWeightedSumOfActiveLinks(int Weight, int *NumberOfDepartures, bool FindLastOfFirstPacketThatLeft);

/*
Input: ConnectionIterator - iterator to current checked connection, Departures - num of departures untill round value.
Output: weight of current connection.
Description: go over the arrivals list of the connection and sum it's arrivals up to current virtual time (= up to round value).
			 if we have more arrivals than departures, returns the weight of the output packet in the connection.
			 has complexity of O(num of packets in checked connection).
*/
int CalcWeightOfConnection(list <ConnectionProperties> ::iterator ConnectionIterator, int Departures);

/*
Input: ConnectionIterator - iterator to current checked connection,
	   FindLastOfFirstPacketThatLeft - if we need to check the "Last" of first packet that left.
Output: total number of departures in current connection.
Description: go over the departures list of the connection and sum it's departures up to current virtual time (= up to round value).
			 has complexity of O(num of packets in checked connection).
*/
int CalcDepartures(list <ConnectionProperties> ::iterator ConnectionIterator, bool FindLastOfFirstPacketThatLeft);

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
	while (getline(InputFilePointer, CurrentLine)) { // parse new line to NewPacket and NewConnection
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
		NewConnection.Weight = 0;
	}
	if (NewPacket.Time <= Scheduler.SystemTime) { // meaning reached end of file
		Scheduler.FinishedReadingInputFile = true;
	}
	else { // save future arrival time packet and insert it later
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
			NewPacket->Weight = ConnectionIterator->Weight;
			UpdateRound(NewPacket);
			CalcLastForPacketInExistingConnection(NewPacket, ConnectionIterator);
			ConnectionIterator->Packets.push_back(*NewPacket);
			break;
		}
	}
	if (!FoundConnection) { // add new connection
		if (NewConnection.Weight == 0) { // if got no weight put default weight
			NewConnection.Weight = 1;
		}
		NewPacket->Weight = NewConnection.Weight;
		UpdateRound(NewPacket);
		CalcLastForPacketInNewConnection(NewPacket, NewConnection);

		ArrivalProperties Arrival; Arrival.Arrival = Scheduler.RoundValue; Arrival.Weight = NewPacket->Weight;
		NewConnection.Arrivals.push_back(Arrival);
		NewConnection.Departures.push_back(NewPacket->Last);
		NewConnection.Packets.push_back(*NewPacket);
		ConnectionList.push_back(NewConnection);
	}
	Scheduler.NumberOfPacketsInQueue += 1;
}

void UpdateRound(PacketProperties *NewPacket) {
	if (NewPacket->Time == 88051) {
		int i = 1; // todo remove
	}
	bool FinishedIteratingToFindRound = false;
	int OldNumberOfDepartures, NewNumberOfDepartures, OldWeightedSumOfActiveLinks, NewWeightedSumOfActiveLinks;
	long double OldRoundValue, x, OldRoundTime;

	while (!FinishedIteratingToFindRound) { // while there are still packets that left and need to re-calculate round parameters
		Scheduler.FirstUpdateOfLastOfLeftPacket = true;
		OldNumberOfDepartures = 0; NewNumberOfDepartures = 0;
		OldRoundValue = Scheduler.RoundValue;
		OldRoundTime = Scheduler.RoundTime;
		OldWeightedSumOfActiveLinks = CalcWeightedSumOfActiveLinks(NewPacket->Weight, &OldNumberOfDepartures, false);

		x = NewPacket->Time - Scheduler.RoundTime;
		Scheduler.RoundTime = NewPacket->Time;
		Scheduler.RoundValue = Scheduler.RoundValue + (long double)x / OldWeightedSumOfActiveLinks;

		NewWeightedSumOfActiveLinks = CalcWeightedSumOfActiveLinks(NewPacket->Weight, &NewNumberOfDepartures, true);
		if (NewNumberOfDepartures > OldNumberOfDepartures) { // a packet has left - need to re-calculate round parameters
			Scheduler.RoundValue = OldRoundValue;
			x = (Scheduler.LastOfLeftPacket - Scheduler.RoundValue) * OldWeightedSumOfActiveLinks;
			Scheduler.RoundValue = Scheduler.LastOfLeftPacket;// +(long double)x / NewWeightedSumOfActiveLinks;
			Scheduler.RoundTime = OldRoundTime + x;
			//FinishedIteratingToFindRound = true; // todo
		}
		else { // no departures - correct value of Round
			FinishedIteratingToFindRound = true;
		}
	}
}

int CalcWeightedSumOfActiveLinks(int Weight, int *NumberOfDepartures, bool FindLastOfFirstPacketThatLeft) {
	int WeightedSumOfActiveLinks = 0, Departures;
	list <ConnectionProperties> ::iterator ConnectionIterator;
	for (ConnectionIterator = ConnectionList.begin(); ConnectionIterator != ConnectionList.end(); ++ConnectionIterator) {
		Departures = CalcDepartures(ConnectionIterator, FindLastOfFirstPacketThatLeft);
		*NumberOfDepartures = *NumberOfDepartures + Departures;
		WeightedSumOfActiveLinks += CalcWeightOfConnection(ConnectionIterator, Departures);
		/*if (Arrivals > Departures) {
			//WeightedSumOfActiveLinks = WeightedSumOfActiveLinks + (Arrivals - Departures) * ConnectionIterator->Weight; // todo check weight
			WeightedSumOfActiveLinks += ConnectionIterator->Weight;
		}*/
		/*if (FindLastPacketThatLeft) {
			if (ConnectionIterator->TempDepartures < Departures) {
				if (ConnectionIterator->Departures.back() > Scheduler.LastOfLeftPacket) {
					Scheduler.LastOfLeftPacket = ConnectionIterator->Departures.back();
				}
			}
		}
		else {*/
		if (!FindLastOfFirstPacketThatLeft) {
			ConnectionIterator->TempDepartures = Departures;
		}
	}
	if (WeightedSumOfActiveLinks == 0) { // if no one is active
		WeightedSumOfActiveLinks = Weight;
	}
	return WeightedSumOfActiveLinks;
}

int CalcWeightOfConnection(list <ConnectionProperties> ::iterator ConnectionIterator, int Departures) {
	int Arrivals = 0;
	list <ArrivalProperties> ::iterator ArrivalsIterator;
	for (ArrivalsIterator = ConnectionIterator->Arrivals.begin(); ArrivalsIterator != ConnectionIterator->Arrivals.end(); ++ArrivalsIterator) {
		if (ArrivalsIterator->Arrival <= Scheduler.RoundValue) {
			Arrivals++;
			if (Arrivals == Departures + 1) { // the packet that will be output in GPS simulation
				return ArrivalsIterator->Weight;
				//return ConnectionIterator->Weight;
			}
		}
		else {
			break;
		}
	}
	return 0;
}

int CalcDepartures(list <ConnectionProperties> ::iterator ConnectionIterator, bool FindLastOfFirstPacketThatLeft) {
	int Departures = 0;
	list <long double> ::iterator DeparturesIterator;
	for (DeparturesIterator = ConnectionIterator->Departures.begin(); DeparturesIterator != ConnectionIterator->Departures.end();
																											++DeparturesIterator) {
		if (*DeparturesIterator <= Scheduler.RoundValue) {
			Departures++;
			if (FindLastOfFirstPacketThatLeft) {
				if (Departures == ConnectionIterator->TempDepartures + 1) { // first packet that left in checked connection
					if (Scheduler.FirstUpdateOfLastOfLeftPacket) {
						Scheduler.LastOfLeftPacket = *DeparturesIterator;
						Scheduler.FirstUpdateOfLastOfLeftPacket = false;
					}
					else if (*DeparturesIterator < Scheduler.LastOfLeftPacket) {
						Scheduler.LastOfLeftPacket = *DeparturesIterator;
					}
				}
			}
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
	ArrivalProperties Arrival; Arrival.Arrival = Scheduler.RoundValue; Arrival.Weight = NewPacket->Weight;
	ConnectionIterator->Arrivals.push_back(Arrival);
	ConnectionIterator->Departures.push_back(NewPacket->Last);
}

void CalcLastForPacketInNewConnection(PacketProperties *NewPacket, ConnectionProperties NewConnection) {
	NewPacket->Last = Scheduler.RoundValue + (long double)NewPacket->Length / NewConnection.Weight;
}