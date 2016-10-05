//
// Created by svalov on 10/5/16.
//

#ifndef MOARSTACK_MOARROUTINGTABLESHANDLER_H
#define MOARSTACK_MOARROUTINGTABLESHANDLER_H

#include <moarRoutingNeighborsStorage.h>
#include <moarRouting.h>
#include <moarRoutingPrivate.h>

int helperAddRoute(RouteAddr_T* dest, RouteAddr_T* relay);
int helperAddNeighbor(ChannelAddr_T* address);
int helperRemoveRoute(RouteAddr_T* dest, RouteAddr_T* relay);
int helperRemoveNeighbor(ChannelAddr_T* address);
int helperUpdateRoute();
int helperUpdateNeighbor();

#endif //MOARSTACK_MOARROUTINGTABLESHANDLER_H
