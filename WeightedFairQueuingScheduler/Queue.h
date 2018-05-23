#pragma once

#include <string>
#include <iostream>
#include <list>
#include <iterator>

typedef struct _PacketProperties {
	long Time = 0;
	int Length;
	std::string InputLine;
	long double Last;
}PacketProperties;

typedef struct _ConnectionProperties {
	std::string SAdd;
	long SPort;
	std::string Dadd;
	long DPort;
	int Weight = 0;
	//bool IsLinkActive = false; // todo
	std::list<long double> Arrivals;
	std::list<long double> Departures;
	std::list<PacketProperties> Packets;
	int TempDepartures;
}ConnectionProperties;


extern std::list<ConnectionProperties> ConnectionList;

void ReadPacketsInCurrentTime(std::istream &InputFilePointer);
void HandleNewPacket(PacketProperties *NewPacket, ConnectionProperties NewConnection);