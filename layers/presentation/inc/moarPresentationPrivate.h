//
// Created by kryvashek on 18.08.16.
//

#ifndef MOARSTACK_MOARPRESENTATIONPRIVATE_H
#define MOARSTACK_MOARPRESENTATIONPRIVATE_H

#include <sys/epoll.h>
#include <stdbool.h>
#include <moarCommons.h>

#define EPOLL_SOCKETS_COUNT 				2
#define EPOLL_SERVICE_EVENTS 				EPOLLIN
#define EPOLL_ROUTING_EVENTS 				EPOLLIN
#define EPOLL_TIMEOUT						1000
#define EPOLL_EVENTS_COUNT					EPOLL_SOCKETS_COUNT
#define SERIVCE_PROCESSING_RULES_COUNT		2
#define ROUTING_PROCESSING_RULES_COUNT		3


typedef struct{
	int 						RoutingSocket;
	int 						ServiceSocket;
	int 						EpollHandler;
	struct epoll_event 			EpollEvent[EPOLL_EVENTS_COUNT];
	int 						EpollCount;
	bool 						Running;
	int 						EpollTimeout;
	CommandProcessingRule_T 	ServiceProcessingRules[SERIVCE_PROCESSING_RULES_COUNT];
	CommandProcessingRule_T		RoutingProcessingRules[ROUTING_PROCESSING_RULES_COUNT];
}PresentationLayer_T;

#endif //MOARSTACK_MOARPRESENTATIONPRIVATE_H
