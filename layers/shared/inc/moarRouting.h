//
// Created by kryvashek on 05.07.16.
// for public notations

#ifndef MOARSTACK_MOARROUTING_H
#define MOARSTACK_MOARROUTING_H

#include <stdint.h>
#include <moarRoutingMessageId.h>
#include <moarChannel.h>
//#define ROUTE_ADDR_SIZE	8

#pragma pack(push, 1)

//typedef struct{
//	uint8_t		Address[ ROUTE_ADDR_SIZE ];
//} RouteAddr_T;

typedef ChannelAddr_T RouteAddr_T;

typedef enum{
	RoutePackType_Data,
	RoutePackType_Service,
}RoutePackType_T;

typedef struct{
	RoutePackType_T 	PacketType;
	RouteAddr_T 		Source;
	RouteAddr_T 		Destination;
	RoutingMessageId_T 	Id;
}RoutingHeader_T;

#pragma pack(pop)

#endif //MOARSTACK_MOARROUTING_H
