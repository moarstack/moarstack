//
// Created by svalov on 8/13/16.
//

#include <moarRoutingPrivate.h>
#include <funcResults.h>
#include "moarRouitngCommandProcessing.h"

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
				// res = processSendMessage(layer, fd, &command);
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
				//res = processSendMessage(layer, fd, &command);
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