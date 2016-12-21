//
// Created by svalov, kryvashek on 05.07.16.
//

#include <moarPresentationPrivate.h>
#include <funcResults.h>
#include <sys/epoll.h>
#include <moarPresentationCommand.h>
#include "moarLayerEntryPoint.h"
#include "moarCommons.h"
#include "moarPresentation.h"
#include "moarRoutingPresentation.h"
#include "moarPresentationService.h"

//init layer
int presentationInit(PresentationLayer_T* layer, void* arg){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == arg)
		return FUNC_RESULT_FAILED_ARGUMENT;

	MoarLayerStartupParams_T* params = (MoarLayerStartupParams_T*)arg;

	layer->RoutingProcessingRules[0] = MakeProcessingRule(LayerCommandType_Receive, processReceiveCommand);
	layer->RoutingProcessingRules[1] = MakeProcessingRule(LayerCommandType_MessageState, processMsgStateCommand);
	layer->RoutingProcessingRules[2] = MakeProcessingRule(LayerCommandType_None, NULL);
	//again
	layer->ServiceProcessingRules[0] = MakeProcessingRule(LayerCommandType_Send, processSendCommand);
	layer->ServiceProcessingRules[1] = MakeProcessingRule(LayerCommandType_None, NULL);
	return FUNC_RESULT_SUCCESS;
}
//deinit layer
int presentationDeinit(PresentationLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;

	return FUNC_RESULT_SUCCESS;
}
//init epoll
int initEpoll(PresentationLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;

	// setup settings
	layer->EpollCount = EPOLL_EVENTS_COUNT;
	layer->EpollTimeout = EPOLL_TIMEOUT;
	//epoll init here
	memset(layer->EpollEvent,0,layer->EpollCount*sizeof(struct epoll_event));
	//init
	layer->EpollHandler = epoll_create(1);
	if(-1 == layer->EpollHandler) {
		return FUNC_RESULT_FAILED;
	}
	// add routing socket
	struct epoll_event epollEventRouting;
	epollEventRouting.events = EPOLL_ROUTING_EVENTS;
	epollEventRouting.data.fd = layer->RoutingSocket;
	int routingRes = epoll_ctl(layer->EpollHandler, EPOLL_CTL_ADD, layer->RoutingSocket, &epollEventRouting);
	CHECK_RESULT(routingRes);
	// add service socket
	struct epoll_event epollEventService;
	epollEventService.events = EPOLL_SERVICE_EVENTS;
	epollEventService.data.fd = layer->ServiceSocket;
	int serviceRes = epoll_ctl(layer->EpollHandler, EPOLL_CTL_ADD, layer->ServiceSocket, &epollEventService);
	CHECK_RESULT(serviceRes);
	//return
	return FUNC_RESULT_SUCCESS;

}

void * MOAR_LAYER_ENTRY_POINT(void* arg){
	PresentationLayer_T layer = {0};
	int initRes = presentationInit(&layer, arg);
	if(FUNC_RESULT_SUCCESS != initRes){
		return NULL;
	}
	// load configuration
	// init epoll
	int epollInitRes = initEpoll(&layer);
	if(FUNC_RESULT_SUCCESS != epollInitRes)
		return NULL;
	// enable process
	layer.Running = true;
	while(layer.Running) {
		// in poll
		int epollRes = epoll_wait(layer.EpollHandler, layer.EpollEvent,
								  layer.EpollCount, layer.EpollTimeout);
		// in poll
		if(epollRes<0){
			//perror("Routing epoll_wait");
		}
		for(int i=0; i<epollRes;i++) {
			uint32_t event = layer.EpollEvent[i].events;
			int fd = layer.EpollEvent[i].data.fd;
			int processRes = FUNC_RESULT_FAILED;
			if(fd == layer.ServiceSocket){
				processRes = ProcessCommand(&layer, fd, event, EPOLL_SERVICE_EVENTS, layer.ServiceProcessingRules);
			}
			else if(fd == layer.RoutingSocket){
				processRes = ProcessCommand(&layer, fd, event, EPOLL_ROUTING_EVENTS, layer.RoutingProcessingRules);
			}else{
				// wtf? i don`t add another sockets
			}
			//error processing
			if(FUNC_RESULT_SUCCESS != processRes){
				// we have problems
				// return NULL;
			}
		}
	}
	presentationDeinit(&layer);
}
