//
// Created by svalov on 8/13/16.
//

#ifndef MOARSTACK_MOARROUTINGPRIVATE_H
#define MOARSTACK_MOARROUTINGPRIVATE_H

#include <moarRouting.h>
#include <moarChannelRouting.h>
#include <moarRoutingPresentation.h>

#define EPOLL_SOCKETS_COUNT 				2
#define EPOLL_CHANNEL_EVENTS 				EPOLLIN
#define EPOLL_PRESENTATION_EVENTS 			EPOLLIN
#define EPOLL_TIMEOUT						1000
#define EPOLL_EVENTS_COUNT					EPOLL_SOCKETS_COUNT


typedef struct{
	int ChannelSocket;
	int PresentationSocket;
	int 					EpollHandler;
	struct epoll_event 		EpollEvent[EPOLL_EVENTS_COUNT];
	int 					EpollCount;
	bool 					Running;
	int 					EpollTimeout;
} RoutingLayer_T;

#endif //MOARSTACK_MOARROUTINGPRIVATE_H
