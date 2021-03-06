//
// Created by svalov on 8/16/16.
//

#ifndef MOARSTACK_MOARROUTINGPACKETMETADATA_H
#define MOARSTACK_MOARROUTINGPACKETMETADATA_H

#include <moarRouting.h>
#include <moarRoutingMessageId.h>
#include <moarTime.h>

#define STORED_PACK_STATE_BITS	8

#pragma pack(push, 1)

typedef enum{
	StoredPackState_Received,
	StoredPackState_InProcessing,
	StoredPackState_WaitSent,
	StoredPackState_WaitAck,
	StoredPackState_Disposed,
}StoredPackState_T;

typedef struct{
	RoutePackType_T 	PackType:ROUTE_PACK_TYPE_BITS;
	StoredPackState_T	State:STORED_PACK_STATE_BITS;

	RouteAddr_T			Source;
	RouteAddr_T			Destination;

	ChannelAddr_T 		LastHop;
	ChannelAddr_T 		NextHop;

	moarTime_T 			NextProcessing;

	void * 				Payload;
	PayloadSize_T 		PayloadSize;

	MessageId_T			InternalId;
	RoutingMessageId_T 	MessageId;
	// ttl/htl
	int 				TrysLeft;
	RouteXTL_T 			XTL;
}RouteStoredPacket_T;

#pragma pack(pop)

#endif //MOARSTACK_MOARROUTINGPACKETMETADATA_H
