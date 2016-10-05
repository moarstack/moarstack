//
// Created by svalov on 8/13/16.
//

#ifndef MOARSTACK_MOARROUTINGPRIVATE_H
#define MOARSTACK_MOARROUTINGPRIVATE_H

#include <moarRouting.h>
#include <moarChannelRouting.h>
#include <moarRoutingPresentation.h>
#include <sys/epoll.h>
#include <moarRoutingPacketStorage.h>
#include <moarRoutingNeighborsStorage.h>

#define EPOLL_SOCKETS_COUNT 				2
#define EPOLL_CHANNEL_EVENTS 				EPOLLIN
#define EPOLL_PRESENTATION_EVENTS 			EPOLLIN
#define EPOLL_TIMEOUT						1000
#define EPOLL_EVENTS_COUNT					EPOLL_SOCKETS_COUNT
#define CHANNEL_PROCESSING_RULES_COUNT		6
#define PRESENTATION_PROCESSING_RULES_COUNT	2
#define ACK_WAITING_TIMEOUT					10000

typedef struct{
	int 						ChannelSocket;
	int 						PresentationSocket;
	int 						EpollHandler;
	struct epoll_event 			EpollEvent[EPOLL_EVENTS_COUNT];
	int 						EpollCount;
	bool 						Running;
	int 						EpollTimeout;
	CommandProcessingRule_T 	ChannelProcessingRules[CHANNEL_PROCESSING_RULES_COUNT];
	CommandProcessingRule_T		PresentationProcessingRules[PRESENTATION_PROCESSING_RULES_COUNT];
	RouteAddr_T					LocalAddress;
	PacketStorage_T				PacketStorage;
	RoutingNeighborsStorage_T 	NeighborsStorage;
} RoutingLayer_T;



#endif //MOARSTACK_MOARROUTINGPRIVATE_H
