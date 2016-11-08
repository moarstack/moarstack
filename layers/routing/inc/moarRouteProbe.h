//
// Created by kryvashek on 07.11.16.
//

#ifndef MOARSTACK_MOARROUTEPROBE_H
#define MOARSTACK_MOARROUTEPROBE_H

typedef uint16_t	RouteProbeDepth_T;

typedef struct {
	RouteProbeDepth_T	DepthMax,
						DepthCurrent;
} RoutePayloadProbe_T;

extern int produceProbeFirst( RoutingLayer_T * layer, RouteAddr_T * next, RouteStoredPacket_T * packet );
extern int produceProbeNext( RoutingLayer_T * layer, RouteStoredPacket_T * oldPacket, RouteAddr_T * next, RouteStoredPacket_T * newPacket );
extern int sendProbeFirst( RoutingLayer_T * layer );

#endif //MOARSTACK_MOARROUTEPROBE_H
