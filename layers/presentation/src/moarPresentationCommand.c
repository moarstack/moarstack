//
// Created by svalov on 12/21/16.
//

#include <moarCommons.h>
#include <funcResults.h>
#include <moarPresentationPrivate.h>
#include <moarPresentation.h>
#include <moarPresentationService.h>
#include <moarRoutingPresentation.h>
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

	if(NULL == command->Data || sizeof(PresentHeader_T) > command->DataSize)
		return FUNC_RESULT_FAILED_ARGUMENT;

	//metadata
	RouteReceivedMetadata_T* metadata = command->MetaData;
	PresentHeader_T* header = command->Data;
	// process header
	// still header is empty

	PresentReceivedMsg_T meta = {0};
	meta.Mid = metadata->Id;
	meta.Source = metadata->From;

	LayerCommandStruct_T upCom = {0};
	upCom.Command = LayerCommandType_Receive;
	upCom.Data = header + 1;
	upCom.DataSize = command->DataSize - sizeof(PresentHeader_T);
	upCom.MetaData = &meta;
	upCom.MetaSize = sizeof(PresentReceivedMsg_T);

	int res = WriteCommand(layer->ServiceSocket, &upCom);

	return res;
}