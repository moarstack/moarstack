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


typedef struct
{
    uint8_t            MaxSize;
} RouteInitialPayloadFinder_T;

int sendFindersFirst( RoutingLayer_T * layer, RouteAddr_T * dest );

int produceInitialRouteFinder(RoutingLayer_T *layer, RouteAddr_T *destination, RouteAddr_T *next_hop,
                              RouteStoredPacket_T *ptr);

int produceNextRouteFinder( RoutingLayer_T * layer, RouteStoredPacket_T * oldPacket, RouteStoredPacket_T * newPacket );

int getNextFinderPayload( RoutingLayer_T * layer, void * oldPayload, PayloadSize_T oldSize, void ** newPayload, PayloadSize_T * newSize );

#endif //MOARSTACK_MOARROUTEFINDER_H