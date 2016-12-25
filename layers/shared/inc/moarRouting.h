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
#define ROUTING_HEADER_SIZE 			sizeof(RoutingHeader_T)
#define ROUTE_PACK_TYPE_BITS			8
#define routeAddrEqual(first,second)	routeAddrEqualPtr(&(first),&(second))

#pragma pack(push, 1)

//typedef struct{
//	uint8_t		Address[ ROUTE_ADDR_SIZE ];
//} RouteAddr_T;

typedef ChannelAddr_T	RouteAddr_T;
typedef uint16_t 		RouteXTL_T;

typedef enum{
	RoutePackType_Data,
	RoutePackType_Ack,
	RoutePackType_FinderAck,
	RoutePackType_Finder,
	RoutePackType_Probe,
}RoutePackType_T;

typedef struct{
	RoutePackType_T 	PacketType:ROUTE_PACK_TYPE_BITS;
	PayloadSize_T 		PayloadSize;
	RouteAddr_T 		Source;
	RouteAddr_T 		Destination;
	RoutingMessageId_T 	Id;
	RouteXTL_T			XTL;		// something To Live - hops or time or something similiar (now supposed to be HOPS)
}RoutingHeader_T;

#pragma pack(pop)

__BEGIN_DECLS

extern bool routeAddrEqualPtr(const RouteAddr_T* first, const RouteAddr_T* second);
extern int routeAddrFromStr(char* address, RouteAddr_T* routeAddr);

__END_DECLS

#endif //MOARSTACK_MOARROUTING_H
