#pragma once
#include <functional>

struct Time {
	unsigned long long int S;
	unsigned long long int nS;
};

Time AddTimeToIOTime(Time TempTime, unsigned long long int S, unsigned long long int nS);
Time AddIOTimeToIOTime(Time Time0, Time Time1);

struct IOOperation {
	unsigned short int RequestFlags;
	unsigned long int LogicalBlockAddress;
	unsigned long long int TransferLength;
};


struct ByTime{
	bool operator()(const Time& lhs, const Time& rhs) const
	{
		return ((lhs.S < rhs.S) || (lhs.S == rhs.S && lhs.nS < rhs.nS));
	}
};

bool IOOperationConstructor(IOOperation** IO, unsigned short int RequestFlags, unsigned long int LogicalBlockAddress, unsigned long long int TransferLength);
void IOOperationDestructor(IOOperation** IO);
