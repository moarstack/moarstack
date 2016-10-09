//
// Created by svalov on 10/6/16.
//

#include <funcResults.h>
#include <moarRoutingStoredPacket.h>
#include <moarRoutingPrivate.h>
#include "moarRoutingPacketProcessing.h"

int processReceivedDataPacket(RoutingLayer_T* layer, RouteStoredPacket_T* packet) {
	if (NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (RoutePackType_Data != packet->PackType)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = FUNC_RESULT_FAILED;
	// if destination
	//// forward up
	//// create ack
	//// add ack with processing state
	//// dispose packet
	// else
	//// change state to processing
	return res;
}
int processReceivedAckPacket(RoutingLayer_T* layer, RouteStoredPacket_T* packet) {
	if (NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (RoutePackType_Ack != packet->PackType)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = FUNC_RESULT_FAILED;
	// if destination
	//// forward up event
	//// dispose packet
	// else
	//// change state to processing
	return res;
}
int processReceivedFinderAckPacket(RoutingLayer_T* layer, RouteStoredPacket_T* packet) {
	if (NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (RoutePackType_FinderAck != packet->PackType)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = FUNC_RESULT_FAILED;
	// process content
	// if destination
	//// dispose packet
	// else
	//// create new packet
	//// add with processing state
	return res;
}
int processReceivedFinderPacket(RoutingLayer_T* layer, RouteStoredPacket_T* packet) {
	if (NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (RoutePackType_Finder != packet->PackType)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = FUNC_RESULT_FAILED;
	// process content
	// if destination
	//// create finder ack
	//// add finder ack with procesing state
	//// dispose packet
	// else
	//// create new packet
	//// try to send
	// also multiple stage finders process here
	return res;
}
int processReceivedProbePacket(RoutingLayer_T* layer, RouteStoredPacket_T* packet) {
	if (NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (RoutePackType_Probe != packet->PackType)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = FUNC_RESULT_FAILED;
	// update tables
	// create new probe
	// send probe
	// dispose packet
	return res;
}

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
			res = processReceivedDataPacket(layer, packet);
			break;
		case RoutePackType_Ack:
			res = processReceivedAckPacket(layer, packet);
			break;
		case RoutePackType_Finder:
			res = processReceivedFinderPacket(layer, packet);
			break;
		case RoutePackType_FinderAck:
			res = processReceivedFinderAckPacket(layer, packet);
			break;
		case RoutePackType_Probe:
			res = processReceivedProbePacket(layer, packet);
			break;
		default:
			res = FUNC_RESULT_FAILED_ARGUMENT;
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