//
// Created by kryvashek on 09.07.16.
// for private notations

#ifndef MOARSTACK_MOARINTERFACEPRIVATE_H
#define MOARSTACK_MOARINTERFACEPRIVATE_H

#include <stdint.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <math.h>

#include <moarLayerEntryPoint.h>
#include <moarCommons.h>
#include <moarInterface.h>
#include <moarInterfaceChannel.h>
#include <funcResults.h>

#define IFACE_ADDR_SIZE					sizeof( unsigned int )
#define IFACE_HEADER_SIZE				sizeof( IfaceHeader_T )
#define IFACE_FOOTER_SIZE				sizeof( IfaceFooter_T )
#define CHANNEL_SEND_METADATA_SIZE		sizeof( ChannelSendMetadata_T )
#define IFACE_RECEIVE_METADATA_SIZE		sizeof( IfaceReceiveMetadata_T )
#define IFACE_NEIGHBOR_METADATA_SIZE	sizeof( IfaceNeighborMetadata_T )
#define IFACE_REGISTER_METADATA_SIZE	sizeof( IfaceRegisterMetadata_T )
#define IFACE_UNREGISTER_METADATA_SIZE	sizeof( IfaceUnregisterMetadata_T )
#define IFACE_MTU_SIZE					4096 // may be any, 4096 value is just for example
#define IFACE_MAX_PAYLOAD_USUAL_SIZE	(IFACE_MTU_SIZE-IFACE_HEADER_SIZE)
#define IFACE_MAX_PAYLOAD_BEACON_SIZE	(IFACE_MAX_PAYLOAD_USUAL_SIZE-IFACE_FOOTER_SIZE)
#define IFACE_BUFFER_SIZE				17 // keep it max of 12 (to keep address) and strlen(REGISTRATION_OK)
#define IFACE_OPENING_SOCKETS			2 // just count of simultaneously kept sockets
#define IFACE_MAX_NEIGHBOR_COUNT		10

typedef float	PowerFloat_T;
typedef uint8_t PowerInt_T;
typedef float	LinkQuality_T;
typedef uint8_t LinkAttempts_T;

typedef enum {
	IfacePackType_NeedResponse,
	IfacePackType_NeedNoResponse,
	IfacePackType_IsResponse,
	IfacePackType_Beacon
} IfacePackType_T;

#pragma pack(push, 1)

typedef struct {
	uint8_t	Value[ IFACE_ADDR_SIZE ];
} IfaceAddr_T;

// type for usual iface header
typedef struct {
	IfaceAddr_T		To,			// receiver of packet
					From;		// sender of packet
	Crc_T			CRC;		// CRC summ of current packet (assuming part of From field instead of CRC bytes while calculating them)
	PowerInt_T		TxPower;	// power level of packet transmitting
	IfacePackType_T	Type;		// type of the packet
	size_t			Size;		// size of payload in current packet
} IfaceHeader_T;

// type for iface beacon footer
typedef struct {	// TODO: add other fields when needed
	PowerInt_T	MinSensitivity; // minimal sensitivity of interface
} IfaceFooter_T;

// struct for data about one neighbor
typedef struct {
	IfaceAddr_T		Address;
	PowerFloat_T	MinPower;
	LinkAttempts_T	AttemptsLeft;
	LinkQuality_T 	LinkQuality;
} IfaceNeighbor_T;

// struct for configuration variables
typedef struct {
	IfaceAddr_T			Address;
	int					MockitSocket,
						ChannelSocket,
						NeighborsCount,
						BeaconPayloadSize,
						BeaconIntervalCurrent,
						EpollHandler;
	PowerFloat_T		CurrentSensitivity,
						CurrentBeaconPower;
	bool				IsWaitingForResponse,
						IsConnectedToChannel,
						IsConnectedToMockit;
	SocketFilepath_T	ChannelSocketFilepath;
} IfaceConfiguration_T;

// struct with preallocated memory for iface routines
typedef struct {
	char					Buffer[ IFACE_BUFFER_SIZE ],
							Payload[ IFACE_MAX_PAYLOAD_USUAL_SIZE ],
							BeaconPayload[ IFACE_MAX_PAYLOAD_BEACON_SIZE ];
	IfaceHeader_T			BufferHeader;
	IfaceFooter_T			BufferFooter;
	IfaceNeighbor_T			Neighbors[ IFACE_MAX_NEIGHBOR_COUNT ];
	LayerCommandStruct_T	Command;
	MessageId_T				ProcessingMessageId;
	struct epoll_event		EpollEvents[ IFACE_OPENING_SOCKETS ];
} IfacePreallocated_T;

// struct to unify configuration and preallocated memory for interface layer
typedef struct {
	IfaceConfiguration_T	Config;
	IfacePreallocated_T		Memory;
} IfaceState_T;

// commands (actually, they should be public, but it is impossible due to architectural reasons (address length))

typedef struct {
	UnIfaceAddrLen_T	Length;
	IfaceAddr_T			Value;
} IfaceRegisterMetadata_T;

// interface unregistration metadata
typedef struct {

} IfaceUnregisterMetadata_T;

// channel send command metadata
typedef struct {
	MessageId_T Id;
	bool 		NeedResponse;
	IfaceAddr_T	To;
} ChannelSendMetadata_T;

// interface receive command metadata
typedef struct {
	MessageId_T Id;
	IfaceAddr_T	From;
} IfaceReceiveMetadata_T;

// interface neighbor info command
typedef struct {
	IfaceAddr_T	Neighbor;
} IfaceNeighborMetadata_T;

#pragma pack(pop)

#endif //MOARSTACK_MOARINTERFACEPRIVATE_H
