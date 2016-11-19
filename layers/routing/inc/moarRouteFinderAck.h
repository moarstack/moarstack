//
// Created by kryvashek on 12.11.16.
//

#ifndef MOARSTACK_MOARROUTEFINDERACK_H
#define MOARSTACK_MOARROUTEFINDERACK_H

#include <moarRoutingPrivate.h>
#include <moarRoutingStoredPacket.h>

extern int processPayloadFinderAck( RoutingLayer_T * layer, RouteStoredPacket_T * packet );
extern int produceFack( RoutingLayer_T * layer, RouteStoredPacket_T * oldPacket, RouteStoredPacket_T * newPacket );
extern int sendFack( RoutingLayer_T * layer, RouteStoredPacket_T * oldPacket );

#endif //MOARSTACK_MOARROUTEFINDERACK_H
