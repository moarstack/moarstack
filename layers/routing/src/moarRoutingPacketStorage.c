//
// Created by svalov on 9/14/16.
//

#include "moarRoutingPacketStorage.h"
#include <funcResults.h>
#include <hashFunc.h>
#include <moarMessageId.h>
#include <moarRoutingMessageId.h>
#include <moarRoutingPacketStorage.h>
#include <memory.h>
#include <moarRoutingStoredPacket.h>

int timeCompareFunction(void* k1, void* k2, size_t size){
	if(k1 == NULL)
		return 0;
	if(k2 == NULL)
		return 0;
	moarTime_T time1 = *((moarTime_T*)k1);
	moarTime_T time2 = *((moarTime_T*)k2);
	return -timeCompare(time1,time2);
}

int psInit(PacketStorage_T* storage){
	if(NULL == storage)
		return FUNC_RESULT_FAILED_ARGUMENT;

	//init queue
	int queueRes = pqInit(&(storage->NextProcessingTime), QUEUE_SIZE, timeCompareFunction,
						  sizeof(moarTime_T), sizeof(RouteStoredPacket_T*));
	if(FUNC_RESULT_SUCCESS != queueRes)
		return queueRes;
	//init dest table
	int destRes = hashInit(&(storage->Destinations), hashRoutingAddress, DEST_HASH_SIZE,
						   sizeof(RouteAddr_T), sizeof(RouteStoredPacket_T*));
	if(FUNC_RESULT_SUCCESS != destRes)
		return destRes;
	//init mid table
	int midRes = hashInit(&(storage->MessageIds), midHash, MID_HASH_SIZE,
						   sizeof(MessageId_T), sizeof(RouteStoredPacket_T*));
	if(FUNC_RESULT_SUCCESS != midRes)
		return midRes;
	//init rmid table
	int rmidRes = hashInit(&(storage->RoutingMessageIds), rmidHash, RMID_HASH_SIZE,
						  sizeof(RoutingMessageId_T), sizeof(RouteStoredPacket_T*));
	if(FUNC_RESULT_SUCCESS != rmidRes)
		return rmidRes;

	storage->Count = 0;

	return FUNC_RESULT_SUCCESS;
}

int psDeinit(PacketStorage_T* storage){
	if(NULL == storage)
		return FUNC_RESULT_FAILED_ARGUMENT;

	//init queue
	int queueRes = pqDeinit(&(storage->NextProcessingTime));
	if(FUNC_RESULT_SUCCESS != queueRes)
		return queueRes;
	//init dest table
	int destRes = hashFree(&(storage->Destinations));
	if(FUNC_RESULT_SUCCESS != destRes)
		return destRes;
	//init mid table
	int midRes = hashFree(&(storage->MessageIds));
	if(FUNC_RESULT_SUCCESS != midRes)
		return midRes;
	//init rmid table
	int rmidRes = hashFree(&(storage->RoutingMessageIds));
	if(FUNC_RESULT_SUCCESS != rmidRes)
		return rmidRes;

	storage->Count = 0;

	return FUNC_RESULT_SUCCESS;
}

int psAdd(PacketStorage_T* storage, RouteStoredPacket_T* packet){
	if(NULL == storage)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;

	// allocate memory and copy
	RouteStoredPacket_T* allocatedPack = malloc(sizeof(RouteStoredPacket_T));
	if(NULL == allocatedPack)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	memcpy(allocatedPack, packet, sizeof(RouteStoredPacket_T));

	// add to queue
	int queueRes = pqEnqueue(&(storage->NextProcessingTime), &(allocatedPack->NextProcessing), &allocatedPack);
	// add to dest
	int destRes = hashAdd(&(storage->Destinations), &(allocatedPack->Destination), &allocatedPack);
	// add to mid
	int midRes = hashAdd(&(storage->MessageIds), &(allocatedPack->InternalId), &allocatedPack);
	// add to rmid
	int rmidRes = hashAdd(&(storage->RoutingMessageIds), &(allocatedPack->MessageId), &allocatedPack);

	//todo think about errors while adding

	if(FUNC_RESULT_SUCCESS != queueRes ||
	   		FUNC_RESULT_SUCCESS != destRes ||
			FUNC_RESULT_SUCCESS != midRes ||
			FUNC_RESULT_SUCCESS != rmidRes){
		//remove
		pqRemove(&(storage->NextProcessingTime), &allocatedPack);
		if(FUNC_RESULT_SUCCESS == destRes)
			hashRemove(&(storage->Destinations), &(allocatedPack->Destination));
		// TODO need remove exact function
		hashRemove(&(storage->MessageIds), &(allocatedPack->InternalId));
		hashRemove(&(storage->RoutingMessageIds), &(allocatedPack->MessageId));
		// free memory
		free(allocatedPack);
		return FUNC_RESULT_FAILED;
	}

	return FUNC_RESULT_SUCCESS;
}

int psRemove(PacketStorage_T* storage, RouteStoredPacket_T* packet){
	if(NULL == storage)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;

	RouteStoredPacket_T* allocatedPack;
	// find pack by mid
	// if can not find - search by rmid

	//

}