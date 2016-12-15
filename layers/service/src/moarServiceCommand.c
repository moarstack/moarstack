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

int processSendCommand(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef || fd <= 0 || NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;

	ServiceLayer_T* layer = (ServiceLayer_T*)layerRef;
	AppStartSendMetadata_T* metadata = (AppStartSendMetadata_T*)command->MetaData;

	AppConection_T* conFd = csGetByFdPtr(&layer->ConnectionStorage,fd);
	if(conFd==NULL)
		return FUNC_RESULT_FAILED;
	size_t dataSize = sizeof(ServiceLayerHeader_T)+command->DataSize;
	ServiceLayerHeader_T* header = malloc(dataSize);
	header->LocalAppId = conFd->AppId;
	header->PayloadSize = (ServiceDataSize_T) command->DataSize;
	header->RemoteAppId = metadata->RemoteAppId;
	memcpy(header+1,command->Data, command->DataSize);

	LayerCommandStruct_T com = {0};
	ServiceSendMsgDown_T meta = {0};
	meta.Destination = metadata->RemoteAddr;
	meta.Mid = metadata->Mid;
	com.Command = LayerCommandType_Send;
	com.DataSize = dataSize;
	com.Data = header;
	com.MetaData = &meta;
	com.MetaSize = sizeof(meta);
	int res = WriteCommand(layer->DownSocket, &com);

	return res;
}
int processConnectCommand(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef || fd <= 0 || NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;

	ServiceLayer_T* layer = (ServiceLayer_T*)layerRef;
	AppConnectMetadata_T* metadata = (AppConnectMetadata_T*)command->MetaData;

	return FUNC_RESULT_SUCCESS;
}

int processBindCommand(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef || fd <= 0 || NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;

	ServiceLayer_T* layer = (ServiceLayer_T*)layerRef;
	AppBindMetadata_T* metadata = (AppBindMetadata_T*)command->MetaData;

	AppBindResult_T result = AppBind_Error;

	AppConection_T* conId = csGetByAppIdPtr(&layer->ConnectionStorage,&metadata->appId);
	AppConection_T* conFd = csGetByFdPtr(&layer->ConnectionStorage,fd);
	// check
	if(conFd != NULL)
		result = AppBind_Error;
	else if(conId != NULL)
		result = AppBind_Used;
	else {
		// add
		AppConection_T con = {0};
		con.fd = fd;
		con.AppId = metadata->appId;
		int res = csAdd(&layer->ConnectionStorage, &con);
		if (FUNC_RESULT_SUCCESS == res)
			result = AppBind_OK;
	}
	// send command
	LayerCommandStruct_T com = {0};
	ServiceBindResultMetadata_T meta = {0};
	meta.BindResult = result;
	com.Command = LayerCommandType_BindResult;
	com.DataSize = 0;
	com.Data = NULL;
	com.MetaData = &meta;
	com.MetaSize = sizeof(ServiceBindResultMetadata_T);
	int res = WriteCommand(fd, &com);

	return res;
}
int processDisonnectCommand(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef || fd <= 0 || NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;

	ServiceLayer_T* layer = (ServiceLayer_T*)layerRef;
//	App* metadata = (*)command->MetaData;

	return FUNC_RESULT_SUCCESS;
}

int processAppMessageStateCommand(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef || fd <= 0 || NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;

	ServiceLayer_T* layer = (ServiceLayer_T*)layerRef;
	AppMsgStateMetadata_T* metadata = (AppMsgStateMetadata_T*)command->MetaData;

	return FUNC_RESULT_SUCCESS;
}