#pragma once
#include "StorageSubsystem.h"
#include "IOtimeIOOperation.h"
#include <queue>
#include <map>
#include <string>
#include <vector>
using namespace std;

class StoragePool;

struct RequestInfo {
	IOOperation IO;
	unsigned long int extent_uid;
};

struct ComplitedRequestInfo {
	Time accept_time;
	RequestInfo request;
};

struct PartionedRequest {
	IOOperation IO;
	unsigned short int logical_volume_uid;
	multimap<bool,RequestInfo> subrequests;
	multimap<Time,ComplitedRequestInfo,ByTime> subrequest_responses;
};

struct RequestInfoForLogicUnit {
	RequestInfo request;
	multimap<Time, PartionedRequest, ByTime>::iterator main_request_itr;
	multimap<bool, RequestInfo>::iterator sub_requests_itr;
};

struct ComplitedLogicalUnitRequestInfo {
	Time accept_time;
	RequestInfoForLogicUnit request;
};

class LogicalUnit {
public:
	Time system_time = {0,0};
	StoragePool* pool_of_logical_unit;
	unsigned short int tier = 0;
	unsigned short int BlockSize = 0;
	unsigned long int ExtentCount = 0;
	unsigned short int queue_depth = 1;
	multimap<Time, RequestInfoForLogicUnit,ByTime> unit_queue_in;
	multimap<Time, ComplitedLogicalUnitRequestInfo,ByTime> unit_queue_out;
	LogicalUnit(void);
	~LogicalUnit();
	bool respond_system_time(unsigned long long int uid,Time respondable_time);
	unsigned short int send_request_to_unit(unsigned long long int uid, RequestInfoForLogicUnit request, Time accept_time);
};
struct Extent {
	bool allocation;
	unsigned short int sotage_sustem_uid;
	unsigned long long int extent_number_in_storage;
};

struct MappingTable {
	std::map<unsigned long int, unsigned long int> Table;
};


struct LogicalVolume {
	unsigned short int block_size = 0;
	unsigned long int extent_count = 0;
};

class StoragePool {
public:
	StoragePool(void);
	~StoragePool();
	bool storage_pool_init(unsigned long long int extent_size, unsigned short int queue_depth);
	bool add_storage_to_pool(unsigned short int tier, unsigned short int storage_device_uid, unsigned short int queue_depth);
	unsigned short int create_logical_volume(unsigned short int logical_unit_number,unsigned long int extent_count, unsigned short int block_size);
	//Time get_system_time();
	bool respond_time_to_pool(Time respondable_time);
	unsigned short int send_io_to_pool(unsigned short int logical_unit_number, IOOperation IO, Time acces_time);
	unsigned short int send_io_complit_report(multimap<Time, PartionedRequest, ByTime>::iterator request_iterator, multimap<bool, RequestInfo>::iterator subrequests_iterator, pair< Time, ComplitedLogicalUnitRequestInfo> complited_request);
protected:
	Time PoolTime = { 0,0 };
	multimap<Time,PartionedRequest,ByTime> requests;
	unsigned short int queue_depth = 1;
	unsigned long long int extent_size = 0;
	unsigned long long int free_extent_count = 0;
	std::map<unsigned short int,LogicalUnit> LogicalUnits{};
	std::map<unsigned long int,Extent> pool_extents{};
	std::map<unsigned short int,MappingTable> MappingTables{};
	std::map<unsigned long int,LogicalVolume> Volumes{};
};

//bool CreateStoragePool(unsigned short int PoolNumberIdentifier, unsigned long long int ExtentSize);

/*
template <typename TX>
struct ListItem
{
	ListItem* Next;
	TX Data;
	ListItem(const TX& init)
		: Data(init)
	{}
};


*/

