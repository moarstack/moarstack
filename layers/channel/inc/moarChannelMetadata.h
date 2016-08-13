//
// Created by svalov on 7/21/16.
//

#ifndef MOARSTACK_MOARCHANNELMETADATA_H
#define MOARSTACK_MOARCHANNELMETADATA_H

#include <moarChannelPrivate.h>
#include <moarCommons.h>
#pragma pack(push, 1)

//in memory structures
// metadata of packet moving from channel to interface
typedef struct {
	MessageId_T Id;
	bool 		NeedResponse;
	UnIfaceAddr_T	Bridge;
} ChannelSendMetadata_T;

// metadata of packet moving from interface to channel
typedef struct {
	MessageId_T Id;
	UnIfaceAddr_T	From;
} InterfaceReceiveMetadata_T;

// interface registration metadata
typedef struct {
	UnIfaceAddr_T	IfaceAddress;
	//some options
} InterfaceRegisterMetadata_T;

// interface unregistration metadata
typedef struct {

} InterfaceUnregisterMetadata_T;

typedef struct {
	UnIfaceAddr_T Address;
} InterfaceNeighborMetadata_T;

// plain socket structures

// metadata of packet moving from channel to interface
typedef struct {
	MessageId_T Id;
	bool 		NeedResponse;
	//last n bytes of metadata is interface address
} ChannelSendMetadataPlain_T;

// metadata of packet moving from interface to channel
typedef struct {
	MessageId_T Id;
	//last n bytes of metadata is interface address
} InterfaceReceiveMetadataPlain_T;

// interface registration metadata
typedef struct {
	UnIfaceAddrLen_T AddressLength;
	//some options
	//last n bytes of metadata is interface address
} InterfaceRegisterMetadataPlain_T;

// interface unregistration metadata
typedef struct {

} InterfaceUnregisterMetadataPlain_T;

typedef struct {
	//last n bytes of metadata is interface address
} InterfaceNeighborMetadataPlain_T;

#pragma pack(pop)

#define	CHANNEL_SEND_METADATA_PLAIN_SIZE  			sizeof( ChannelSendMetadataPlain_T )
#define	INTERFACE_RECEIVE_METADATA_PLAIN_SIZE  		sizeof( InterfaceReceiveMetadataPlain_T )
#define	INTERFACE_REGISTER_METADATA_PLAIN_SIZE  	sizeof( InterfaceRegisterMetadataPlain_T )
#define	INTERFACE_UNREGISTER_METADATA_PLAIN_SIZE 	sizeof( InterfaceUnregisterMetadataPlain_T )
#define	INTERFACE_NEIGHBOR_METADATA_PLAIN_SIZE 		sizeof( InterfaceUnregisterMetadataPlain_T )


int writeSendMetadata(int fd, ChannelSendMetadata_T* metadata, size_t dataSize, void* data);
int readReceiveMetadata(int fd, LayerCommandStruct_T* command, UnIfaceAddrLen_T addrSize, InterfaceReceiveMetadata_T* metadata);
int readRegisterMetadata(int fd, LayerCommandStruct_T* command, InterfaceRegisterMetadata_T* metadata);
int readUnregisterMetadata(int fd, LayerCommandStruct_T* command, InterfaceUnregisterMetadata_T* metadata);
int readNeighborMetadata(int fd, LayerCommandStruct_T* command, UnIfaceAddrLen_T addrSize, InterfaceNeighborMetadata_T* metadata);

#endif //MOARSTACK_MOARCHANNELMETADATA_H
