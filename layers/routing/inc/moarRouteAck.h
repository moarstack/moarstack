//
// Created by spiralis on 29.10.16.
//

#ifndef MOARSTACK_MOARROUTEACK_H
#define MOARSTACK_MOARROUTEACK_H

#include <stdlib.h>
#include <stdint.h>
#include <moarCommons.h>
#include <moarTime.h>
#include <funcResults.h>
#include <moarRouting.h>
#include <moarRoutingStoredPacket.h>
#include <moarRoutingPrivate.h>

typedef struct
{
    RouteAddr_T             originalSource;
    RouteAddr_T             originalDestination;
    RoutingMessageId_T      messageId;
} RoutePayloadAck_T;

int produceAck(RoutingLayer_T *layer, RouteStoredPacket_T* original);



#endif //MOARSTACK_MOARROUTEACK_H
