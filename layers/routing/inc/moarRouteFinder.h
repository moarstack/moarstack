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
#include <string.h>
#include <moarRoutingStoredPacketFunc.h>

#define StartRouteFinderPacketSize 1
#define MaxRouteFinderPacketSize 10
#endif //MOARSTACK_MOARROUTEFINDER_H

typedef struct
{
    uint8_t            Size;
    RouteAddr_T        NodeList[1];
} RoutePayloadFinder_T;

int produceRouteFinder(RoutingLayer_T *layer, RouteAddr_T destination, RouteAddr_T next_hop);