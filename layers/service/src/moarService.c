//
// Created by svalov on 7/5/16.
//

#include "moarLayerEntryPoint.h"
#include "moarCommons.h"
#include "moarService.h"
#include "moarPresentationService.h"
#include <moarServicePrivate.h>
#include <funcResults.h>
#include <moarLayerEntryPoint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int addSocketToEpoll(ServiceLayer_T* layer, int fd, uint32_t events){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	struct epoll_event sockEvent;
	sockEvent.events = events;
	sockEvent.data.fd = fd;
	int resIface = epoll_ctl(layer->EpollHandler, EPOLL_CTL_ADD, fd, &sockEvent);
	if(0 != resIface)
		return FUNC_RESULT_FAILED;
	return FUNC_RESULT_SUCCESS;
}
int initEpoll(ServiceLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = FUNC_RESULT_SUCCESS;
	//init epol
	layer->EpollTimeout = EPOLL_TIMEOUT;
	layer->EpollCount = EPOLL_EVENTS_COUNT;
	memset(layer->EpollEvent,0,layer->EpollCount*sizeof(struct epoll_event));

	layer->EpollHandler = epoll_create(1);
	if(-1 == layer->EpollHandler) {
		return FUNC_RESULT_FAILED;
	}
	// add socket to presentation
	res = addSocketToEpoll(layer, layer->DownSocket, EPOLL_PRESENTATION_EVENTS);
	CHECK_RESULT(res);
	// add socket to service
	res = addSocketToEpoll(layer, layer->UpSocket, EPOLL_APP_EVENTS);
	CHECK_RESULT(res);

	return FUNC_RESULT_SUCCESS;
}
//open connection to interface
int processNewConnection(ServiceLayer_T* layer, int fd){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;

	// accept
	int newFd = accept(fd, NULL, NULL);
	if(-1 == newFd) {
		return FUNC_RESULT_FAILED_IO;
	}
	int res = addSocketToEpoll(layer, newFd, EPOLL_APP_EVENTS);
	return FUNC_RESULT_SUCCESS;
}
//close connection to interface
int processCloseConnection(ServiceLayer_T* layer, int fd){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	struct epoll_event interfaceEvent;
	interfaceEvent.events = EPOLL_APP_EVENTS;
	interfaceEvent.data.fd = fd;
	int resRouting = epoll_ctl(layer->EpollHandler, EPOLL_CTL_DEL, fd, &interfaceEvent);
	if(0 != resRouting){
		//TODO write error message
		return FUNC_RESULT_FAILED;
	}
	//close and don`t care about socket
	int shutRes = shutdown( fd, SHUT_RDWR );
	int closeRes = close( fd );

	//TODO handle disconnection event

	return FUNC_RESULT_SUCCESS;
}

int initService(ServiceLayer_T* layer, MoarLayerStartupParams_T* params){
	if(NULL == layer || NULL == params)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = FUNC_RESULT_SUCCESS;
	// add sockets
	layer->DownSocket = params->DownSocketHandler;
	layer->UpSocket = params->UpSocketHandler;
	//add unified handlers
	return FUNC_RESULT_SUCCESS;
}

void * MOAR_LAYER_ENTRY_POINT(void* arg){
	ServiceLayer_T layer = {0};
    // load configuration
	int res = initService(&layer, (MoarLayerStartupParams_T*)arg);
	CHECK_RESULT(res);
    // listen for connection
    // in poll
        // if new application connected
            // accept
            // add to pool list
        // if command
            // connect
            // disconnect
            // send
            // state request
            // receive
            // message state
            // get message
}