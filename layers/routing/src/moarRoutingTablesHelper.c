//
// Created by svalov on 10/5/16.
//

#include "moarRoutingTablesHelper.h"
#include <funcResults.h>

int resetTimeForDest(RoutingLayer_T* layer, ChannelAddr_T* dest){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == dest)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//foreach packet with same dest
	hashIterator_T iterator = {0};
	psGetDestFirst(&(layer->PacketStorage),dest, &iterator);
	while(!hashIteratorIsLast(&iterator)){
		RouteStoredPacket_T* pack = psIteratorData(&iterator);
		pack->NextProcessing = timeGetCurrent();
		psUpdateTime(&(layer->PacketStorage), pack);
		hashIteratorNext(&iterator);
	}
	hashIteratorFree(&iterator);
	return FUNC_RESULT_SUCCESS;
}
int helperAddRoute(RoutingLayer_T* layer, RouteAddr_T* dest, RouteAddr_T* relay){
	// todo add table work
	int resetRes = resetTimeForDest(layer, dest);
	return FUNC_RESULT_FAILED;
}
int helperAddNeighbor(RoutingLayer_T* layer, ChannelAddr_T* address){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == address)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RoutingNeighborInfo_T neighborInfo = {0};
	neighborInfo.Address = *address;
	int res = storageAdd(&(layer->NeighborsStorage), &neighborInfo);
	if(FUNC_RESULT_SUCCESS!=res)
		return res;

	int resetRes = resetTimeForDest(layer, address);
	return res;
}
int helperRemoveRoute(RoutingLayer_T* layer, RouteAddr_T* dest, RouteAddr_T* relay){
	// todo add table work
	return FUNC_RESULT_FAILED;
}
int helperRemoveNeighbor(RoutingLayer_T* layer, ChannelAddr_T* address){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == address)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int res = storageRemove(&(layer->NeighborsStorage), address);
	return res;
}
int helperUpdateRoute(RoutingLayer_T* layer){
	// todo add table work
	return FUNC_RESULT_FAILED;
}
int helperUpdateNeighbor(RoutingLayer_T* layer){
	return FUNC_RESULT_FAILED;
}