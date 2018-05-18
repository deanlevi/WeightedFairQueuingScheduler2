#pragma once

#include <string>
#include <iostream>
#include <list>
#include <iterator>

typedef struct _PacketProperties {
	int Time = 0;
	int Length;
	std::string InputLine;
}PacketProperties;

typedef struct _ConnectionProperties {
	std::string SAdd;
	int SPort;
	std::string Dadd;
	int DPort;
	int Weight = 0;

	std::list<PacketProperties> Packets;
}ConnectionProperties;


extern std::list<ConnectionProperties> ConnectionList;

void ReadPacketsInCurrentTime(std::istream &InputFilePointer);