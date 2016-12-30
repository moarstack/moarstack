//
// Created by kryvashek on 05.07.16.
//

#ifndef MOARSTACK_MOARCHANNELROUTING_H
#define MOARSTACK_MOARCHANNELROUTING_H

#include <stddef.h>
#include "moarCommons.h"
#include "moarChannel.h"
#include "moarRouting.h"
#include <moarMessageId.h>

#define	CHANNEL_RECEIVE_METADATA_SIZE	sizeof( ChannelReceiveMetadata_T )
#define	ROUTE_SEND_METADATA_SIZE		sizeof( RouteSendMetadata_T )
#define PACK_STATE_CHANNEL_BITS			8

#pragma pack(push, 1)

typedef uint8_t SendTrys_T;

// possible states of packet when it is moving from channel to routing
typedef enum {
	PackStateChannel_None, 			// not defined state of enum
	PackStateChannel_Sent,			// current packet is sent and has no need to get response
	PackStateChannel_NotSent,		// current packet is sent and now is waiting for response
	PackStateChannel_UnknownDest,	// current packet was not sent due to no neighbor info found
	PackStateChannel_Received		// current packet was received
} PackStateChannel_T;

// metadata of packet moving from channel to routing
typedef struct {
	MessageId_T Id;
	ChannelAddr_T	From;
} ChannelReceiveMetadata_T;

// metadata of packet moving from routing to channel
typedef struct {
	MessageId_T Id;
	ChannelAddr_T	Bridge;
} RouteSendMetadata_T;

typedef struct {
	MessageId_T Id;
	PackStateChannel_T State:PACK_STATE_CHANNEL_BITS;
	SendTrys_T SentTrys;
	ChannelAddr_T NeighborAddress;
} ChannelMessageStateMetadata_T;

typedef struct {
	ChannelAddr_T	Address;
} ChannelNeighborMetadata_T;

#pragma pack(pop)

#endif //MOARSTACK_MOARCHANNELROUTING_H
