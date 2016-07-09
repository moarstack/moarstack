//
// Created by kryvashek on 05.07.16.
//

#ifndef MOARSTACK_MOARROUTINGPRESENTATION_H
#define MOARSTACK_MOARROUTINGPRESENTATION_H

#include <stddef.h>
#include "moarCommons.h"
#include "moarRouting.h"
#include "moarPresentation.h"

// possible states of packet when it is moving from routing to presentation
typedef enum {
	PackStateRoute_None, 		// not defined state of enum
	PackStateRoute_Sent,		// current packet is sent and has no need to get response
	PackStateRoute_NotSent,		// current packet is sent and now is waiting for response
	PackStateRoute_Received		// current packet was received
} PackStateRoute_T;

// metadata of packet moving from routing to presentation
typedef struct {
	PackStateRoute_T	State;	// state of packet moving from channel to routing
} RouteMsgUp_T;

// metadata of packet moving from presentation to routing
typedef struct {
	RouteAddr_T	Destination;
} PresentMsgDown_T;

const size_t	ROUTE_MSG_UP_SIZE = sizeof( RouteMsgUp_T );
const size_t	PRESENT_MSG_DOWN_SIZE = sizeof( PresentMsgDown_T );

#endif //MOARSTACK_MOARROUTINGPRESENTATION_H
