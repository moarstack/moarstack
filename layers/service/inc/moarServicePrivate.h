//
// Created by svalov on 12/12/16.
//

#ifndef MOARSTACK_MOARSERVICEPRIVATE_H
#define MOARSTACK_MOARSERVICEPRIVATE_H

#include <stdint.h>
#include <moarService.h>
#include <moarPresentationService.h>
#include <sys/epoll.h>
#include <moarTime.h>
#include <moarCommons.h>
#include <moarServiceConStore.h>
#include <moarService.h>

#define EPOLL_APP_SOCKET_EVENTS 			EPOLLIN
#define EPOLL_APP_EVENTS 					EPOLLIN
#define EPOLL_APP_DISCONNECT_EVENTS		 	(EPOLLERR | EPOLLHUP)
#define EPOLL_PRESENTATION_EVENTS 			EPOLLIN
#define EPOLL_TIMEOUT						1000
#define MAX_APP_COUNT						65535
#define EPOLL_EVENTS_COUNT					(10)
#define PRESENTATION_PROCESSING_RULES_COUNT	3
#define APP_PROCESSING_RULES_COUNT			7

#define MID_CACH_TABLE_SIZE					100


#pragma pack(push, 1)

typedef struct{
	int 					UpSocket;
	int 					DownSocket;
	//epoll
	int 					EpollHandler;
	struct epoll_event 		EpollEvent[EPOLL_EVENTS_COUNT];
	int 					EpollCount;
	bool 					Running;
	int 					EpollTimeout;
	//
	CommandProcessingRule_T PresentationProcessingRules[PRESENTATION_PROCESSING_RULES_COUNT];
	CommandProcessingRule_T AppProcessingRules[APP_PROCESSING_RULES_COUNT];
	//
	AppConnectionStorage_T  ConnectionStorage;
	hashTable_T				MidStorage;
}ServiceLayer_T;

#pragma pack(pop)

int processCloseConnection(ServiceLayer_T* layer, int fd);

#endif //MOARSTACK_MOARSERVICEPRIVATE_H
