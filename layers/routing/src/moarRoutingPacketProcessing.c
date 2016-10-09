//
// Created by svalov on 10/6/16.
//

#include <funcResults.h>
#include <moarRoutingStoredPacket.h>
#include <moarRoutingPrivate.h>
#include "moarRoutingPacketProcessing.h"

int processReceivedPacket(RoutingLayer_T* layer, RouteStoredPacket_T* packet){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == packet)
		return  FUNC_RESULT_FAILED_ARGUMENT;
	if(StoredPackState_Received != packet->State)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int res = FUNC_RESULT_FAILED;
	switch (packet->PackType) {
		case RoutePackType_Data:
			// if destination
			//// forward up
			//// create ack
			//// add ack with processing state
			//// dispose packet
			// else
			//// change state to processing
			break;
		case RoutePackType_Ack:
			// if destination
			//// forward up event
			//// dispose packet
			// else
			//// change state to processing
			break;
		case RoutePackType_Finder:
			// process content
			// if destination
			//// create finder ack
			//// add finder ack with procesing state
			//// dispose packet
			// else
			//// create new packet
			//// try to send
			// also multiple stage finders process here
			break;
		case RoutePackType_FinderAck:
			// process content
			// if destination
			//// dispose packet
			// else
			//// create new packet
			//// add with processing state
			break;
		case RoutePackType_Probe:
			// update tables
			// create new probe
			// send probe
			// dispose packet
			break;
		default:
			res = FUNC_RESULT_FAILED;
	}
	return res;
}
int processInProcessingPacket(RoutingLayer_T* layer, RouteStoredPacket_T* packet){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == packet)
		return  FUNC_RESULT_FAILED_ARGUMENT;
	int res = FUNC_RESULT_FAILED;
	if(StoredPackState_InProcessing != packet->State)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// if no trys left
	//// dispose packet
	// try to find route || neighbor
	// if found
	//// send to relay
	//// change state to wait sent
	//// update timeout
	// else
	//// update timeout
	//// send finder
	return res;
}
int processWaitSentPacket(RoutingLayer_T* layer, RouteStoredPacket_T* packet){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == packet)
		return  FUNC_RESULT_FAILED_ARGUMENT;
	int res = FUNC_RESULT_FAILED;
	if(StoredPackState_WaitSent != packet->State)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// timeout while send
	// change packet state to processing
	return res;
}
int processWaitAckPacket(RoutingLayer_T* layer, RouteStoredPacket_T* packet){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == packet)
		return  FUNC_RESULT_FAILED_ARGUMENT;
	int res = FUNC_RESULT_FAILED;
	if(StoredPackState_WaitAck != packet->State)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// timeout while waiting ack
	// decrease route metric
	// change state to processing
	return res;
}
int processDisposedPacket(RoutingLayer_T* layer, RouteStoredPacket_T* packet){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == packet)
		return  FUNC_RESULT_FAILED_ARGUMENT;
	int res = FUNC_RESULT_FAILED;
	if(StoredPackState_Disposed != packet->State)
		return FUNC_RESULT_FAILED_ARGUMENT;
	res = psRemove(&layer->PacketStorage,packet);
	return res;
}
int processPacketStorage(RoutingLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RouteStoredPacket_T* packet = psGetTop(&layer->PacketStorage);
	while(NULL != packet && timeCompare(packet->NextProcessing, timeGetCurrent()) <= 0){
		// logic
		int res = FUNC_RESULT_FAILED;
		switch(packet->State) {
			//if new packet
			case StoredPackState_Received:
				res = processReceivedPacket(layer, packet);
				break;
			// if data already processed
			case StoredPackState_WaitSent:
				res = processWaitSentPacket(layer, packet);
				break;
			case StoredPackState_WaitAck:
				res = processWaitAckPacket(layer, packet);
				break;
			case StoredPackState_InProcessing:
				res = processInProcessingPacket(layer, packet);
				break;
			case StoredPackState_Disposed:
				res = processDisposedPacket(layer, packet);
				break;
			default:
				res = FUNC_RESULT_FAILED;
		}
		if(FUNC_RESULT_SUCCESS != res)
			return res;
		packet = psGetTop(&layer->PacketStorage);
	}
	return FUNC_RESULT_SUCCESS;
}