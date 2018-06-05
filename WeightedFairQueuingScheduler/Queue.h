#pragma once

#include <string>
#include <iostream>
#include <list>
#include <iterator>

typedef struct _PacketProperties {
	long Time = 0;
	int Length;
	int Weight;
	std::string InputLine;
	long double Last;
}PacketProperties;

typedef struct _ArrivalProperties {
	long double Arrival;
	int Weight;
}ArrivalProperties;

typedef struct _ConnectionProperties {
	std::string SAdd;
	long SPort;
	std::string Dadd;
	long DPort;
	int Weight = 0;
	std::list<ArrivalProperties> Arrivals;
	std::list<long double> Departures;
	std::list<PacketProperties> Packets;
	int TempDepartures;
	long double LastOfLastPacketInConnection = 0;
}ConnectionProperties;


extern std::list<ConnectionProperties> ConnectionList;

void ReadPacketsInCurrentTime();
void HandleNewPacket(PacketProperties *NewPacket, ConnectionProperties NewConnection);