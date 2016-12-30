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
#include <moarTime.h>
#include <queue.h>
#include <priorityQueue.h>

#define LISTEN_COUNT						10
#define EPOLL_INTERFACE_SOCKET_EVENTS 		EPOLLIN
#define EPOLL_INTERFACE_EVENTS 				EPOLLIN
#define EPOLL_INTERFACE_DISCONNECT_EVENTS 	(EPOLLERR | EPOLLHUP)
#define EPOLL_ROUNTING_EVENTS 				EPOLLIN
#define EPOLL_TIMEOUT						1000
#define MAX_INTERFACE_COUNT					255
#define EPOLL_EVENTS_COUNT					(MAX_INTERFACE_COUNT+2)
#define SEND_TRYS							5
#define PROCESSING_TIMEOUT					((moarTimeInterval_T)1000)
#define PROCESSING_UNRESOLVED_TIMEOUT		((moarTimeInterval_T)10000)
#define DATA_NEED_RESPONSE					true
#define HELLO_NEED_RESPONSE					false
#define INTERFACE_PROCESSING_RULES_COUNT	8
#define ROUTING_PROCESSING_RULES_COUNT		2
#define MESSAGE_QUEUE_SIZE					32
#define INTERFACES_TABLE_SIZE				MAX_INTERFACE_COUNT
#define NEIGHBORS_TABLE_SIZE				151 // todo numbers? add it to config later
#define NEIGHBORS_BACK_TABLE_SIZE			151
#define NEIGHBORS_NONRES_TABLE_SIZE			23
#define NEIGHBORS_INTERFACES_TABLE_SIZE		17

#pragma pack(push, 1)

typedef struct {
	UnIfaceAddr_T 	Address;
	time_t 			LastSeen;
} InterfaceNeighbor_T;

typedef struct{
	//processing time
	SendTrys_T 			SendTrys;
	RouteSendMetadata_T Metadata;
	PayloadSize_T  		DataSize;
	void* 				Data;
	moarTime_T 			ProcessingTime;
} ChannelMessageEntry_T;

typedef struct {
	UnIfaceAddr_T Address;
	bool Ready;
	//PackStateIface_T CurrentState;
	int Socket;
	//InterfaceNeighbor_T* Neighbors;
	// pointer to channel neighbor
	ChannelMessageEntry_T CurrentMessage;
} InterfaceDescriptor_T;

typedef struct{
	UnIfaceAddr_T Address; // address of remote interface
	InterfaceDescriptor_T* BridgeInterface; // local interface, that can communicate with remote interface
} RemoteInterface_T;

typedef struct{
	UnIfaceAddr_T 		Address;
	moarTime_T 			NextProcessingTime;
	int 				SendAttempts;
	int 				LocalInterfaceSocket;
} NonResolvedNeighbor_T;

typedef struct{
	ChannelAddr_T 	RemoteAddress;
	time_t 			LastSeen;
	hashTable_T 	Interfaces;
} ChannelNeighbor_T;


typedef struct {
	ChannelAddr_T 			LocalAddress;
	int 					UpSocket;
	int 					DownSocket;
	hashTable_T		 		Interfaces;
	hashTable_T		 		Neighbors;
	hashTable_T		 		NeighborsBackTranslation;
	hashTable_T 			NeighborsNonResolved;
	PriorityQueue_T			MessageQueue;
	int 					EpollHandler;
	struct epoll_event 		EpollEvent[EPOLL_EVENTS_COUNT];
	int 					EpollCount;
	bool 					Running;
	int 					EpollTimeout;
	void *					HelloMessage;
	PayloadSize_T			HelloMessageSize;
	CommandProcessingRule_T InterfaceProcessingRules[INTERFACE_PROCESSING_RULES_COUNT];
	CommandProcessingRule_T RoutingProcessingRules[ROUTING_PROCESSING_RULES_COUNT];
} ChannelLayer_T;

#pragma pack(pop)

int processCloseConnection(ChannelLayer_T* layer, int fd);

#endif //MOARSTACK_MOARCHANNELPRIVATE_H
