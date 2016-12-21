//
// Created by svalov on 12/21/16.
//

#include <moarCommons.h>
#include <funcResults.h>
#include <moarPresentationPrivate.h>
#include "moarPresentationCommand.h"

// send from service
int processSendCommand(void* layerRef, int fd, LayerCommandStruct_T* command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	PresentationLayer_T* layer = (PresentationLayer_T*)layerRef;


	return FUNC_RESULT_FAILED;
}

// msgstate from routing
int processMsgStateCommand(void* layerRef, int fd, LayerCommandStruct_T* command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	PresentationLayer_T* layer = (PresentationLayer_T*)layerRef;


	return FUNC_RESULT_FAILED;
}

// receive from routing
int processReceiveCommand(void* layerRef, int fd, LayerCommandStruct_T* command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	PresentationLayer_T* layer = (PresentationLayer_T*)layerRef;


	return FUNC_RESULT_FAILED;
}