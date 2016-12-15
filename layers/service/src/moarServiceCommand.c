//
// Created by svalov on 12/12/16.
//

#include "moarServiceCommand.h"
#include <funcResults.h>
#include <moarPresentationService.h>
#include <moarCommons.h>
#include <moarServiceApp.h>
#include <moarService.h>
#include <moarServicePrivate.h>
#include <moarServiceConStore.h>

int processReceiveCommand(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef || fd <= 0 || NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;

	ServiceLayer_T* layer = (ServiceLayer_T*)layerRef;
	PresentReceivedMsg_T* receivedMsgMetadata = (PresentReceivedMsg_T*)command->MetaData;

	if(command->DataSize >= sizeof(ServiceLayerHeader_T)) {
		ServiceLayerHeader_T *header = command->Data;
		if(command->DataSize != header->PayloadSize + sizeof(ServiceLayerHeader_T))
			return FUNC_RESULT_FAILED_ARGUMENT;
		//make command
		LayerCommandStruct_T com = {0};

		ServicePacketReceivedMetadata_T meta = {0};
		meta.RemoteAddr = receivedMsgMetadata->Source;
		meta.RemoteAppId = header->LocalAppId;

		com.Command = LayerCommandType_Receive;
		com.DataSize = header->PayloadSize;
		com.Data = header+1;
		com.MetaData = &meta;
		com.MetaSize = sizeof(ServicePacketReceivedMetadata_T);

		// get socket
		AppConection_T* con = csGetByAppIdPtr(&layer->ConnectionStorage, &header->RemoteAppId);
		if(NULL == con) // drop message
			return FUNC_RESULT_SUCCESS;
		int res = WriteCommand(con->fd, &com);
		return res;
	}

	return FUNC_RESULT_FAILED;
}

int processMsgStateCommand(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef || fd <= 0 || NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;

	ServiceLayer_T* layer = (ServiceLayer_T*)layerRef;
	PresentMsgState_T* msgStateMetadata = (PresentMsgState_T*)command->MetaData;

	PackStatePresent_T* ptr = hashGetPtr(&layer->MidStorage, &msgStateMetadata->Mid);
	if(NULL == ptr) // some message, but not sent by service
		return FUNC_RESULT_SUCCESS;
	*ptr = msgStateMetadata->State;

	return FUNC_RESULT_SUCCESS;
}