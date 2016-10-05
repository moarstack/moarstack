//
// Created by svalov on 8/16/16.
//

#ifndef MOARSTACK_MOARROUTINGPACKETMETADATA_H
#define MOARSTACK_MOARROUTINGPACKETMETADATA_H

#include <moarRouting.h>
#include <moarRoutingMessageId.h>
#include <moarTime.h>

typedef enum{
	StoredPackState_Received,
	StoredPackState_InProcessing,
	StoredPackState_WaitSent,
	StoredPackState_WaitAck,
	StoredPackState_Disposed,
}StoredPackState_T;

typedef struct{
	RoutePackType_T 	PackType;
	StoredPackState_T	State;

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

#endif //MOARSTACK_MOARROUTINGPACKETMETADATA_H
