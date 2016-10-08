//
// Created by svalov on 10/6/16.
//

#include <funcResults.h>
#include <moarRoutingStoredPacket.h>
#include "moarRoutingPacketProcessing.h"

int processPacketStorage(RoutingLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RouteStoredPacket_T* packet = psGetTop(&layer->PacketStorage);
	while(NULL != packet && timeCompare(packet->NextProcessing, timeGetCurrent()) <= 0){
		// logic

		switch(packet->State) {
			//if new packet
			case StoredPackState_Received:
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
						//// try to send
						// also multiple stage finders process here
						break;
					case RoutePackType_FinderAck:
						// process content
						// if destination
						//// dispose packet
						// else
						//// change state to processing
						break;
					case RoutePackType_Probe:
						// update tables
						// create new probe
						// send probe
						// dispose packet
						break;
					default:
						return FUNC_RESULT_FAILED;
				}
				break;
			// if already processed
			case StoredPackState_WaitSent:
				// timeout while send
				// change packet state to processing
				break;
			case StoredPackState_WaitAck:
				// timeout while waiting ack
				// change state to processing
				break;
			case StoredPackState_InProcessing:
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
				break;
			case StoredPackState_Disposed:
				// remove packet
				break;
			default:
				return FUNC_RESULT_FAILED;
		}
		packet = psGetTop(&layer->PacketStorage);
	}
	return FUNC_RESULT_SUCCESS;
}