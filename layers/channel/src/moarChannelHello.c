//
// Created by svalov on 8/10/16.
//

#include "moarChannelHello.h"
#include <moarChannelMetadata.h>
#include <memory.h>


int channelHelloFill(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL != layer->HelloMessage) {
		free(layer->HelloMessage);
		layer->HelloMessageSize = 0;
	}
	layer->HelloMessage = malloc(CHANNEL_LAYER_HEADER_SIZE + sizeof(ChannelHelloMessagePayload_T));
	layer->HelloMessageSize = CHANNEL_LAYER_HEADER_SIZE + sizeof(ChannelHelloMessagePayload_T);
	if(NULL == layer->HelloMessage)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	ChannelLayerHeader_T* header = (ChannelLayerHeader_T*)layer->HelloMessage;
	ChannelHelloMessagePayload_T* hello = (ChannelHelloMessagePayload_T*)(layer->HelloMessage+CHANNEL_LAYER_HEADER_SIZE);
	//fill in header
	header->From = layer->LocalAddress;
	header->Hello = true;
	header->PayloadSize = sizeof(ChannelHelloMessagePayload_T);
	//fill hello

	return FUNC_RESULT_SUCCESS;
}
int channelHelloUpdateInterface(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//create update structure
	ChannelBeaconUpdateMetadata_T updateMetadata;
	memset( &updateMetadata, 0, sizeof( ChannelBeaconUpdateMetadata_T ) );
	//fill metadata
	//create command
	LayerCommandStruct_T updateCommand = {0};
	memset( &updateCommand, 0, sizeof( LayerCommandStruct_T ) );
	updateCommand.Command = LayerCommandType_UpdateBeaconPayload;
	updateCommand.MetaData = &updateMetadata;
	updateCommand.MetaSize = sizeof(ChannelBeaconUpdateMetadata_T);
	updateCommand.Data = layer->HelloMessage;
	updateCommand.DataSize = layer->HelloMessageSize;
	//foreach interface
	hashIterator_T iterator = {0};
	hashIterator(&(layer->Interfaces), &iterator);
	while(!hashIteratorEnded( &iterator )){
		InterfaceDescriptor_T* interface = (InterfaceDescriptor_T*)hashIteratorData(&iterator);
		//send
		int res = WriteCommand(interface->Socket, &updateCommand);
		if(FUNC_RESULT_SUCCESS != res)
			return res;
		hashIteratorNext(&iterator);
	}
	return FUNC_RESULT_SUCCESS;
}
int channelHelloProcess(ChannelLayer_T* layer, PayloadSize_T size, void* data){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == data)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 == size)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(CHANNEL_LAYER_HEADER_SIZE + sizeof(ChannelHelloMessagePayload_T) != size)
		return FUNC_RESULT_FAILED_ARGUMENT;
	ChannelLayerHeader_T* header = (ChannelLayerHeader_T*)data;
	ChannelHelloMessagePayload_T* hello = (ChannelHelloMessagePayload_T*)(data+CHANNEL_LAYER_HEADER_SIZE);
	//process

	return FUNC_RESULT_SUCCESS;
}

int channelHelloSendToNeighbor(ChannelLayer_T* layer, UnIfaceAddr_T* address, InterfaceDescriptor_T* bridge){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == address)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == bridge)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == layer->HelloMessage){
		int res = channelHelloFill(layer);
		if(FUNC_RESULT_SUCCESS != res)
			return res;
	}
	ChannelSendMetadata_T sendMetadata = {0};
	int genRes = midGenerate(&(sendMetadata.Id),MoarLayer_Channel);
	if(FUNC_RESULT_SUCCESS != genRes)
		return genRes;
	sendMetadata.NeedResponse = HELLO_NEED_RESPONSE;
	sendMetadata.Bridge = *address;
	int sendRes = writeSendMetadata(bridge->Socket, &sendMetadata, layer->HelloMessageSize, layer->HelloMessage);
	return sendRes;
}
