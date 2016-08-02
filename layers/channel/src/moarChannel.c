//
// Created by svalov, kryvashek on 05.07.16.
//

#include <sys/epoll.h>
#include <moarLayerEntryPoint.h>
#include <moarChannelPrivate.h>
#include <funcResults.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <moarChannelInterfaces.h>

int epollInit(ChannelLayer_T *layer) {
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//epoll init
	layer->EpollTimeout = EPOLL_TIMEOUT;
	struct epoll_event epollEventInterface;
	struct epoll_event epollEventRouting;

	layer->EpollCount = EPOLL_EVENTS_COUNT;
	memset(layer->EpollEvent,0,layer->EpollCount*sizeof(struct epoll_event));

	layer->EpollHandler = epoll_create(1);
	if(-1 == layer->EpollHandler) {
		return FUNC_RESULT_FAILED;
	}
	//add socket to interface
	epollEventInterface.events = EPOLL_INTERFACE_SOCKET_EVENTS;
	epollEventInterface.data.fd = layer->DownSocket;
	int resIface = epoll_ctl(layer->EpollHandler, EPOLL_CTL_ADD, layer->DownSocket, &epollEventInterface);
	if(0 != resIface){
		return FUNC_RESULT_FAILED;
	}
	//add socket to routing
	epollEventRouting.events = EPOLL_ROUNTING_EVENTS;
	epollEventRouting.data.fd = layer->UpSocket;
	int resRouting = epoll_ctl(layer->EpollHandler, EPOLL_CTL_ADD, layer->UpSocket, &epollEventRouting);
	if(0 != resRouting){
		return FUNC_RESULT_FAILED;
	}
	return FUNC_RESULT_SUCCESS;
}
//open connection to interface
int processNewConnection(ChannelLayer_T* layer, int fd){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;

	// accept
	int newFd = accept(fd, NULL, NULL);
	if(-1 == newFd) {
		return FUNC_RESULT_FAILED_IO;
	}
	struct epoll_event interfaceEvent;
	interfaceEvent.events = EPOLL_INTERFACE_EVENTS;
	interfaceEvent.data.fd = newFd;
	int resPoll = epoll_ctl(layer->EpollHandler, EPOLL_CTL_ADD, newFd, &interfaceEvent);
	if(0 != resPoll){
		//TODO write error message
		return FUNC_RESULT_FAILED;
	}
	return FUNC_RESULT_SUCCESS;
}
//close connection to interface
int processCloseConnection(ChannelLayer_T* layer, int fd){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	struct epoll_event interfaceEvent;
	interfaceEvent.events = EPOLL_INTERFACE_EVENTS;
	interfaceEvent.data.fd = fd;
	int resRouting = epoll_ctl(layer->EpollHandler, EPOLL_CTL_DEL, fd, &interfaceEvent);
	if(0 != resRouting){
		//TODO write error message
		return FUNC_RESULT_FAILED;
	}
	//close and don`t care about socket
	int shutRes = shutdown( fd, SHUT_RDWR );
	int closeRes = close( fd );

	// remove interface record from list
	int removeRes = interfaceRemove(layer, fd);
	if(FUNC_RESULT_SUCCESS != removeRes)
		return removeRes;
	return FUNC_RESULT_SUCCESS;
}

void * MOAR_LAYER_ENTRY_POINT(void* arg){
	if(NULL == arg)
		return NULL;
	ChannelLayer_T channelLayer;
	MoarLayerStartupParams_T* startupParams = (MoarLayerStartupParams_T*) arg;

	if(startupParams->DownSocketHandler <=0)
		return NULL;
	if(startupParams->UpSocketHandler <=0)
		return NULL;

	channelLayer.UpSocket = startupParams->UpSocketHandler;
	channelLayer.DownSocket = startupParams->DownSocketHandler;

	int listRes = CreateList(&(channelLayer.Interfaces));
	if(FUNC_RESULT_SUCCESS != listRes)
		return NULL;

	// load configuration
	//
	// listen for interface connection
	//listen(channelLayer.DownSocket, LISTEN_COUNT);
	int res = epollInit(&channelLayer);
	if(FUNC_RESULT_SUCCESS != res)
		return NULL;

	channelLayer.Running = true;
	while(channelLayer.Running) {
		int epollRes = epoll_wait(channelLayer.EpollHandler, channelLayer.EpollEvent,
								  channelLayer.EpollCount, channelLayer.EpollTimeout);
		// in poll
		if(epollRes<0){
			perror("Channel epoll_wait");
		}
		for(int i=0; i<epollRes;i++) {
			//interface descriptors
			uint32_t event = channelLayer.EpollEvent[i].events;
			int fd = channelLayer.EpollEvent[i].data.fd;
			// if new interface connected
			if(fd == channelLayer.DownSocket) {
				int res = processNewConnection(&channelLayer, fd);
				if(FUNC_RESULT_SUCCESS != res){
					//TODO write error message
				}
			} // if command from routing
			else if(fd == channelLayer.UpSocket) {
				int res = 0;
				if(FUNC_RESULT_SUCCESS != res){
					//TODO write error message
				}
			} //data from interface
			else {
				int res = 0;
				if(FUNC_RESULT_SUCCESS != res){
					processCloseConnection(&channelLayer,fd);
				}
			}
		}
		// TODO add queue processing
		// timeout | end of commands processing
		// if have messages in queue
		// get message
		// try to send

		// update interval
	}
}
