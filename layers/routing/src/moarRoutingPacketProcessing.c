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
		//if new packet
		switch(packet->State) {
			case StoredPackState_Received:
				switch (packet->PackType) {
					case RoutePackType_Data:
						break;
					case RoutePackType_Ack:
						break;
					case RoutePackType_Finder:
						break;
					case RoutePackType_FinderAck:
						break;
					case RoutePackType_Probe:
						break;
					default:
						return FUNC_RESULT_FAILED;
				}
				break;
			case StoredPackState_WaitSent:
				break;
			case StoredPackState_WaitAck:
				break;
			case StoredPackState_InProcessing:
				break;
			case StoredPackState_Disposed:
				break;
			default:
				return FUNC_RESULT_FAILED;
		}
		packet = psGetTop(&layer->PacketStorage);
	}
	return FUNC_RESULT_SUCCESS;
}