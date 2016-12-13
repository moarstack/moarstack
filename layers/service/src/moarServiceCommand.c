//
// Created by svalov on 12/12/16.
//

#include "moarServiceCommand.h"
#include <funcResults.h>
#include <moarPresentationService.h>
#include <moarCommons.h>

int processReceiveCommand(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef || fd <= 0 || NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;

	ServiceLayer_T* layer = (ServiceLayer_T*)layerRef;
	PresentReceivedMsg_T* receivedMsgMetadata = (PresentReceivedMsg_T*)command->MetaData;

	return FUNC_RESULT_FAILED;
}

int processMsgStateCommand(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef || fd <= 0 || NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;

	ServiceLayer_T* layer = (ServiceLayer_T*)layerRef;
	PresentMsgState_T* msgStateMetadata = (PresentMsgState_T*)command->MetaData;

	return FUNC_RESULT_FAILED;
}