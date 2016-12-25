//
// Created by kryvashek on 07.07.16.
//

#ifndef MOARSTACK_MOARPRESENTATIONSERVICE_H
#define MOARSTACK_MOARPRESENTATIONSERVICE_H

#include <stddef.h>
#include "moarCommons.h"
#include "moarRouting.h"	// RouteAddr_T
#include "moarPresentation.h"
#include "moarService.h"

#define PACK_STATE_PRESENT_BITS	8

#pragma pack(push, 1)

// possible states of packet when it is moving from presentation to service
typedef enum {
	PackStatePresent_None, 			// not defined state of enum
	PackStatePresent_Sent,			// current packet is sent and has no need to get response
	PackStatePresent_NotSent,		// current packet is sent and now is waiting for response
	PackStatePresent_Received		// current packet was received
} PackStatePresent_T;

// metadata of packet moving from presentation to service
typedef struct {
	MessageId_T 		Mid; // mid of current packet
	RouteAddr_T			Source; // source node of current packet
} PresentReceivedMsg_T;

typedef struct {
	MessageId_T 		Mid; // mid of current packet
	PackStatePresent_T State:PACK_STATE_PRESENT_BITS;
} PresentMsgStateMetadata_T;

// metadata of packet moving from service to presentation
typedef struct {
	MessageId_T 		Mid;  // mid of current packet
	RouteAddr_T			Destination; // destination of packet
} ServiceSendMsgDown_T;

#pragma pack(pop)

#endif //MOARSTACK_MOARPRESENTATIONSERVICE_H
