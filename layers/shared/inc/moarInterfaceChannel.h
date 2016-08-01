//
// Created by kryvashek on 05.07.16.
//

#ifndef MOARSTACK_MOARINTERFACECHANNEL_H
#define MOARSTACK_MOARINTERFACECHANNEL_H

#include <stddef.h>
#include <stdbool.h>
#include <moarCommons.h>
#include <moarInterface.h>
#include <moarChannel.h>

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

// type for length of universal address
typedef uint8_t UnIfaceAddrLen_T;


typedef struct {
	MessageId_T Id;
	PackStateIface_T	State;	// state of packet moving from interface to channel
} InterfaceStateMetadata_T;


typedef struct {
	bool Registred;
} ChannelRegisterResultMetadata_T;

typedef struct {

} ChannelUnregisterResultMetadata_T;
#define CHANNEL_REGISTER_RESULT_METADATA_SIZE 		sizeof(ChannelRegisterResultMetadata_T)
#define CHANNEL_UNREGISTER_RESULT_METADATA_SIZE 	sizeof(ChannelUnregisterResultMetadata_T)
#endif //MOARSTACK_MOARINTERFACECHANNEL_H
