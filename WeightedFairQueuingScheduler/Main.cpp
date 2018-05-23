#include <iostream>
#include <vector>
using namespace std;

#include "Scheduler.h"
#include "Queue.h"

#define SUCCESS_CODE 0

int main(int argc, char *argv[]) {

	if (argc != 6) { // todo check
		fprintf(stderr, "Not the right amount of input arguments.\nNeed to give five.\nExiting...\n"); // first is path, other five are inputs
		//return ERROR_CODE;
	}
	HandleScheduler(argv); // todo check
	return SUCCESS_CODE;
}