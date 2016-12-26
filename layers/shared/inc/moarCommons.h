//
// Created by kryvashek on 04.07.16.
//

#ifndef MOARSTACK_MOARCOMMONS_H
#define MOARSTACK_MOARCOMMONS_H

#include <stdint.h>
#include <stddef.h>
#include <moarMessageId.h>

#define SOCKET_FILEPATH_SIZE	108 // limited with length of [struct sockadddr_un].sun_path
#define CRC_SIZE				sizeof( Crc_T )
#define LAYER_COMMAND_TYPE_BITS	8

#pragma pack(push, 1)

typedef char		SocketFilepath_T[ SOCKET_FILEPATH_SIZE ];
typedef uint32_t	Crc_T; // type for values of CRC calculating result

// stack-wide command type
typedef enum {
	LayerCommandType_None,
    LayerCommandType_Send,
	LayerCommandType_SendResult,
	LayerCommandType_Receive,
    LayerCommandType_NewNeighbor,
    LayerCommandType_LostNeighbor,
	LayerCommandType_UpdateNeighbor,
	LayerCommandType_MessageState,
	LayerCommandType_MessageStateResult,
	LayerCommandType_RegisterInterface,
	LayerCommandType_RegisterInterfaceResult,
	LayerCommandType_UnregisterInterface,
	LayerCommandType_ConnectApplication,
	LayerCommandType_ConnectApplicationResult,
	LayerCommandType_Bind,
	LayerCommandType_BindResult,
	LayerCommandType_DisconnectApplication,
	LayerCommandType_InterfaceState,
	LayerCommandType_UpdateBeaconPayload,
	// some other commands
	LayerCommandType_TypesCount // should always be last
} LayerCommandType_T;

typedef size_t 	PayloadSize_T;

// struct to describe command and related arguments (so-called 'metadata') in socket
typedef struct {
    LayerCommandType_T  Command:LAYER_COMMAND_TYPE_BITS;    // type of command according to enum LayerCommandType_T
	PayloadSize_T       MetaSize;   // size of command arguments depending on the command type
	PayloadSize_T 		DataSize;
} LayerCommandPlain_T;

// struct to describe command and related arguments (so-called 'metadata') in memory
typedef struct {
	LayerCommandType_T  Command:LAYER_COMMAND_TYPE_BITS;    // type of command according to enum LayerCommandType_T
	PayloadSize_T       MetaSize;   // size of command arguments depending on the command type
	PayloadSize_T 		DataSize;
	void				* MetaData;	// metadata for current command
	void				* Data;		// data for current command

} LayerCommandStruct_T;

typedef uint8_t NeighborsCount_T;   // type to describe nearmates count

typedef int (*CommandProcessor_T)(void *, int, LayerCommandStruct_T*);

typedef struct{
	LayerCommandType_T CommandType:LAYER_COMMAND_TYPE_BITS;
	CommandProcessor_T ProcessingRule;
} CommandProcessingRule_T;

#pragma pack(pop)

__BEGIN_DECLS

// function to use if bytes order needs to be changed
// do nothing if size = 0 or input = NULL
// save bytes in the same memory if output = NULL or output = input
extern void ChangeBytesOrder(void *output, const void *input, const size_t size);

// read command from socket
extern int ReadCommand(int fd, LayerCommandStruct_T* command);

// write command to socket
extern int WriteCommand(int fd, LayerCommandStruct_T* command);

// open fiel socket with specified path
extern int SocketOpenFile( const SocketFilepath_T socketFilePath, const bool isServer, int * sockResult );

extern int FreeCommand( LayerCommandStruct_T * command );

extern int ProcessCommand(void* layer, int fd, uint32_t event, uint32_t eventMask, CommandProcessingRule_T* rules);
extern CommandProcessingRule_T MakeProcessingRule(LayerCommandType_T type, CommandProcessor_T rule);
extern char * mStrDup( const char * source );

__END_DECLS

#endif //MOARSTACK_MOARCOMMONS_H