//
// Created by kryvashek on 05.07.16.
// for public notations

#ifndef MOARSTACK_MOARINTERFACE_H
#define MOARSTACK_MOARINTERFACE_H

#include <moarCommons.h>	// SocketFilepath_T
#include <moarLogger.h>		// LogFilepath_T

#define IFACE_ADDR_BASE_TYPE	unsigned int
#define IFACE_ADDR_SIZE			sizeof( IFACE_ADDR_BASE_TYPE )

#pragma pack(push, 1)

typedef struct {
	uint8_t	Value[ IFACE_ADDR_SIZE ];
} IfaceAddr_T;

#pragma pack(pop)

#endif //MOARSTACK_MOARINTERFACE_H
