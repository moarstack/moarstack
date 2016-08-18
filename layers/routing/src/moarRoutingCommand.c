//
// Created by svalov on 8/13/16.
//

#include <moarRoutingPrivate.h>
#include <funcResults.h>
#include <moarRoutingCommand.h>

int processReceiveCommand(void* layerRef, int fd, LayerCommandStruct_T* command){
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
	PresentSendMetadata_T	* metadata;
	RouteStoredPacket_T		storedPacket;

	if( NULL == layerRef || fd <= 0 || NULL == command || 0 == command->MetaSize || NULL == command->MetaData )
		return FUNC_RESULT_FAILED_ARGUMENT;

	layer = ( RoutingLayer_T * )layerRef;
	metadata = ( PresentSendMetadata_T * )command->MetaData;

	if( 0 == command->DataSize || NULL == command->Data ) { // if no meaningfull data
		// inform presentation about not sent message
	} else { //
		storedPacket.Destination = metadata->Destination;
		storedPacket.InternalId = metadata->Id;
		result = rmidGenerate( &( storedPacket.MessageId ) );
	}


	return FUNC_RESULT_SUCCESS;
}