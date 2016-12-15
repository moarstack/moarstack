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
#include <moarServiceCommand.h>

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
	//init epol
	layer->EpollTimeout = EPOLL_TIMEOUT;
	layer->EpollCount = EPOLL_EVENTS_COUNT;
	memset(layer->EpollEvent,0,layer->EpollCount*sizeof(struct epoll_event));

	layer->EpollHandler = epoll_create(1);
	if(-1 == layer->EpollHandler) {
		return FUNC_RESULT_FAILED;
	}
	// add socket to presentation
	int res = addSocketToEpoll(layer, layer->DownSocket, EPOLL_PRESENTATION_EVENTS);
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
	CHECK_RESULT(res);
	return FUNC_RESULT_SUCCESS;
}
//close connection to app
int processCloseConnection(ServiceLayer_T* layer, int fd){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	struct epoll_event appEvent;
	appEvent.events = EPOLL_APP_EVENTS;
	appEvent.data.fd = fd;
	int resRouting = epoll_ctl(layer->EpollHandler, EPOLL_CTL_DEL, fd, &appEvent);
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
	// TODO load config here

	res = csInit(&layer->ConnectionStorage);
	// add sockets
	layer->DownSocket = params->DownSocketHandler;
	layer->UpSocket = params->UpSocketHandler;
	//add unified handlers
	//app
	layer->AppProcessingRules[0] = MakeProcessingRule(LayerCommandType_None, NULL);
	//presentation
	layer->PresentationProcessingRules[0] = MakeProcessingRule(LayerCommandType_Receive, processReceiveCommand);
	layer->PresentationProcessingRules[1] = MakeProcessingRule(LayerCommandType_MessageState, processMsgStateCommand);
	layer->PresentationProcessingRules[2] = MakeProcessingRule(LayerCommandType_None, NULL);
	return res;
}

void * MOAR_LAYER_ENTRY_POINT(void* arg){
	ServiceLayer_T servicelayer = {0};
    // load configuration
	int res = initService(&servicelayer, (MoarLayerStartupParams_T*)arg);

	if( FUNC_RESULT_SUCCESS != res )
		return NULL;

	res = initEpoll(&servicelayer);

	if( FUNC_RESULT_SUCCESS != res )
		return NULL;

	servicelayer.Running = true;
	while(servicelayer.Running) {
		int epollRes = epoll_wait(servicelayer.EpollHandler, servicelayer.EpollEvent,
								  servicelayer.EpollCount, servicelayer.EpollTimeout);
		// in poll
		if(epollRes<0){
			//perror("Channel epoll_wait");
		}
		for(int i=0; i<epollRes;i++) {
			uint32_t event = servicelayer.EpollEvent[i].events;
			int fd = servicelayer.EpollEvent[i].data.fd;
			// if new app connected
			int res = FUNC_RESULT_FAILED;
			if (fd == servicelayer.UpSocket) {
				res = processNewConnection(&servicelayer, fd);
			} // if command from presentation
			else if (fd == servicelayer.DownSocket) {
				res = ProcessCommand(&servicelayer, fd, event, EPOLL_PRESENTATION_EVENTS,
									 servicelayer.PresentationProcessingRules);
			} //data from app
			else {
				//process disconnected event
				// if app disconnected
				if ((event & EPOLL_APP_DISCONNECT_EVENTS) == 0) {
					res = ProcessCommand(&servicelayer, fd, event, EPOLL_APP_EVENTS,
										 servicelayer.AppProcessingRules);
				}
				if (FUNC_RESULT_SUCCESS != res) {
					processCloseConnection(&servicelayer, fd);
				}
			}
		}
		//some processing here
	}
}