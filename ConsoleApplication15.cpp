// ConsoleApplication15.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <map>
#include "IOtimeIOOperation.h"
#include "randomU.h"
#include "StorageSubsystem.h"
#include "PoolAndExtent.h"
using namespace std;
extern std::map<unsigned short int, DataStorageModel*> DataStorageModels;
int main()
{
	StoragePool Pool;
	IOOperation TempIo;
	TempIo.LogicalBlockAddress = 16380;
	TempIo.RequestFlags = 0;
	TempIo.TransferLength = 60000;
	CreateNewStoragaSubsustemUnit(0, 512, 98304, 5, 0, 100, 1, 0, 100, 10, 2, 50, 0);
	CreateNewStoragaSubsustemUnit(1, 512, 196608, 3, 0, 100, 1000, 1, 1000, 100, 2, 500, 0);
	CreateNewStoragaSubsustemUnit(2, 512, 393216, 3, 0, 1000, 10000, 1, 10000, 1000, 2, 5000, 0);
	Pool.storage_pool_init(8388608, 6);
	Pool.add_storage_to_pool(0, 0, 3);
	Pool.add_storage_to_pool(1, 1, 3);
	Pool.add_storage_to_pool(2, 2, 3);
	Pool.create_logical_volume(0, 10, 512);
	Pool.create_logical_volume(1, 10, 512);
	Pool.create_logical_volume(2, 10, 512);
	Pool.send_io_to_pool(0, TempIo, {1,1});
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
