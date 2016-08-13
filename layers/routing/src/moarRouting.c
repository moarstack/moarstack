//
// Created by svalov, kryvashek on 05.07.16.
//

#include "moarLayerEntryPoint.h"
#include "moarCommons.h"
#include "moarRouting.h"
#include "moarChannelRouting.h"
#include "moarRoutingPresentation.h"
#include <moarRoutingPrivate.h>
#include <funcResults.h>
#include <moarLayerEntryPoint.h>
#include <sys/epoll.h>
#include <memory.h>
#include <moarRouitngCommandProcessing.h>
#include <moarCommons.h>

int initEpoll(RoutingLayer_T* layer){
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
	// add channel socket
	struct epoll_event epollEventChannel;
	epollEventChannel.events = EPOLL_CHANNEL_EVENTS;
	epollEventChannel.data.fd = layer->ChannelSocket;
	int channelRes = epoll_ctl(layer->EpollHandler, EPOLL_CTL_ADD, layer->ChannelSocket, &epollEventChannel);
	if(0 != channelRes)
		return FUNC_RESULT_FAILED;
	// add presentation socket
	struct epoll_event epollEventPresentation;
	epollEventChannel.events = EPOLL_PRESENTATION_EVENTS;
	epollEventChannel.data.fd = layer->PresentationSocket;
	int presentationRes = epoll_ctl(layer->EpollHandler, EPOLL_CTL_ADD, layer->PresentationSocket, &epollEventPresentation);
	if(0 != presentationRes)
		return FUNC_RESULT_FAILED;
	//return
	return FUNC_RESULT_SUCCESS;
}
int routingInit(RoutingLayer_T* layer, void* arg){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == arg)
		return FUNC_RESULT_FAILED_ARGUMENT;

	MoarLayerStartupParams_T* params = (MoarLayerStartupParams_T*)arg;
	//setup socket to channel
	if(params->DownSocketHandler <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	layer->ChannelSocket = params->DownSocketHandler;
	//setup socket to presentation
	if(params->UpSocketHandler <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	layer->PresentationSocket = params->UpSocketHandler;
	//fill processing pointers
	layer->ChannelProcessingRules[0] = MakeProcessingRule(LayerCommandType_Receive, processReceiveCommand);
	layer->ChannelProcessingRules[1] = MakeProcessingRule(LayerCommandType_MessageState, processMessageStateCommand);
	layer->ChannelProcessingRules[2] = MakeProcessingRule(LayerCommandType_NewNeighbor, processNewNeighborCommand);
	layer->ChannelProcessingRules[3] = MakeProcessingRule(LayerCommandType_LostNeighbor, processLostNeighborCommand);
	layer->ChannelProcessingRules[4] = MakeProcessingRule(LayerCommandType_UpdateNeighbor, processUpdateNeighborCommand);
	layer->ChannelProcessingRules[5] = MakeProcessingRule(LayerCommandType_None, NULL);
	//again
	layer->PresentationProcessingRules[0] = MakeProcessingRule(LayerCommandType_Send, processSendCommand);
	layer->PresentationProcessingRules[1] = MakeProcessingRule(LayerCommandType_None, NULL);
	return FUNC_RESULT_SUCCESS;
}

void * MOAR_LAYER_ENTRY_POINT(void* arg){
	RoutingLayer_T layer = {0};
	int initRes = routingInit(&layer, arg);
	if(FUNC_RESULT_SUCCESS != initRes){
		return NULL;
	}
	// load configuration
	// init epoll
	int epollRes = initEpoll(&layer);
	if(FUNC_RESULT_SUCCESS != epollRes)
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
			if(fd == layer.ChannelSocket){
				processRes = ProcessCommand(&layer, fd, event, EPOLL_CHANNEL_EVENTS, layer.ChannelProcessingRules);
			}
			else if(fd == layer.PresentationSocket){
				processRes = ProcessCommand(&layer, fd, event, EPOLL_PRESENTATION_EVENTS, layer.PresentationProcessingRules);
			}else{
				// wtf? i don`t add another sockets
			}
			//error processing
			if(FUNC_RESULT_SUCCESS != processRes){
				// we have problems
				// return NULL;
			}
		}
		//timeout | end of command processing
		// if need to send probes
		// add probe to queue | send probe to channel layer
		// try to process message queue
		// calculate optimal sleep time
		// change pool timeout
	}

}