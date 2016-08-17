//
// Created by svalov on 8/17/16.
//

#include <moarRouting.h>
#include <moarRoutingPrivate.h>
#include <moarChannelRouting.h>
#include <moarRoutingStoredPacket.h>
#include <funcResults.h>
#include <stdlib.h>
#include <memory.h>


int disposeStoredPacketData(RouteStoredPacket_T* packet){
	if(NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;

	free(packet->Payload);
	packet->Payload = NULL;

	return FUNC_RESULT_SUCCESS;
}
int disposeStoredPacket(RouteStoredPacket_T** packet){
	if(NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int res = disposeStoredPacketData(*packet);
	free(*packet);
	*packet = NULL;

	return res;
}

int sendPacketToChannel(RoutingLayer_T* layer, RouteStoredPacket_T* packet){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(layer->ChannelSocket <=0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// allocate memory for data
	void* newData = malloc(packet->PayloadSize + ROUTING_HEADER_SIZE);
	if(NULL == newData)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	//copy
	memcpy(newData+ROUTING_HEADER_SIZE, packet->Payload, packet->PayloadSize);
	// get header
	RoutingHeader_T* header = newData;
	// fill header
	header->PayloadSize = packet->PayloadSize;
	header->PacketType = packet->PackType;
	header->Destination = packet->Destination;
	header->Source = packet->Source;
	header->Id = packet->MessageId;
	// make send metadata
	RouteSendMetadata_T metadata = {0};
	metadata.Id = packet->InternalId;
	metadata.Bridge = packet->NextHop;
	// make send command
	LayerCommandStruct_T command = {0};
	command.Command = LayerCommandType_Send;
	command.Data = newData;
	command.DataSize = packet->PayloadSize + ROUTING_HEADER_SIZE;
	command.MetaData = &metadata;
	command.MetaSize = sizeof(RouteSendMetadata_T);
	// send
	int sendRes = WriteCommand(layer->ChannelSocket, &command);
	// free allocated memory
	free(newData);
	return sendRes;
}

int processInputFromChannel(RouteStoredPacket_T* packet, ChannelReceiveMetadata_T* metadata, void* data, PayloadSize_T dataSize){
	if(NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == metadata)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == data)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(dataSize < ROUTING_HEADER_SIZE)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//get header
	RoutingHeader_T* header = (RoutingHeader_T*)data;
	//check for size
	if(header->PayloadSize + ROUTING_HEADER_SIZE != dataSize)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// clean
	memset(packet, 0, sizeof(RouteStoredPacket_T));
	// allocate payload
	packet->Payload = malloc(header->PayloadSize);
	if(NULL == packet->Payload)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	// copy
	memcpy(packet->Payload, data+ROUTING_HEADER_SIZE, header->PayloadSize);
	// set size
	packet->PayloadSize = header->PayloadSize;
	// fill from metadata
	packet->LastHop = metadata->From;
	packet->InternalId = metadata->Id;
	// fill from header
	packet->Destination = header->Destination;
	packet->Source = header->Source;
	packet->MessageId = header->Id;
	packet->PackType = header->PacketType;
	// additional
	packet->NextProcessing = timeGetCurrent();
	// TODO add metrics here
	return FUNC_RESULT_SUCCESS;
}