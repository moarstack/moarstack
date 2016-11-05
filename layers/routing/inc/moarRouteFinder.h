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
#include "moarRouteFinder.h"

#endif //MOARSTACK_MOARROUTEFINDER_H

typedef struct
{
    RouteAddr_T        NodeList[];
    int                Size;
} RoutePayloadFinder_T;

int produceRouteFinder(RoutingLayer_T *layer);