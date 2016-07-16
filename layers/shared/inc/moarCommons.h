//
// Created by kryvashek on 04.07.16.
//

#ifndef MOARSTACK_MOARCOMMONS_H
#define MOARSTACK_MOARCOMMONS_H

#include <stdint.h>
#include <stddef.h>

// stack-wide command type
typedef enum {
	LayerCommandType_None,
    LayerCommandType_Send,
	LayerCommandType_Receive,
    LayerCommandType_NewNeighbor,
    LayerCommandType_LostNeighbor,
	LayerCommandType_UpdateNeighbor,
	LayerCommandType_MessageState,
	LayerCommandType_Connect,
	LayerCommandType_ConnectResult,
	LayerCommandType_InterfaceState,
	LayerCommandType_UpdateBeaconPayload,
	// some other commands
} LayerCommandType_T;

// struct to describe command and related arguments (so-called 'metadata') in socket
typedef struct {
    LayerCommandType_T  Command;    // type of command according to enum LayerCommandType_T
    size_t              MetaSize;   // size of command arguments depending on the command type
} LayerCommandPlain_T;

// struct to describe command and related arguments (so-called 'metadata') in memory
typedef struct {
	LayerCommandType_T  Command;    // type of command according to enum LayerCommandType_T
	size_t              MetaSize;   // size of command arguments depending on the command type
	void				* MetaData;	// metadata for current command
} LayerCommandStruct_T;

// struct to describe packet and its metadata (layer message) in memory
typedef struct {
	size_t	DataSize,	// size of payload, including all added headers
			MsgSize;	// size of layer message (differs for different layers, see <Layer>Msg<Up/Down>_T structs)
	void	* Data,		// place of payload in memory (in memory space of current layer!)
			* Msg;		// place of layer message in memory (in memory space of current layer!)
} LayerPacketStruct_T;

typedef uint8_t NeighborsCount_T;   // type to describe nearmates count

// struct ofr metadata of 'LayerCommandType_ReportNearmates' command
typedef struct {
	NeighborsCount_T    Count;      // count of nearmates in report
    size_t              EntrySize;  // size of data per one nearmate
} NeighborsReportMeta_T;

#ifdef __cplusplus
extern "C" {
#endif

// function to use if bytes order needs to be changed
// do nothing if size = 0 or input = NULL
// save bytes in the same memory if output = NULL or output = input
extern void ChangeBytesOrder(void *output, const void *input, const size_t size);

// read command from socket
extern int ReadCommand(int fd, LayerCommandStruct_T* command);

// write command to socket
extern int WriteCommand(int fd, LayerCommandStruct_T* command);

#ifdef __cplusplus
}
#endif

#endif //MOARSTACK_MOARCOMMONS_H
