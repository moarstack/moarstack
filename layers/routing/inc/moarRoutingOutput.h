//
// Created by svalov on 8/17/16.
//

#ifndef MOARSTACK_MOARROUTINGOUTPUT_H
#define MOARSTACK_MOARROUTINGOUTPUT_H

#include <moarRoutingPrivate.h>
#include <moarRoutingStoredPacket.h>

int sendPacketToChannel(RoutingLayer_T* layer, RouteStoredPacket_T* packet);
//int sendPacketToPresentation(RoutingLayer_T* layer, RouteStoredPacket_T* packet);

#endif //MOARSTACK_MOARROUTINGOUTPUT_H
