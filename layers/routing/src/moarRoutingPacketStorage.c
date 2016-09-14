//
// Created by svalov on 9/14/16.
//

#include "moarRoutingPacketStorage.h"
#include <funcResults.h>
#include <hashFunc.h>
#include <moarMessageId.h>
#include <moarRoutingMessageId.h>
#include <moarRoutingPacketStorage.h>

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
						  sizeof(moarTime_T), sizeof(RouteStoredPacket_T));
	if(FUNC_RESULT_SUCCESS != queueRes)
		return queueRes;
	//init dest table
	int destRes = hashInit(&(storage->Destinations), hashRoutingAddress, DEST_HASH_SIZE,
						   sizeof(RouteAddr_T), sizeof(RouteStoredPacket_T));
	if(FUNC_RESULT_SUCCESS != destRes)
		return destRes;
	//init mid table
	int midRes = hashInit(&(storage->MessageIds), midHash, MID_HASH_SIZE,
						   sizeof(MessageId_T), sizeof(RouteStoredPacket_T));
	if(FUNC_RESULT_SUCCESS != midRes)
		return midRes;
	//init rmid table
	int rmidRes = hashInit(&(storage->RoutingMessageIds), rmidHash, RMID_HASH_SIZE,
						  sizeof(RoutingMessageId_T), sizeof(RouteStoredPacket_T));
	if(FUNC_RESULT_SUCCESS != rmidRes)
		return rmidRes;

	storage->Count = 0;

	return FUNC_RESULT_SUCCESS;
}

