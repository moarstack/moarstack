//
// Created by svalov on 8/17/16.
//
#include <moarRoutingOutput.h>
#include <funcResults.h>
#include <moarRoutingPrivate.h>
#include <stdlib.h>
#include <moarRoutingStoredPacket.h>
#include <moarRouting.h>
#include <moarCommons.h>
#include <memory.h>

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