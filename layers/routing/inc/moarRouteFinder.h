//
// Created by spiralis on 05.11.16.
//

#ifndef MOARSTACK_MOARROUTEFINDER_H
#define MOARSTACK_MOARROUTEFINDER_H

#include <stdlib.h>
#include <stdint.h>
#include <moarCommons.h>
#include <moarTime.h>
#include <funcResults.h>
#include <moarRouting.h>
#include <moarRoutingStoredPacket.h>
#include <moarRoutingPrivate.h>
#include "moarRouteFinder.h"
#include <string.h>

#define StartRouteFinderPacketSize 1
#define MaxRouteFinderPacketSize 10
#endif //MOARSTACK_MOARROUTEFINDER_H

typedef struct
{
    int                Size;
    RouteAddr_T        NodeList[];
} RoutePayloadFinder_T;

int produceRouteFinder(RoutingLayer_T *layer, RouteAddr_T destination);