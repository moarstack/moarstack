//
// Created by svalov on 8/13/16.
//

#include <moarRoutingPrivate.h>
#include <funcResults.h>
#include <moarRoutingCommand.h>
#include <moarRoutingStoredPacket.h>
#include <moarCommons.h>
#include <moarRoutingPacketStorage.h>
#include <moarRoutingStoredPacketFunc.h>

int processReceiveCommand(void* layerRef, int fd, LayerCommandStruct_T* command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RoutingLayer_T* layer = (RoutingLayer_T*)layerRef;
	//logic here
	RouteStoredPacket_T packet = {0};
	int prepareRes = prepareReceivedPacket(&packet, command->MetaData, command->Data, command->DataSize);
	if(FUNC_RESULT_SUCCESS != prepareRes)
		return prepareRes;

	packet.NextProcessing = timeGetCurrent();
	int storeRes = psAdd(&(layer->PacketStorage), &packet);
	if(FUNC_RESULT_SUCCESS != storeRes)
		clearStoredPacket(&packet);
	return storeRes;
}

int processMessageStateCommand(void* layerRef, int fd, LayerCommandStruct_T* command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RoutingLayer_T* layer = (RoutingLayer_T*)layerRef;
	//logic here
	return FUNC_RESULT_SUCCESS;
}

int processNewNeighborCommand(void* layerRef, int fd, LayerCommandStruct_T* command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RoutingLayer_T* layer = (RoutingLayer_T*)layerRef;
	//logic here
	return FUNC_RESULT_SUCCESS;
}

int processLostNeighborCommand(void* layerRef, int fd, LayerCommandStruct_T* command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RoutingLayer_T* layer = (RoutingLayer_T*)layerRef;
	//logic here
	return FUNC_RESULT_SUCCESS;
}

int processUpdateNeighborCommand(void* layerRef, int fd, LayerCommandStruct_T* command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RoutingLayer_T* layer = (RoutingLayer_T*)layerRef;
	//logic here
	return FUNC_RESULT_SUCCESS;
}

int processSendCommand( void * layerRef, int fd, LayerCommandStruct_T * command ) {
	int						result;
	RoutingLayer_T			* layer;
	RouteStoredPacket_T		storedPacket;

	if( NULL == layerRef || fd <= 0 || NULL == command || 0 == command->MetaSize || NULL == command->MetaData )
		return FUNC_RESULT_FAILED_ARGUMENT;

	layer = ( RoutingLayer_T * )layerRef;
	result = prepareSentPacket( &storedPacket, command->MetaData, command->Data, command->DataSize );

	storedPacket.NextProcessing = timeGetCurrent();
	int storeRes = psAdd(&(layer->PacketStorage), &storedPacket);
	if(FUNC_RESULT_SUCCESS != storeRes)
		clearStoredPacket(&storedPacket);

	return result;
}