//
// Created by svalov, kryvashek on 05.07.16.
//

#include <moarPresentationPrivate.h>
#include <funcResults.h>
#include <sys/epoll.h>
#include "moarLayerEntryPoint.h"
#include "moarCommons.h"
#include "moarPresentation.h"
#include "moarRoutingPresentation.h"
#include "moarPresentationService.h"

//init layer
int presentationInit(PresentationLayer_T* layer, void* arg){

}
//deinit layer
int presentationDeinit(PresentationLayer_T* layer){

}
//init epoll
int initEpoll(PresentationLayer_T* layer){

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
