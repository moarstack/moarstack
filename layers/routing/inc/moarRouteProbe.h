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

#endif //MOARSTACK_MOARROUTEPROBE_H
