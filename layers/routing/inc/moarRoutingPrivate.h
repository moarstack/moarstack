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
#include <moarRouteTable.h>

#define EPOLL_SOCKETS_COUNT 				2
#define EPOLL_CHANNEL_EVENTS 				EPOLLIN
#define EPOLL_PRESENTATION_EVENTS 			EPOLLIN
#define EPOLL_TIMEOUT						1000
#define EPOLL_EVENTS_COUNT					EPOLL_SOCKETS_COUNT
#define CHANNEL_PROCESSING_RULES_COUNT		6
#define PRESENTATION_PROCESSING_RULES_COUNT	2
#define ACK_WAITING_TIMEOUT					10000
#define FACK_WAITING_TIMEOUT				15000
#define SENT_WAITING_TIMEOUT				10000
#define UNSENT_WAITING_TIMEOUT				5000
#define DEFAULT_ROUTE_TRYS					3
#define	DEFAULT_XTL							((RouteXTL_T)1024)
#define DEFAULT_XTL_STEP					((RouteXTL_T)1)
#define DEC_XTL_ON_TRYS						false
#define DEFAULT_XTL_DATA					DEFAULT_XTL
#define DEFAULT_XTL_ACK						DEFAULT_XTL
#define DEFAULT_XTL_FINDER					DEFAULT_XTL
#define DEFAULT_XTL_FACK					DEFAULT_XTL
#define DEFAULT_XTL_PROBE					DEFAULT_XTL
#define DEFAULT_PROBE_DEPTH					64
#define DEFAULT_PROBE_SEND_PERIOD			120000 // in milliseconds
#define DEFAULT_TABLE_RENEW_PERIOD			15000 // calibrate!

#pragma pack(push, 1)

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
	RouteDataTable_T			RouteTable,
								RouteTableSolved;
	moarTime_T					NextProbeSentTime,
								NextTableRenewTime;
} RoutingLayer_T;

#pragma pack(pop)

int helperFindRelay( RoutingLayer_T * layer, RouteAddr_T * dest, ChannelAddr_T * relay );
int helperUpdateRouteAddrChainBefore( RoutingLayer_T * layer, RouteAddr_T * list, size_t count );
int helperUpdateRouteAddrChainAfter( RoutingLayer_T * layer, RouteAddr_T * list, size_t count );
int helperUpdateRouteAddrChain( RoutingLayer_T * layer, RouteAddr_T * list, size_t count, bool before );
int helperSolveRoutes( RoutingLayer_T * layer );

#endif //MOARSTACK_MOARROUTINGPRIVATE_H
