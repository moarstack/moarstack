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

#define CHANNEL_REGISTER_RESULT_METADATA_SIZE 		sizeof( ChannelRegisterResultMetadata_T )
#define CHANNEL_UNREGISTER_RESULT_METADATA_SIZE 	sizeof( ChannelUnregisterResultMetadata_T )
#define CHANNEL_BEACON_UPDATE_METADATA_SIZE			sizeof( ChannelBeaconUpdateMetadata_T )
#define IFACE_PACK_STATE_METADATA_SIZE				sizeof( IfacePackStateMetadata_T )
#define IFACE_MODE_STATE_METADATA_SIZE				sizeof( IfaceModeStateMetadata_T )
#define IFACE_PACK_STATE_BITS	8
#define IFACE_MODE_STATE_BITS	8

#pragma pack(push, 1)

// type for length of universal address
typedef uint8_t UnIfaceAddrLen_T;

// possible states of packet when it is moving from interface to channel
typedef enum {
	IfacePackState_None, 		// not defined state of enum
	IfacePackState_Sending,		// current packet is sending now
	IfacePackState_Sent,		// current packet is sent and has no need to get response
	IfacePackState_Waiting,		// current packet is sent and now is waiting for response
	IfacePackState_Responsed,	// current packet was sent and got response in time
	IfacePackState_Timeouted,	// current packet was sent and got NO response in time
	IfacePackState_UnknownDest,	// current packet was not sent due to no neighbor info found
	IfacePackState_Notsent,		// current packet was not sent due to internal problems
	IfacePackState_Received		// current packet was received
} IfacePackState_T;

typedef enum {
	IfaceModeState_None,		// undefined interface state
	IfaceModeState_Sending,		// interface is sending something
	IfaceModeState_Waiting,		// interface is waiting for response (if needed)
	IfaceModeState_Receiving,	// interface is receiving something
	IfaceModeState_Responsing,	// interface is sending response (if needed)
	IfaceModeState_Ready		// interface is ready to send or receive
} IfaceModeState_T;

// metadata of packet state command
typedef struct {
	MessageId_T			Id;		// message identifier
	IfacePackState_T	State:IFACE_PACK_STATE_BITS;	// state of packet with specified identifier
} IfacePackStateMetadata_T;

// metadata of iface state command
typedef struct {
	IfaceModeState_T	State:IFACE_MODE_STATE_BITS; // current state of interface
} IfaceModeStateMetadata_T;

// metadata of command result of interface registering
typedef struct {
	bool	Registred;
} ChannelRegisterResultMetadata_T;

// metadata of command result of interface unregistering
typedef struct {
	// nothing is here yet
} ChannelUnregisterResultMetadata_T;

// metadata of command to update beacon payload
typedef struct {
	// nothing is here yet
} ChannelBeaconUpdateMetadata_T;

#pragma pack(pop)

#endif //MOARSTACK_MOARINTERFACECHANNEL_H
