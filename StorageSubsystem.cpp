#include "StorageSubsystem.h"
#include "randomU.h"
#include "IOtimeIOOperation.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <iostream>

std::map<unsigned short int, DataStorageModel*> DataStorageModels;

DataStorageModel::DataStorageModel(void) {
};

void DataStorageModel::StorageSubsystemInit(unsigned short int BlockSize, unsigned long long int BlockCount, unsigned short int StorageSystemQueue,signed short int AccessDistribution,
	double  AccessDistributionMin, double  AccessDistributionMax, signed short int ReadDistribution, double  ReadDistributionMin, double  ReadDistributionMax, signed short int WriteDistribution, double WriteDistributionMin, double WriteDistributionMax) {
	unsigned int i, n, k;
	DataStorageModel::block_size = BlockSize;
	DataStorageModel::block_count = BlockCount;
	DataStorageModel::system_queue_depth = StorageSystemQueue;
	DataStorageModel::system_time = { 0,0 };
	destributiontype[0] = AccessDistribution;
	destributiontype[1] = ReadDistribution;
	destributiontype[2] = WriteDistribution;
	for (n = 0,i = 0; i < 3; i++) {
		if (destributiontype[i] < 0) {
			continue;
		}
		else {
			n++;
		}
	}
	generator_meta = new generator_info [n];
	for (i = 0, k = 0 ; k < n ;i++) {
		if (destributiontype[i] == -1)
			continue;
		if (destributiontype[i] == 0) {
			generator_meta[k].Union = new CRandomMother();
			generator_meta[k].destributio_min_max[0] = AccessDistributionMin;
			generator_meta[k].destributio_min_max[1] = AccessDistributionMax;
			k++;
		}
		if (destributiontype[i] == 1) {
			generator_meta[k].Normal = new CRandomNormal();
			generator_meta[k].destributio_min_max[0] = ReadDistributionMin;
			generator_meta[k].destributio_min_max[1] = ReadDistributionMax;
			k++;
		}
		if (destributiontype[i] == 2) {
			generator_meta[k].Exp = new CRandomExp();
			generator_meta[k].destributio_min_max[0] = WriteDistributionMin;
			k++;
		}
	}
};
DataStorageModel::~DataStorageModel() {
	int x = 0;
	for (int i = 0; i < 3; i++) {
		if (this->destributiontype[i] >= 0) { x++; }
	}
	for (int i = 0; i < x; i++) {
		if (this->generator_meta[i].Union != nullptr)
			delete(this->generator_meta[i].Union);
		if (this->generator_meta[i].Normal != nullptr)
			delete(this->generator_meta[i].Normal);
		if (this->generator_meta[i].Exp != nullptr)
			delete(this->generator_meta[i].Exp);
	}
	delete(generator_meta);
	//for(int i =0;i<3;i++)
	//delete generator_meta;
};

bool DataStorageModel::respond_time_to_system (Time AcceptTime) {
	int i = 0;
	ByTime x;
	if (x( AcceptTime , this->system_time ))
		return false;
	if (!this->QueueChannelArray.empty())
		for (std::map<Time, IOOperation, ByTime>::iterator it = this->QueueChannelArray.begin(); it != this->QueueChannelArray.end(); it = this->QueueChannelArray.begin()) {
			if (x (it->first,AcceptTime)) {
				this->QueueChannelArray.erase(it);
				continue;
			}
			break;
		}
	system_time = AcceptTime;
	return true;
};


IOProcessReport DataStorageModel::send_request_to_system(IOOperation IO, Time AcceptTime) {
	unsigned long int i = 0;
	unsigned short int n = 0, m = 0;
	unsigned long long TempTransferLength = 0, k =0;
	Time ComplitionTime = {0, 0};
	IOProcessReport Report = {1,ComplitionTime};
	long int Place = 0;
	
	if (!DataStorageModel::respond_time_to_system(AcceptTime))
		return Report;
	/*
	printf(" AcceptTime %llu %llu \n", SorageSystemTime.S, SorageSystemTime.nS);
	for (int i = 0; i < StorageSubsystem::StorageSystemQueue; i++) {
		printf("%s %llu %llu \n", QueueChannelArray[i].ChannelInProcess ? "true" : "false", QueueChannelArray[i].RidTime.S, QueueChannelArray[i].RidTime.nS);
		//QueueChannelArray[i] = { true, { 1,1 } };
	};
	*/
	if(this->system_queue_depth <= this->QueueChannelArray.size())
		return { 2,ComplitionTime };
	if ((IO.LogicalBlockAddress + IO.TransferLength) > this->block_count)
		return { 3,ComplitionTime };
	ComplitionTime = AcceptTime;
	for (i = 0, m = 0,n = 0; m < 2; i=IO.RequestFlags+1,m++) {
		generator_info xu = generator_meta[n];
		switch(DataStorageModel::destributiontype[i]) {
		case 0:
			if (i == 0) { TempTransferLength = 1; }
			else { TempTransferLength = IO.TransferLength+1; }
			for (k = 0; k < TempTransferLength; k++) {
				ComplitionTime = AddTimeToIOTime(ComplitionTime, 0, this->generator_meta[n].Union->IRandom(this->generator_meta[n].destributio_min_max[0], this->generator_meta[n].destributio_min_max[1]));
			};
			n+= IO.RequestFlags + 1;
			break;
		case 1:
			if (i == 0) { TempTransferLength = 1; }
			else { TempTransferLength = IO.TransferLength+1; }
			for (k = 0; k < TempTransferLength; k++) {
				ComplitionTime = AddTimeToIOTime(ComplitionTime, 0, this->generator_meta[n].Normal->NRandom(this->generator_meta[n].destributio_min_max[0], this->generator_meta[n].destributio_min_max[1]));
			}
			n += IO.RequestFlags + 1;
			break;
		case 2:
			if (i == 0) { TempTransferLength = 1; }
			else { TempTransferLength = IO.TransferLength+1; }
			for (k = 0; k < TempTransferLength; k++) {
				ComplitionTime = AddTimeToIOTime(ComplitionTime, 0, this->generator_meta[n].Exp->ExpRandom(this->generator_meta[n].destributio_min_max[0]));
			}
			n += IO.RequestFlags + 1;
			break;
		default:
			continue;
		}
	}
	this->QueueChannelArray.insert(pair<Time, IOOperation>({ComplitionTime, IO}));
	return { 0,ComplitionTime };
};





SorageSystemStat DataStorageModel::GetSystemStat() {
	return {system_queue_depth, block_size, block_count};
};

Time DataStorageModel::GetSystemTime() {
	return DataStorageModel::system_time;
};

void DataStorageModel::print_chanels_stat() {
	printf(" system_time %llu s %llu ns \n", system_time.S, system_time.nS);
	std::map<Time, IOOperation, ByTime>::iterator it = this->QueueChannelArray.begin();
	cout << this->QueueChannelArray.size() << endl;
	for (unsigned int i = 0; i < this->QueueChannelArray.size(); it++,i++) {
		cout << it->first.S << " / " << it->first.nS << " / " << it->second.RequestFlags << " / " << it->second.LogicalBlockAddress << " / " << it->second.TransferLength << endl;
	};
};

void DataStorageModel::drop_all_requests() {
	this->QueueChannelArray.clear();
};


Time DataStorageModel::get_min_free_time() {
 std::multimap<Time, IOOperation, ByTime>::iterator it = QueueChannelArray.begin();
 return it->first;
};


bool CreateNewStoragaSubsustemUnit(unsigned short int UnitLogicalNumber,unsigned short int BlockSize, unsigned long long int BlockCount, unsigned short int StorageSystemQueue, signed short int AccessDistribution,
	double  AccessDistributionMin, double  AccessDistributionMax, signed short int ReadDistribution, double  ReadDistributionMin, double  ReadDistributionMax, signed short int WriteDistribution, double WriteDistributionMin, double WriteDistributionMax) {
	DataStorageModel* Temp;
	try {
		DataStorageModels.at(UnitLogicalNumber);
	}
	catch (const std::out_of_range&) {
		Temp = new DataStorageModel();
		(*Temp).StorageSubsystemInit(BlockSize, BlockCount, StorageSystemQueue, AccessDistribution, AccessDistributionMin, AccessDistributionMax, ReadDistribution, ReadDistributionMin, ReadDistributionMax, WriteDistribution, WriteDistributionMin, WriteDistributionMax);
		DataStorageModels.insert(std::pair<unsigned short int, DataStorageModel*>(UnitLogicalNumber, Temp));
		//StorageSubsystemUnits[UnitLogicalNumber]->ClearQueueChanells();
		std::cout << "Storage with identifier " << UnitLogicalNumber << " created." << '\n';
		return true;
	};
	std::cout << "Storage with identifier "<<UnitLogicalNumber<<" already exist."<<'\n';
	return false;
};


void print_all_storage_systems() {
	for (std::map<unsigned short int, DataStorageModel*>::iterator it = DataStorageModels.begin(); it != DataStorageModels.end(); it++)
		cout << it->first << " / " << it->second->block_size << " / " << it->second->block_count << " / " << endl;
};