//
// Created by kryvashek on 07.07.16.
//

#ifndef MOARSTACK_MOARPRESENTATIONSERVICE_H
#define MOARSTACK_MOARPRESENTATIONSERVICE_H

#include <stddef.h>
#include "moarCommons.h"
#include "moarPresentation.h"
#include "moarService.h"

const size_t	PRESENT_MSG_UP_SIZE = sizeof( PresentMsgUp_T );
const size_t	SERVICE_MSG_DOWN_SIZE = sizeof( ServiceMsgDown_T );

// possible states of packet when it is moving from presentation to service
typedef enum {
	PackStatePresent_None, 			// not defined state of enum
	PackStatePresent_Sent,			// current packet is sent and has no need to get response
	PackStatePresent_NotSent,		// current packet is sent and now is waiting for response
	PackStatePresent_Received		// current packet was received
} PackStatePresent_T;

// metadata of packet moving from presentation to service
typedef struct {
	PackStatePresent_T	State;	// state of packet moving from channel to routing
	size_t				Size;	// size of payload
	RouteAddr_T			Source; // source node of current packet
} PresentMsgUp_T;

// metadata of packet moving from service to presentation
typedef struct {
	RouteAddr_T	Destination;
	size_t		Size;
} ServiceMsgDown_T;

#endif //MOARSTACK_MOARPRESENTATIONSERVICE_H
