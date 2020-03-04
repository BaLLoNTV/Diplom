// ConsoleApplication4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "IOtimeIOOperation.h"
#include <functional>
#include <iostream>
#include <stdlib.h>


Time AddTimeToIOTime(Time Time, unsigned long long int S, unsigned long long int nS) {
	Time.S += unsigned long long int((Time.nS + nS) / 1000000000) + S;
	Time.nS = (Time.nS + nS) % 1000000000;
	return Time;
};

Time AddIOTimeToIOTime(Time Time0, Time Time1) {
	Time0.S += int((Time0.nS + Time1.nS) / 1000000000) + Time1.S;
	Time0.nS = (Time0.nS + Time1.nS) % 1000000000;
	return Time0;
};


bool IOOperationConstructor(IOOperation** IO, unsigned short int RequestFlags, unsigned long int LogicalBlockAddress, unsigned long long int TransferLength) {
	if (!IO) { return false; }
	*IO = (IOOperation*)malloc(sizeof(IOOperation));
	if (*IO) {
		(*IO)->LogicalBlockAddress = LogicalBlockAddress;
		(*IO)->TransferLength = TransferLength;
		(*IO)->RequestFlags = RequestFlags;
		return true;
	}
	else {
		return false;
	}
};


void IOOperationDestructor(IOOperation** IO) {
	free(*IO);
	*IO = NULL;
};
