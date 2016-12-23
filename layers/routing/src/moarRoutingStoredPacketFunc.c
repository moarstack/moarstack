//
// Created by svalov on 8/17/16.
//

#include <moarRouting.h>
#include <moarRoutingPrivate.h>
#include <moarChannelRouting.h>
#include <moarRoutingStoredPacket.h>
#include <moarRoutingStoredPacketFunc.h>
#include <funcResults.h>
#include <stdlib.h>
#include <memory.h>
#include <moarCommons.h>


// отчистка пакета
int clearStoredPacket(RouteStoredPacket_T* packet){
	if(NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;

	free(packet->Payload);
	packet->Payload = NULL;
	packet->State = StoredPackState_Disposed;
	return FUNC_RESULT_SUCCESS;
}

// убийство пакета
int disposeStoredPacket(RouteStoredPacket_T** packet){
	if(NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int res = clearStoredPacket(*packet);
	free(*packet);
	*packet = NULL;

	return res;
}

// отправка пакета вниз
int sendPacketToChannel(RoutingLayer_T* layer, RouteStoredPacket_T* packet){
	if(NULL == layer|| NULL == packet || 0 >= layer->ChannelSocket || 0 >= packet->XTL ) // won`t send if XTL exposed, but lets processing be done (all routine before calling that function)
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
	header->XTL = packet->XTL;
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
	if(FUNC_RESULT_SUCCESS == sendRes)
		packet->State = StoredPackState_WaitSent;
	// free allocated memory
	free(newData);
	return sendRes;
}

// отправка наверх
int sendPacketToPresentation( RoutingLayer_T * layer, RouteStoredPacket_T * packet ) {
	int						result;
	LayerCommandStruct_T	command = { 0 };
	RouteReceivedMetadata_T	metadata = { 0 };

	if( NULL == layer || NULL == packet || 0 >= layer->PresentationSocket )
		return FUNC_RESULT_FAILED_ARGUMENT;

	command.Command = LayerCommandType_Receive;

	command.Data = packet->Payload;
	command.DataSize = packet->PayloadSize;

	metadata.From = packet->Source;
	metadata.Id = packet->InternalId;
	command.MetaData = &metadata;
	command.MetaSize = sizeof( RouteReceivedMetadata_T );

	result = WriteCommand( layer->PresentationSocket, &command );

	return result;
}

// обработка принятого снизу пакета
int prepareReceivedPacket( RouteStoredPacket_T* packet, ChannelReceiveMetadata_T* metadata, void* data, PayloadSize_T dataSize){
	if(NULL == packet || NULL == metadata || NULL == data || dataSize < ROUTING_HEADER_SIZE)
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
	packet->XTL = header->XTL - DEFAULT_XTL_STEP; // decreasing packet`s XTL due to being receiver of that packet => it made hop
	// additional
	packet->NextProcessing = timeGetCurrent();
	packet->State = StoredPackState_Received;
	packet->TrysLeft = DEFAULT_ROUTE_TRYS;
	// TODO add metrics here
	return FUNC_RESULT_SUCCESS;
}

int prepareSentPacket(RoutingLayer_T* layer, RouteStoredPacket_T * packet, PresentSendMetadata_T * metadata, void * data, PayloadSize_T dataSize ) {

	int	result;

	if( NULL == packet || NULL == metadata || NULL == data || 0 == dataSize || NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;

	memset( packet, 0, sizeof( RouteStoredPacket_T ) );
	result = rmidGenerate( &( packet->MessageId ) );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	packet->Payload = malloc( dataSize );

	if( NULL == packet->Payload )
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;

	memcpy( packet->Payload, data, dataSize );
	packet->PayloadSize = dataSize;
	packet->PackType = RoutePackType_Data;
	packet->NextProcessing = timeGetCurrent();
	packet->Destination = metadata->Destination;
	packet->InternalId = metadata->Id;
	packet->State = StoredPackState_Received; // received (=got) by routing from its input
	packet->TrysLeft = DEFAULT_ROUTE_TRYS;
	packet->Source = layer->LocalAddress;
	packet->XTL = DEFAULT_XTL_DATA;
		// no LastHop due to current hop being first
		// no NextHop because its routing work to know such things

	return FUNC_RESULT_SUCCESS;
}