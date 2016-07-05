//
// Created by kryvashek on 05.07.16.
//

#ifndef MOARSTACK_MOARINTERFACECHANNEL_H
#define MOARSTACK_MOARINTERFACECHANNEL_H

#include <stddef.h>

const size_t	IFACE_MSG_UP_SIZE = sizeof( IfaceMsgUp_T );
const size_t	CHANNEL_MSG_DOWN_SIZE = sizeof( ChannelMsgDown_T );

// possible states of packet when it is moving from interface to channel
typedef enum {
	PackStateIface_None, 		// not defined state of enum
	PackStateIface_Sending,		// current packet is sending now
	PackStateIface_Sent,		// current packet is sent and has no need to get response
	PackStateIface_Waiting,		// current packet is sent and now is waiting for response
	PackStateIface_Responsed,	// current packet was sent and got response in time
	PackStateIface_Timeouted,	// current packet was sent and got NO response in time
	PackStateIface_UnknownDest,	// current packet was not sent due to no neighbor info found
	PackStateIface_Received		// current packet was received
} PackStateIface_T;

// metadata of packet moving from interface to channel
typedef struct {
	PackStateIface_T	State;	// state of packet moving from interface to channel
	size_t				Size;	// size of payload
} IfaceMsgUp_T;

// metadata of packet moving from channel to interface
typedef struct {
	IfaceAddr_T	Bridge;
	size_t		Size;
} ChannelMsgDown_T;

#endif //MOARSTACK_MOARINTERFACECHANNEL_H
