//
// Created by kryvashek on 04.07.16.
//

#ifndef MOARSTACK_MOARCOMMONS_H
#define MOARSTACK_MOARCOMMONS_H

#include <stdint.h>
#include <stddef.h>

#define ROUTE_ADDR_SIZE	8

// stack-wide command type
typedef enum {
    LayerCommandType_ProcessData,
    LayerCommandType_RequestNearmates,
    LayerCommandType_ReportNearmates
    // some other commands
} LayerCommandType_T;

// struct to describe command and related arguments (so-called 'metadata')
typedef struct {
    LayerCommandType_T  Command;    // type of command according to enum LayerCommandType_T
    size_t              MetaSize;   // size of command arguments depending on the command type
} LayerCommand_T;

typedef uint8_t NearmatesCount_T;   // type to describe nearmates count

// struct ofr metadata of 'LayerCommandType_ReportNearmates' command
typedef struct {
    NearmatesCount_T    Count;      // count of nearmates in report
    size_t              EntrySize;  // size of data per one nearmate
} NearmatesReportMeta_T;

typedef uint8_t	RouteAddr_T[ ROUTE_ADDR_SIZE ];

// function to use if bytes order needs to be changed
// do nothing if size = 0 or input = NULL
// save bytes in the same memory if output = NULL or output = input
extern void ChangeBytesOrder(void *output, const void *input, const size_t size);

#endif //MOARSTACK_MOARCOMMONS_H
