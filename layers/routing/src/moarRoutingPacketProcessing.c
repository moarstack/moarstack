//
// Created by svalov on 10/6/16.
//

#include "moarRoutingPacketProcessing.h"

int processPacketStorage(RoutingLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RouteStoredPacket_T* packet = psGetTop(&layer->PacketStorage);
	while(timeCompare(packet, timeGetCurrent()) <= 0){
		packet = psGetTop(&layer->PacketStorage);
	}
	return FUNC_RESULT_SUCCESS;
}