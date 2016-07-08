//
// Created by kryvashek on 05.07.16.
//

#ifndef MOARSTACK_MOARCHANNELROUTING_H
#define MOARSTACK_MOARCHANNELROUTING_H

#include <stddef.h>
#include "moarCommons.h"
#include "moarChannel.h"
#include "moarRouting.h"

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
	PackStateChannel_T	State;	// state of packet moving from channel to routing
	size_t				Size;	// size of payload
} ChannelMsgUp_T;

// metadata of packet moving from routing to channel
typedef struct {
	ChannelAddr_T	Bridge;
	size_t			Size;
} RouteMsgDown_T;

const size_t	CHANNEL_MSG_UP_SIZE = sizeof( ChannelMsgUp_T );
const size_t	ROUTE_MSG_DOWN_SIZE = sizeof( RouteMsgDown_T );

#endif //MOARSTACK_MOARCHANNELROUTING_H
