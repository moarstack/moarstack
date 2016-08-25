//
// Created by kryvashek on 05.07.16.
// for public notations

#ifndef MOARSTACK_MOARINTERFACE_H
#define MOARSTACK_MOARINTERFACE_H

#include <moarCommons.h>	// SocketFilepath_T
#include <moarLogger.h>		// LogFilepath_T

typedef struct{
	SocketFilepath_T	socketToChannel;
	LogFilepath_T 		filepathToLog;
} MoarIfaceStartupParams_T;

#endif //MOARSTACK_MOARINTERFACE_H
