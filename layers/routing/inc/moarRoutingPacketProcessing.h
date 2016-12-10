//
// Created by svalov on 10/6/16.
//

#ifndef MOARSTACK_MOARROUTINGPACKETPROCESSING_H
#define MOARSTACK_MOARROUTINGPACKETPROCESSING_H

#include <moarRoutingPrivate.h>

int processPacketStorage(RoutingLayer_T* layer);
int ProduceAndSendProbePacketFurther(RoutingLayer_T *layer, RouteStoredPacket_T *packet, RouteAddr_T *next);

#endif //MOARSTACK_MOARROUTINGPACKETPROCESSING_H
