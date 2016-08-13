//
// Created by svalov on 8/13/16.
//

#include <moarRoutingPrivate.h>
#include <funcResults.h>
#include "moarRouitngCommandProcessing.h"

int processReceiveCommand(RoutingLayer_T* layer, int fd, LayerCommandStruct_T* command){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//logic here
	return FUNC_RESULT_SUCCESS;
}

int processMessageStateCommand(RoutingLayer_T* layer, int fd, LayerCommandStruct_T* command){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//logic here
	return FUNC_RESULT_SUCCESS;
}

int processNewNeighborCommand(RoutingLayer_T* layer, int fd, LayerCommandStruct_T* command){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//logic here
	return FUNC_RESULT_SUCCESS;
}

int processLostNeighborCommand(RoutingLayer_T* layer, int fd, LayerCommandStruct_T* command){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//logic here
	return FUNC_RESULT_SUCCESS;
}

int processUpdateNeighborCommand(RoutingLayer_T* layer, int fd, LayerCommandStruct_T* command){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//logic here
	return FUNC_RESULT_SUCCESS;
}

int processChannelEvent(RoutingLayer_T* layer, int fd, uint32_t event){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;

	if((event & EPOLL_CHANNEL_EVENTS) !=0) {
		// send
		LayerCommandStruct_T command = {0};
		int commandRes = ReadCommand(fd, &command);
		if (FUNC_RESULT_SUCCESS != commandRes) {
			return commandRes;
		}
		int res = FUNC_RESULT_FAILED;
		switch (command.Command) {
			case LayerCommandType_Receive:
				res = processReceiveCommand(layer, fd, &command);
				break;
			case LayerCommandType_MessageState:
				res = processMessageStateCommand(layer, fd, &command);
				break;
			case LayerCommandType_NewNeighbor:
				res = processNewNeighborCommand(layer, fd, &command);
				break;
			case LayerCommandType_LostNeighbor:
				res = processLostNeighborCommand(layer, fd, &command);
				break;
			case LayerCommandType_UpdateNeighbor:
				res = processUpdateNeighborCommand(layer, fd, &command);
				break;
			default:
				res = FUNC_RESULT_FAILED_ARGUMENT;
				break;
		}
		FreeCommand(&command);
		return res;
	}
	return FUNC_RESULT_SUCCESS;
}

int processSendCommand(RoutingLayer_T* layer, int fd, LayerCommandStruct_T* command){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//logic here
	return FUNC_RESULT_SUCCESS;
}

int processPresentationEvent(RoutingLayer_T* layer, int fd, uint32_t event){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;

	if((event & EPOLL_PRESENTATION_EVENTS) !=0) {
		// send
		LayerCommandStruct_T command = {0};
		int commandRes = ReadCommand(fd, &command);
		if (FUNC_RESULT_SUCCESS != commandRes) {
			return commandRes;
		}
		int res = FUNC_RESULT_FAILED;
		switch (command.Command) {
				case LayerCommandType_Send:
				res = processSendCommand(layer, fd, &command);
				break;
			default:
				res = FUNC_RESULT_FAILED_ARGUMENT;
				break;
		}
		FreeCommand(&command);
		return res;
	}
	return FUNC_RESULT_SUCCESS;
}