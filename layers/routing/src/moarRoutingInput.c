//
// Created by svalov on 8/16/16.
//

#include <moarRoutingInput.h>
#include <funcResults.h>
#include <memory.h>
#include <moarRoutingStoredPacket.h>
#include <moarRouting.h>
#include <stdlib.h>
#include <moarRoutingPrivate.h>

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
