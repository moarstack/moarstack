//
// Created by kryvashek on 09.07.16.
// for private notations

#ifndef MOARSTACK_MOARCHANNELPRIVATE_H
#define MOARSTACK_MOARCHANNELPRIVATE_H

#include <time.h>
#include <stdint.h>
#include <moarChannel.h>
#include <moarUnIfaceAddr.h>
#include <sys/epoll.h>
#include <linkedList.h>
#include <moarChannelRouting.h>

#define LISTEN_COUNT						10
#define EPOLL_INTERFACE_SOCKET_EVENTS 		EPOLLIN
#define EPOLL_INTERFACE_EVENTS 				EPOLLIN
#define EPOLL_INTERFACE_DISCONNECT_EVENTS 	(EPOLLERR | EPOLLHUP)
#define EPOLL_ROUNTING_EVENTS 				EPOLLIN
#define EPOLL_TIMEOUT						1000
#define MAX_INTERFACE_COUNT					255
#define EPOLL_EVENTS_COUNT					(MAX_INTERFACE_COUNT+2)
#define SEND_TRYS							5

typedef struct {
	UnIfaceAddr_T 	Address;
	time_t 			LastSeen;
} InterfaceNeighbor_T;

typedef struct{
	//processing time
	int8_t TrysLeft;
	RouteSendMetadata_T Metadata;
	PayloadSize_T  DataSize;
	void* Data;
} ChannelMessageEntry_T;

typedef struct {
	UnIfaceAddr_T Address;
	bool Ready;
	//PackStateIface_T CurrentState;
	int Socket;
	InterfaceNeighbor_T* Neighbors;
	// pointer to channel neighbor
	ChannelMessageEntry_T CurrentMessage;
} InterfaceDescriptor_T;

typedef struct{
	UnIfaceAddr_T Address; // address of remote interface
	InterfaceDescriptor_T* BridgeInterface; // local interface, that can communicate with remote interface
} RemoteInterface_T;

typedef struct{
	ChannelAddr_T 	RemoteAddress;
	time_t 			LastSeen;
	LinkedListItem_T Interfaces;
} ChannelNeighbor_T;


typedef struct {
	ChannelAddr_T 			LocalAddress;
	int 					UpSocket;
	int 					DownSocket;
	uint8_t 				InterfacesCount;
	LinkedListItem_T 		Interfaces;
	LinkedListItem_T 		Neighbors;
	LinkedListItem_T		MessageQueue;
	int 					EpollHandler;
	struct epoll_event 		EpollEvent[EPOLL_EVENTS_COUNT];
	int 					EpollCount;
	bool 					Running;
	int 					EpollTimeout;
	//mesage state table
	//some message queue
} ChannelLayer_T;

int processCloseConnection(ChannelLayer_T* layer, int fd);

#endif //MOARSTACK_MOARCHANNELPRIVATE_H
