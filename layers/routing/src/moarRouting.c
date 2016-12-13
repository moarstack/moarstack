//
// Created by svalov, kryvashek on 05.07.16.
//

#include "moarLayerEntryPoint.h"
#include "moarCommons.h"
#include <moarRoutingPrivate.h>
#include <funcResults.h>
#include <memory.h>
#include <moarRoutingCommand.h>
#include <moarRoutingPacketStorage.h>
#include <moarRoutingStoredPacket.h>
#include <moarRoutingNeighborsStorage.h>
#include <moarRoutingPacketProcessing.h>
#include <moarRouteTable.h>
#include <moarRouteProbe.h>


// инициализация работы с Epoll - прополка сокетов
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

//инит роутинга
int routingInit(RoutingLayer_T* layer, void* arg){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == arg)
		return FUNC_RESULT_FAILED_ARGUMENT;

	MoarLayerStartupParams_T* params = (MoarLayerStartupParams_T*)arg;
	// init packet storege
	int initRes = psInit(&layer->PacketStorage);
	if(FUNC_RESULT_SUCCESS != initRes)
		return initRes;
	//init neighbors storage
	int neighborsInitRes = storageInit(&(layer->NeighborsStorage));
	if(FUNC_RESULT_SUCCESS != neighborsInitRes)
		return neighborsInitRes;
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

	// todo write correct params here
	RouteTableSettings_T tableSettings = {0};
	tableSettings.TableSize = 45;
	tableSettings.FinderMarkerRenewRate = 27;
	tableSettings.FinderMarkerDefaultMetric = 128;
	tableSettings.RouteRenewRate = 1;
	tableSettings.RouteDefaultMetric = 138;
	tableSettings.RouteQualityThreshold = 10;
	// init table
	int tableInitRes = RouteTableInit(&layer->RouteTable, &tableSettings);
	if(FUNC_RESULT_SUCCESS != tableInitRes)
		return tableInitRes;

	// init solved table
	tableSettings.TableSize = 10;					// 10 * ( 10 - 1 ) / 2 = 45 (see above)
	tableSettings.FinderMarkerRenewRate = 0;		// no finders in the solved table
	tableSettings.FinderMarkerDefaultMetric = 0;	// no finders in the solved table
	tableSettings.RouteRenewRate = 1;				// routes chance shouldn`t even be decreased in the solved table
	tableSettings.RouteDefaultMetric = 0;			// no defaults, metric is set as a solving result
	tableSettings.RouteQualityThreshold = 0;
	int solvedTableInitRes = RouteTableInit(&layer->RouteTableSolved, &tableSettings);
	if(FUNC_RESULT_SUCCESS != solvedTableInitRes)
		return tableInitRes;
	// probe sending
	layer->NextProbeSentTime = timeAddInterval( timeGetCurrent(), DEFAULT_PROBE_SEND_PERIOD );
	// route table renewing
	layer->NextTableRenewTime = timeAddInterval( timeGetCurrent(), DEFAULT_TABLE_RENEW_PERIOD );
	return FUNC_RESULT_SUCCESS;
}
int routingDeinit(RoutingLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int deinitRes = psDeinit(&layer->PacketStorage);
	if(FUNC_RESULT_SUCCESS != deinitRes)
		return deinitRes;
	// deinit neighbors storage
	int neighborsDeinitRes = storageDeinit(&(layer->NeighborsStorage));
	if(FUNC_RESULT_SUCCESS != neighborsDeinitRes)
		return neighborsDeinitRes;
	//deinit route table
	int tableDeinitRes = RouteTableClear(&layer->RouteTable);
	//int tableDeinitRes = RouteTableDestroy(&layer->RouteTable);
	if(FUNC_RESULT_SUCCESS != tableDeinitRes)
		return tableDeinitRes;
	return FUNC_RESULT_SUCCESS;
}

int updateEpollTimeout( RoutingLayer_T * layer ) {
	RouteStoredPacket_T * pack;
	moarTimeInterval_T 	interval;

	if( NULL == layer )
		return FUNC_RESULT_FAILED_ARGUMENT;

	pack = psGetTop( &( layer->PacketStorage ) );
	interval = EPOLL_TIMEOUT;

	if( NULL != pack ) {
		moarTime_T	nextProcessing = pack->NextProcessing,
					nextMaintain,
					now = timeGetCurrent();

		if( -1 == timeCompare( layer->NextTableRenewTime, layer->NextProbeSentTime ) )
			nextMaintain = layer->NextTableRenewTime;
		else
			nextMaintain = layer->NextProbeSentTime;
		
		if( -1 == timeCompare( nextProcessing, nextMaintain ) )
			interval = timeGetDifference( nextProcessing, now );
		else
			interval = timeGetDifference( nextMaintain, now );

		if( 0 > interval )
			interval = 0;
	}

	layer->EpollTimeout = ( int )interval;
	return FUNC_RESULT_SUCCESS;
}

int routingMaintain( RoutingLayer_T * layer ) {
	moarTime_T	now;
	int 		result;

	if( NULL == layer )
		return FUNC_RESULT_FAILED_ARGUMENT;

	now = timeGetCurrent();

	if( 0 >= timeCompare( layer->NextProbeSentTime, now ) ) { // if need to send probes
		result = sendProbeFirst( layer ); // add probe to queue | send probe to channel layer
		CHECK_RESULT( result );
	}

	if( 0 >= timeCompare( layer->NextTableRenewTime, now ) ) { // if need to renew table
		result = RouteTableRenew( &( layer->RouteTable ), now ); // renew table
		CHECK_RESULT( result );

		layer->NextTableRenewTime = timeAddInterval( now, DEFAULT_TABLE_RENEW_PERIOD );
	}

	result = updateEpollTimeout( layer ); // calculate optimal sleep time & change pool timeout
	return result;
}

void * MOAR_LAYER_ENTRY_POINT(void* arg){
	RoutingLayer_T layer = {0};
	int initRes = routingInit(&layer, arg);
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
		int res = processPacketStorage(&layer); // try to process message queue
		res = routingMaintain( &layer );
	}
	routingDeinit(&layer);
	return NULL;
}