//
// Created by kryvashek on 05.07.16.
// for public notations

#ifndef MOARSTACK_MOARROUTING_H
#define MOARSTACK_MOARROUTING_H

#include <stdint.h>
#include <moarRoutingMessageId.h>
#include <moarChannel.h>
#include <moarCommons.h>
//#define ROUTE_ADDR_SIZE	8

#pragma pack(push, 1)

//typedef struct{
//	uint8_t		Address[ ROUTE_ADDR_SIZE ];
//} RouteAddr_T;

typedef ChannelAddr_T RouteAddr_T;

typedef enum{
	RoutePackType_Data,
	RoutePackType_Ack,
	RoutePackType_FinderAck,
	RoutePackType_Finder,
	RoutePackType_Probe,
}RoutePackType_T;

typedef struct{
	RoutePackType_T 	PacketType;
	PayloadSize_T 		PayloadSize;
	RouteAddr_T 		Source;
	RouteAddr_T 		Destination;
	RoutingMessageId_T 	Id;
}RoutingHeader_T;

#pragma pack(pop)

#define ROUTING_HEADER_SIZE 	sizeof(RoutingHeader_T)

__BEGIN_DECLS
extern bool routeAddrEqualPtr(RouteAddr_T* first, RouteAddr_T* second);

__END_DECLS
#define routeAddrEqual(first, second) routeAddrEqualPtr(&(first),&(second))

#endif //MOARSTACK_MOARROUTING_H
