#pragma once

#include <string>
#include <iostream>
#include <list>
#include <iterator>

typedef struct _QueueProperties {
	int Time;
	std::string SAdd;
	int SPort;
	std::string Dadd;
	int DPort;
	int Length;
	int Weight = 1;
}QueueProperties;

extern std::list<QueueProperties> Queue;