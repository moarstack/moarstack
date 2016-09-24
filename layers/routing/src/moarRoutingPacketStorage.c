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
#include <priorityQueue.h>
#include <hashTable.h>

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

int psClear(PacketStorage_T* storage){
	if(NULL == storage)
		return FUNC_RESULT_FAILED_ARGUMENT;

	// cleanup
	while(storage->Count!=0)
	{
		RouteStoredPacket_T* packet = psGetTop(storage);
		int removeRes = psRemove(storage, packet);
		if(FUNC_RESULT_SUCCESS != removeRes)
			return removeRes;
	}
	return FUNC_RESULT_SUCCESS;
}

int psDeinit(PacketStorage_T* storage){
	if(NULL == storage)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int cleanRes = psClear(storage);
	if(FUNC_RESULT_SUCCESS != cleanRes)
		return cleanRes;
	// all structs should be clean

	//deinit queue
	int queueRes = pqDeinit(&(storage->NextProcessingTime));
	if(FUNC_RESULT_SUCCESS != queueRes)
		return queueRes;
	//deinit dest table
	int destRes = hashFree(&(storage->Destinations));
	if(FUNC_RESULT_SUCCESS != destRes)
		return destRes;
	//deinit mid table
	int midRes = hashFree(&(storage->MessageIds));
	if(FUNC_RESULT_SUCCESS != midRes)
		return midRes;
	//deinit rmid table
	int rmidRes = hashFree(&(storage->RoutingMessageIds));
	if(FUNC_RESULT_SUCCESS != rmidRes)
		return rmidRes;

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
		hashRemoveExact(&(storage->Destinations), &(allocatedPack->Destination), &allocatedPack);
		hashRemove(&(storage->MessageIds), &(allocatedPack->InternalId));
		hashRemove(&(storage->RoutingMessageIds), &(allocatedPack->MessageId));
		// free memory
		free(allocatedPack);
		return FUNC_RESULT_FAILED;
	}
	storage->Count++;
	return FUNC_RESULT_SUCCESS;
}

RouteStoredPacket_T* psFindPacketPtr(PacketStorage_T* storage, RouteStoredPacket_T* packet){
	if(NULL == storage)
		return NULL;
	if(NULL == packet)
		return NULL;

	RouteStoredPacket_T* allocatedPack;
	// find pack by mid
	int searchRes = hashGet(&(storage->MessageIds), &(packet->InternalId), &allocatedPack);
	if(FUNC_RESULT_SUCCESS != searchRes){
		// if can not find - search by rmid
		searchRes = hashGet(&(storage->RoutingMessageIds), &(packet->MessageId), &allocatedPack);
		if(FUNC_RESULT_SUCCESS != searchRes){
			//not found
			return NULL;
		}
	}
	return allocatedPack;
}

int psRemove(PacketStorage_T* storage, RouteStoredPacket_T* packet){
	if(NULL == storage)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;

	RouteStoredPacket_T* allocatedPack = psFindPacketPtr(storage, packet);
	if(NULL == allocatedPack)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// found, and remove
	pqRemove(&(storage->NextProcessingTime), &allocatedPack);
	hashRemoveExact(&(storage->Destinations), &(allocatedPack->Destination), &allocatedPack);
	hashRemove(&(storage->MessageIds), &(allocatedPack->InternalId));
	hashRemove(&(storage->RoutingMessageIds), &(allocatedPack->MessageId));
	free(allocatedPack);
	storage->Count--;
	//
	return FUNC_RESULT_SUCCESS;
}

RouteStoredPacket_T* psGetTop(PacketStorage_T* storage){
	if(NULL == storage)
		return NULL;
	if(0 == storage->Count)
		return NULL;

	return *((RouteStoredPacket_T**)pqTopData(&(storage->NextProcessingTime)));

}

RouteStoredPacket_T* psGetMidPtr(PacketStorage_T* storage, MessageId_T* mid){
	if(NULL == storage)
		return NULL;
	if(0 == storage->Count)
		return NULL;
	if(NULL == mid)
		return NULL;
	void* pack = hashGetPtr(&(storage->MessageIds), mid);
	if(pack!=NULL)
		return *((RouteStoredPacket_T**)pack);
	return NULL;
}
RouteStoredPacket_T* psGetRmidPtr(PacketStorage_T* storage, RoutingMessageId_T* rmid){
	if(NULL == storage)
		return NULL;
	if(0 == storage->Count)
		return NULL;
	if(NULL == rmid)
		return NULL;
	void* pack = hashGetPtr(&(storage->RoutingMessageIds), rmid);
	if(pack!=NULL)
		return *((RouteStoredPacket_T**)pack);
	return NULL;
}

int psGetMid(PacketStorage_T* storage, MessageId_T* mid, RouteStoredPacket_T* packet){
	if(NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RouteStoredPacket_T* pack = psGetMidPtr(storage, mid);
	if(NULL == pack)
		return FUNC_RESULT_FAILED;
	*packet = *pack;
	return FUNC_RESULT_SUCCESS;
}
int psGetRmid(PacketStorage_T* storage, RoutingMessageId_T* rmid, RouteStoredPacket_T* packet){
	if(NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RouteStoredPacket_T* pack = psGetRmidPtr(storage, rmid);
	if(NULL == pack)
		return FUNC_RESULT_FAILED;
	*packet = *pack;
	return FUNC_RESULT_SUCCESS;
}

int psGetDestFirst(PacketStorage_T* storage, RouteAddr_T* dest, hashIterator_T* iterator){
	if(NULL == storage)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == dest)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == iterator)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int res = hashGetFirst(&(storage->Destinations), dest, iterator);
	return  res;
}
RouteStoredPacket_T* psIteratorData(hashIterator_T* iterator){
	if(NULL == iterator)
		return NULL;
	RouteStoredPacket_T** packPtr = (RouteStoredPacket_T**)hashIteratorData(iterator);
	if(NULL == packPtr)
		return NULL;
	return *packPtr;

}

int psUpdateTime(PacketStorage_T* storage, RouteStoredPacket_T* packet){
	if(NULL == storage)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;

	RouteStoredPacket_T* allocatedPack = psFindPacketPtr(storage, packet);
	if(NULL == allocatedPack)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int res = pqChangePriority(&(storage->NextProcessingTime), &allocatedPack, &(packet->NextProcessing));
	return res;
}