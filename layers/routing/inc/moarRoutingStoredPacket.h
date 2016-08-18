//
// Created by svalov on 8/16/16.
//

#ifndef MOARSTACK_MOARROUTINGPACKETMETADATA_H
#define MOARSTACK_MOARROUTINGPACKETMETADATA_H

#include <moarRoutingPrivate.h>
#include <moarRoutingMessageId.h>
#include <moarTime.h>

typedef struct{
	RoutePackType_T 	PackType;

	RouteAddr_T			Source;
	RouteAddr_T			Destination;

	ChannelAddr_T 		LastHop;
	ChannelAddr_T 		NextHop;

	moarTime_T 			NextProcessing;

	void * 				Payload;
	PayloadSize_T 		PayloadSize;

	MessageId_T			InternalId;
	RoutingMessageId_T 	MessageId;
	//ttl/htl
}RouteStoredPacket_T;

int clearStoredPacket(RouteStoredPacket_T* packet);
int disposeStoredPacket(RouteStoredPacket_T** packet);

int prepareReceivedPacket(RouteStoredPacket_T* packet, ChannelReceiveMetadata_T* metadata, void* data, PayloadSize_T dataSize);
//int processInputFromPresentation(RouteStoredPacket_T* packet, PresentationSendMetadata_T* metadata, void* data, PayloadSize_T dataSize);
int sendPacketToChannel(RoutingLayer_T* layer, RouteStoredPacket_T* packet);
//int sendPacketToPresentation(RoutingLayer_T* layer, RouteStoredPacket_T* packet);

#endif //MOARSTACK_MOARROUTINGPACKETMETADATA_H
