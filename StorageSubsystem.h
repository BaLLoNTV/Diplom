#pragma once
#include "IOtimeIOOperation.h"
#include "randomU.h"
#include <map>
#include <iostream>
#include <cstdlib>
using namespace std;

struct IOProcessReport {
	unsigned short int Acceptance;
	Time RequestComplitionTime;
};
/*
struct QueueChannel
{
	bool ChannelInProcess;
	IOOperation IO;
	Time RidTime;
};
*/

struct SorageSystemStat {
	unsigned short int StorageSystemQueue = 0;
	unsigned short int BlockSize = 0;
	unsigned long long int BlockCount = 0;
};



struct generator_info {
	CRandomNormal *Normal = nullptr;
	CRandomMother *Union = nullptr;
	CRandomExp *Exp = nullptr;
	double destributio_min_max[2] = { 0.0,0.0 };
};

class DataStorageModel {
public:
	Time system_time = {0,0};
	unsigned short int system_queue_depth = 0;
	unsigned short int block_size = 0;
	unsigned long long int block_count = 0;
	DataStorageModel(void);
	void StorageSubsystemInit(
		unsigned short int BlockSize,
		unsigned long long int BlockCount,
		unsigned short int StorageSystemQueue,
		signed short int AccessDistribution,
		double  AccessDistributionMin,
		double  AccessDistributionMax,
		signed short int ReadDistribution,
		double  ReadDistributionMin,
		double  ReadDistributionMax,
		signed short int WriteDistribution,
		double WriteDistributionMin,
		double WriteDistributionMax);
	bool respond_time_to_system(Time AcceptTime);
	SorageSystemStat GetSystemStat();
	Time GetSystemTime();
	Time get_min_free_time();
	void print_chanels_stat();
	IOProcessReport send_request_to_system(IOOperation IO, Time AcceptTime);
	void drop_all_requests();
	~DataStorageModel();
protected:
	multimap<Time, IOOperation, ByTime> QueueChannelArray;
	signed short int destributiontype[3] = {-1,-1,-1};
	generator_info* generator_meta = nullptr;
};

bool CreateNewStoragaSubsustemUnit(unsigned short int UnitLogicalNumber, unsigned short int BlockSize, unsigned long long int BlockCount, unsigned short int StorageSystemQueue, signed short int AccessDistribution,
	double  AccessDistributionMin, double  AccessDistributionMax, signed short int ReadDistribution, double  ReadDistributionMin, double  ReadDistributionMax, signed short int WriteDistribution, double WriteDistributionMin, double WriteDistributionMax);

void print_all_storage_systems();