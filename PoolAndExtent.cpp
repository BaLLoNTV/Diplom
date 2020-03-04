#pragma once
#include "StorageSubsystem.h"
#include "IOtimeIOOperation.h"
#include "PoolAndExtent.h"
#include <queue>
#include <map>
#include <string>
#include <vector>
#include <cmath>
#include <fstream>

//multimap<Time, RequestInfo, ByTime> unit_queue_in;    multimap<Time, RequestInfo, ByTime>::iterator itr = this->unit_queue_in.begin()
//multimap<Time, ComplitedRequestInfo, ByTime> unit_queue_out; multimap<Time, ComplitedRequestInfo, ByTime>::iterator itr = unit_queue_out.begin()


extern std::map<unsigned short int, DataStorageModel*> DataStorageModels;
LogicalUnit::LogicalUnit(void) {};
LogicalUnit::~LogicalUnit(void) {};

bool LogicalUnit::respond_system_time(unsigned long long int uid,Time respondable_time) {
	ByTime lhs_less_rhs;
	IOProcessReport data_sorage_model_response;
	pair<Time, ComplitedLogicalUnitRequestInfo> temp_complited_request_info;
	Time temp_time;
	if (lhs_less_rhs(respondable_time, this->system_time))
		return false;
	
		for (multimap<Time, ComplitedLogicalUnitRequestInfo, ByTime>::iterator itr = this->unit_queue_out.begin(); (itr != this->unit_queue_out.end()); itr = this->unit_queue_out.begin()) {
			if (!lhs_less_rhs(this->unit_queue_out.begin()->first, respondable_time)) {
				break;
			};
			temp_complited_request_info = *(this->unit_queue_out.begin());
			this->pool_of_logical_unit->send_io_complit_report(this->unit_queue_out.begin()->second.request.main_request_itr, this->unit_queue_out.begin()->second.request.sub_requests_itr, *this->unit_queue_out.begin());
			this->unit_queue_out.erase(this->unit_queue_out.begin());//					!!!ÑÎÎÁÙÈÒÜ ÎÒ×¨Ò ÏÓËÓ!!!
			if (this->unit_queue_in.size() == 0) {
				continue;
			}
			data_sorage_model_response = DataStorageModels[uid]->send_request_to_system(this->unit_queue_in.begin()->second.request.IO, { temp_complited_request_info.first.S,temp_complited_request_info.first.nS + 1 });
			if (!data_sorage_model_response.Acceptance) {
				this->unit_queue_out.insert(pair<Time, ComplitedLogicalUnitRequestInfo>(data_sorage_model_response.RequestComplitionTime, { { temp_complited_request_info.first.S,temp_complited_request_info.first.nS + 1 },this->unit_queue_in.begin()->second }));
				this->unit_queue_in.erase(this->unit_queue_in.begin());
				this->system_time = { temp_complited_request_info.first.S,temp_complited_request_info.first.nS + 1 };
			}
		}
		this->system_time = respondable_time;
		return true;
	
};

unsigned short int LogicalUnit::send_request_to_unit(unsigned long long int uid, RequestInfoForLogicUnit request, Time accept_time) {
	IOProcessReport data_sorage_model_response;
	ByTime lhs_less_rhs;
	if (!respond_system_time(uid, accept_time))
		return 1;
	if (unit_queue_in.size() < this->queue_depth) {
		data_sorage_model_response = DataStorageModels[uid]->send_request_to_system(request.request.IO, accept_time);
		if (data_sorage_model_response.Acceptance == 2) {
			this->unit_queue_in.insert(pair<Time, RequestInfoForLogicUnit>(accept_time, request));
			return 0;
		}
		else if(data_sorage_model_response.Acceptance == 0) {
			this->unit_queue_out.insert(pair<Time, ComplitedLogicalUnitRequestInfo>(data_sorage_model_response.RequestComplitionTime, { accept_time,request }));
			return 0;
		};
	}
	if (this->queue_depth == 0) {
		data_sorage_model_response = DataStorageModels[uid]->send_request_to_system(request.request.IO, accept_time);
		if (!data_sorage_model_response.Acceptance) {
			this->unit_queue_out.insert(pair<Time, ComplitedLogicalUnitRequestInfo>(data_sorage_model_response.RequestComplitionTime, { accept_time,request }));
			return 0;
		};
	};
	return 2;

};
 



StoragePool::StoragePool(void) {

};


StoragePool::~StoragePool() {

};

bool StoragePool::storage_pool_init(unsigned long long int extent_size, unsigned short int queue_depth) {
	this->extent_size = extent_size;
	this->queue_depth = queue_depth;
	return true;
};

bool StoragePool::add_storage_to_pool(unsigned short int tier, unsigned short int storage_device_uid, unsigned short int queue_depth) {
	if (DataStorageModels.end() == DataStorageModels.find(storage_device_uid)) {
		return false;
	}
	if (this->LogicalUnits.find(storage_device_uid) != this->LogicalUnits.end()) {
		return false;
	}
	unsigned long int j = 0 , i = 0;
	LogicalUnit Temp;
	SorageSystemStat TempStat = DataStorageModels[storage_device_uid]->GetSystemStat();
	Temp.tier = tier;
	Temp.BlockSize = TempStat.BlockSize;
	Temp.queue_depth = queue_depth;
	Temp.ExtentCount = (unsigned long int)(TempStat.BlockSize * TempStat.BlockCount / this->extent_size);
	for ( j = 0, i = 0;(i < 4194304) && (j < Temp.ExtentCount);i++) {
		if (pool_extents.end() == pool_extents.find(i)) {
			pool_extents.insert(pair<unsigned long int, Extent>(i, {false,storage_device_uid,j++}));
		};
	};
	Temp.ExtentCount = j;
	this->free_extent_count += j;
	Temp.pool_of_logical_unit = this;
	this->LogicalUnits.insert(pair<unsigned short int, LogicalUnit>(storage_device_uid, Temp));
	return true;
};

unsigned short int StoragePool::create_logical_volume(unsigned short int logical_unit_number, unsigned long int extent_count, unsigned short int block_size) {
	std::map<unsigned long int, Extent>::iterator itr = this->pool_extents.begin();
	if (this->Volumes.find(logical_unit_number) != this->Volumes.end())
		return 1;
	if (itr == this->pool_extents.end())
		return 2;
	if (extent_count > this->free_extent_count)
		return 3;
	MappingTable temp_mapping_table;
	for (unsigned short int i = 0; i < extent_count; i++)
		for (; itr != this->pool_extents.end();itr++) {
			if (!itr->second.allocation) {
				itr->second.allocation = true;
				temp_mapping_table.Table.insert(pair<unsigned long int, unsigned long int>(i, itr->first));
				itr++;
				this->free_extent_count--;
				break;
			};
		}
	this->MappingTables.insert(pair<unsigned short int, MappingTable>(logical_unit_number, temp_mapping_table));
	this->Volumes.insert(pair< unsigned long int, LogicalVolume >(logical_unit_number, { block_size , extent_count }));
};



bool StoragePool::respond_time_to_pool(Time respondable_time) {
	ByTime lhs_less_rhs;
	if(lhs_less_rhs(respondable_time,this->PoolTime))
		return false;
	this->PoolTime = respondable_time;
	return true;
};

unsigned short int StoragePool::send_io_to_pool(unsigned short int logical_unit_number, IOOperation IO, Time accept_time){
	if(!this->respond_time_to_pool(accept_time))
		return 1;
	if (this->Volumes.find(logical_unit_number) == this->Volumes.end())
		return 2;
	if ((this->Volumes[logical_unit_number].block_size * (IO.LogicalBlockAddress + IO.TransferLength)) > ((this->Volumes[logical_unit_number].extent_count) * this->extent_size))
		return 3;
	if (this->queue_depth <= this->requests.size())
		return 4; // íàïèñàòü ïðî îòêàç
	PartionedRequest temp_partitioned_request;
	temp_partitioned_request.logical_volume_uid = logical_unit_number;
	temp_partitioned_request.IO = IO;
	RequestInfo temp_request_info;
	IOOperation temp_IO;
	unsigned long long int temp_transfer_length = 0 , temp_storage_transfer_length = 0, temp_volume_transfer_length = 0;
	double storage_unit_block_size_corelation = 0;
	unsigned long int temp_logical_block_adress = 0 , temp_storagr_logical_block_adress = 0 , temp_storage_block_size = 0 ;
	unsigned long int temp_asked_logical_volume_extent = 0, number_of_asked_logical_volume_extents = 1;
	std::map<unsigned long int, LogicalVolume>::iterator volume_itr = this->Volumes.find(logical_unit_number);
	std::map<unsigned short int, LogicalUnit>::iterator logigal_unit_iterator;
	temp_asked_logical_volume_extent = (unsigned long int)(IO.LogicalBlockAddress / (this->extent_size / volume_itr->second.block_size));
	std::map<unsigned long int, unsigned long int>::iterator in_map_table_itr = this->MappingTables[logical_unit_number].Table.find(temp_asked_logical_volume_extent);
	number_of_asked_logical_volume_extents = (unsigned long int)((IO.LogicalBlockAddress + IO.TransferLength) * volume_itr->second.block_size / this->extent_size + 1);
	temp_logical_block_adress = IO.LogicalBlockAddress;
	temp_transfer_length = IO.TransferLength;
	for (unsigned long int i = 0; i < number_of_asked_logical_volume_extents; i++) {
		temp_storage_block_size = this->LogicalUnits[this->pool_extents[in_map_table_itr->second].sotage_sustem_uid].BlockSize;
		storage_unit_block_size_corelation = temp_storage_block_size / volume_itr->second.block_size;
		temp_storagr_logical_block_adress = this->pool_extents[in_map_table_itr->second].extent_number_in_storage * this->extent_size / temp_storage_block_size + (unsigned long int)ceil(temp_logical_block_adress % (this->extent_size / volume_itr->second.block_size) / storage_unit_block_size_corelation);
		temp_volume_transfer_length = this->extent_size / volume_itr->second.block_size - temp_logical_block_adress % (this->extent_size / volume_itr->second.block_size) - 1;
		if (temp_volume_transfer_length < temp_transfer_length) {
			temp_transfer_length -= temp_volume_transfer_length;
		} else {
			temp_volume_transfer_length = temp_transfer_length;
		};
		temp_storage_transfer_length = (unsigned long int)floor(temp_volume_transfer_length / storage_unit_block_size_corelation);
		temp_request_info.IO = { IO.RequestFlags,temp_storagr_logical_block_adress,temp_storage_transfer_length };
		temp_request_info.extent_uid = in_map_table_itr->second;
		temp_partitioned_request.subrequests.insert(pair<bool, RequestInfo>(false, temp_request_info));
		temp_logical_block_adress += temp_volume_transfer_length+1;
		in_map_table_itr++;
	};
	this->requests.insert(pair<Time,PartionedRequest>(accept_time, temp_partitioned_request));

};



unsigned short int StoragePool::send_io_complit_report(multimap<Time, PartionedRequest, ByTime>::iterator request_iterator, multimap<bool, RequestInfo>::iterator subrequests_iterator, pair< Time, ComplitedLogicalUnitRequestInfo> complited_request) {
	if (request_iterator == this->requests.end())
		return 0;
	if (subrequests_iterator == request_iterator->second.subrequests.end())
		return 0;
	ComplitedRequestInfo tenp_report;
	multimap<Time, ComplitedRequestInfo, ByTime>::iterator itr;
	tenp_report.accept_time = complited_request.second.accept_time;
	tenp_report.request = subrequests_iterator->second;
	request_iterator->second.subrequest_responses.insert(pair<Time, ComplitedRequestInfo>(complited_request.first, tenp_report));
	request_iterator->second.subrequests.erase(subrequests_iterator);
	if (request_iterator->second.subrequests.size() == 0) {
		std::ofstream myfile;
		myfile.open("1.csv", fstream::app);
		myfile << "IO INFO" << endl;
		myfile << request_iterator->first.S << ";" << request_iterator->first.nS << ";";
		itr = request_iterator->second.subrequest_responses.begin();;
		myfile << itr->first.S << ";" << itr->first.nS;
		myfile << request_iterator->second.IO.LogicalBlockAddress << ";" << request_iterator->second.IO.TransferLength << ";" << request_iterator->second.IO.RequestFlags << ";" << endl;
		myfile << "SUBIO INFO" << endl;
		for (itr = request_iterator->second.subrequest_responses.begin(); itr != request_iterator->second.subrequest_responses.end(); itr++)
			myfile << itr->first.S << ";" << itr->first.nS << ";" << itr->second.accept_time.S << ";" << itr->second.accept_time.nS << ";" << itr->second.request.extent_uid << ";" << itr->second.request.IO.LogicalBlockAddress << ";" << itr->second.request.IO.TransferLength << ";" << itr->second.request.IO.RequestFlags << endl;
		myfile.close();
		this->requests.erase(request_iterator);
	}
};