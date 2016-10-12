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
#include <moarChannelRouting.h>
#include <moarRoutingTablesHelper.h>

// прилетело сообщение снизу
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

// состояние сообщения(отправлено/не отправлено)
int processMessageStateCommand(void* layerRef, int fd, LayerCommandStruct_T* command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RoutingLayer_T* layer = (RoutingLayer_T*)layerRef;
	ChannelMessageStateMetadata_T* metadata = (ChannelMessageStateMetadata_T*)command->MetaData;
	//logic here
	RouteStoredPacket_T* pack = psGetTop(&(layer->PacketStorage));
	if(NULL != pack) {
		// todo review logic of neighbors/routetable update not send
		switch (metadata->State) {
			case PackStateChannel_Sent:
				// add timeout of ack waiting
				pack->State = StoredPackState_WaitAck;
				// change state
				pack->NextProcessing = timeAddInterval(timeGetCurrent(), ACK_WAITING_TIMEOUT); // todo calculate timeout from stats
				break;
			case PackStateChannel_NotSent:
			case PackStateChannel_UnknownDest:
				// set to processing
				pack->State = StoredPackState_InProcessing;
				// set next processing to current
				pack->NextProcessing = timeGetCurrent();
				break;
			default:
				return FUNC_RESULT_FAILED;
		}
		int updateRes = psUpdateTime(&(layer->PacketStorage), pack);
	}
	return FUNC_RESULT_SUCCESS;
}

// новый сосед появился
int processNewNeighborCommand(void* layerRef, int fd, LayerCommandStruct_T* command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RoutingLayer_T* layer = (RoutingLayer_T*)layerRef;
	ChannelNeighborMetadata_T* metadata = (ChannelNeighborMetadata_T*)command->MetaData;
	//logic here
	int res = helperAddNeighbor(layer,&(metadata->Address));
	return res;
}

// сосед пропал
int processLostNeighborCommand(void* layerRef, int fd, LayerCommandStruct_T* command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RoutingLayer_T* layer = (RoutingLayer_T*)layerRef;
	ChannelNeighborMetadata_T* metadata = (ChannelNeighborMetadata_T*)command->MetaData;
	//logic here
	int res = helperRemoveNeighbor(layer,&(metadata->Address));
	return res;

}

// сосед изменился
int processUpdateNeighborCommand(void* layerRef, int fd, LayerCommandStruct_T* command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RoutingLayer_T* layer = (RoutingLayer_T*)layerRef;
	ChannelNeighborMetadata_T* metadata = (ChannelNeighborMetadata_T*)command->MetaData;
	//logic here
	//todo add update here
	return FUNC_RESULT_SUCCESS;
}

// обработка команды "послать" от верхнего уровня
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