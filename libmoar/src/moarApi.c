#include <moarApiCommon.h>
#include <moarCommons.h>
#include <moarServiceApp.h>
#include <moarService.h>
#include <sys/socket.h>
#include "moarApiCommon.h"
#include "moarApiCommands.h"
#include "moarServiceApp.h"
#include "funcResults.h"
#include <moarApi.h>

#ifndef MIN
#define MIN(x,y) (((x)>(y))?(y):(x))
#endif


int moarAddrFromStr(char* address, RouteAddr_T* routeAddr){
	if(NULL == address || NULL == routeAddr)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int res = routeAddrFromStr(address, routeAddr);
	return res;
}
int moarClose(MoarDesc_T *fd) {
	if(NULL == fd)
		return FUNC_RESULT_FAILED_ARGUMENT;
    shutdown(fd->SocketFd, SHUT_RDWR);
	int closeRes = close(fd->SocketFd);
	fd->SocketFd = -1;
	free(fd);
    return (0 == closeRes)?FUNC_RESULT_SUCCESS:FUNC_RESULT_FAILED;
}

int moarSocketGetDescriptor(MoarDesc_T *fd) {
	if(NULL == fd)
		return FUNC_RESULT_FAILED_ARGUMENT;
    return fd->SocketFd;
}

MoarDesc_T* moarSocket() {
    int result = 0;
	MoarDesc_T* moarDesc = malloc(sizeof(MoarDesc_T));
	if(NULL == moarDesc)
		return NULL;
    int socketValue;
    result = SocketOpenFile(SERVICE_APP_SOCKET_FILE, false, &socketValue);
    if (result != FUNC_RESULT_SUCCESS) {
        free(moarDesc);
        return NULL;
    }
    moarDesc->SocketFd = socketValue;
    //Create command structure
    LayerCommandStruct_T command = {0};
    command.MetaSize = 0;
    command.DataSize = 0;
    command.MetaData = NULL;
    command.Data = NULL;
    command.Command = LayerCommandType_ConnectApplication;
    result = WriteCommand(socketValue, &command);
    if (result != FUNC_RESULT_SUCCESS) {
		moarClose(moarDesc);
        return NULL;
    }
    return moarDesc;
}

int moarBind(MoarDesc_T* fd, const AppId_T *appId) {
	if(NULL == fd || fd->SocketFd < 0 || NULL == appId) {
        return FUNC_RESULT_FAILED_ARGUMENT;
    }
	int result = FUNC_RESULT_FAILED;
    LayerCommandStruct_T command = {0};
    AppBindMetadata_T bindMetadata;
    //bindMetadata.MoarFd = fd.MoarFd;
    bindMetadata.appId = *appId;
    command.Command = LayerCommandType_Bind;
    command.Data = NULL;
    command.DataSize = 0;
    command.MetaSize = sizeof(AppBindMetadata_T);
    command.MetaData = &bindMetadata;
    result = WriteCommand(fd->SocketFd, &command);
    CHECK_RESULT(result);
    LayerCommandStruct_T readCommand = {0};
    result = ReadCommand(fd->SocketFd, &readCommand);
	CHECK_RESULT(result);
    //validation incoming command
    if (readCommand.Command != LayerCommandType_BindResult) {
        FreeCommand(&readCommand);
        return FUNC_RESULT_FAILED_UNEXPECTED_COMMAND;
    }
    ServiceBindResultMetadata_T *bindResultMetadata = readCommand.MetaData;
    switch (bindResultMetadata->BindResult) {
        case AppBind_Error:
            result = FUNC_RESULT_FAILED;
            break;
        case AppBind_OK:
            result = FUNC_RESULT_SUCCESS;
            break;
        case AppBind_Used:
            result = FUNC_RESULT_FAILED_APPID_INUSE;
            break;
        default:
            result = FUNC_RESULT_FAILED;
    }
    FreeCommand(&readCommand);
    return result;
}

/* Traditional receive function. Places data to msg limiting by size len */


ssize_t moarRecvFrom(MoarDesc_T* fd, void *msg, size_t len, RouteAddr_T *routeAddr, AppId_T  *appId) {
    void* dataPtr = NULL;
	ssize_t recvRes = moarRecvFromRaw(fd, &dataPtr, routeAddr, appId);
	if(recvRes >= 0) {
		ssize_t copyLen = 0;
		if (NULL != msg && NULL != dataPtr) {
			copyLen = MIN(recvRes, (ssize_t)len);
			memcpy(msg, dataPtr, copyLen);
		}
		free(dataPtr);
		return copyLen;
	}
	return recvRes;
}

/* Raw variant of Read. Return allocated message buffer */
ssize_t moarRecvFromRaw(MoarDesc_T* fd, void **msg, RouteAddr_T *routeAddr, AppId_T  *appId) {
    int result;
    if (NULL == fd || fd->SocketFd < 0) {
        return FUNC_RESULT_FAILED_ARGUMENT;
    }
    LayerCommandStruct_T readCommand = {0};
    result = ReadCommand(fd->SocketFd, &readCommand);
	CHECK_RESULT(result);
    if (readCommand.Command != LayerCommandType_Receive) {
        FreeCommand(&readCommand);
        return FUNC_RESULT_FAILED_UNEXPECTED_COMMAND;
    }

    ServicePacketReceivedMetadata_T *metadata = readCommand.MetaData;

	if(appId != NULL)
    	*appId = metadata->RemoteAppId;
	if(routeAddr != NULL)
    	*routeAddr = metadata->RemoteAddr;

	ssize_t len = 0;
	if(NULL != msg) {
		*msg = readCommand.Data;
		readCommand.Data = NULL; // prevent Data ptr from being disposed in FreeCommand()
		len = readCommand.DataSize;
	}
    FreeCommand(&readCommand);
    return len;
}

int waitCommand(int fd, LayerCommandStruct_T* readCommand, const LayerCommandType_T commandType){
	if(0 >= fd || NULL == readCommand)
		return FUNC_RESULT_FAILED_ARGUMENT;
	do {
		int result = ReadCommand(fd, readCommand);
		CHECK_RESULT(result);
		if (commandType == readCommand->Command)
			break;
		result = WriteCommand(fd, readCommand);
		FreeCommand(readCommand);
		CHECK_RESULT(result);
	}while(commandType != readCommand->Command);

	return FUNC_RESULT_SUCCESS;
}

ssize_t moarSendTo(MoarDesc_T* fd, const void *msg, size_t len, const RouteAddr_T *routeAddr, const AppId_T *appId, MessageId_T *msgId) {
    int result;
    if (NULL == fd || fd->SocketFd < 0 || NULL == msg || 0 == len || NULL == routeAddr || NULL == appId) {
        return FUNC_RESULT_FAILED_ARGUMENT;
    }
    LayerCommandStruct_T command = {0};
    command.Command = LayerCommandType_Send;
    AppStartSendMetadata_T sendMetadata = {0};
    sendMetadata.RemoteAddr = *routeAddr;
    sendMetadata.RemoteAppId = *appId;
    command.MetaData = &sendMetadata;
    command.MetaSize = sizeof(AppStartSendMetadata_T);
    command.DataSize = len;
    command.Data = msg; //assignment const pointer to non-const pointer
    result = WriteCommand(fd->SocketFd, &command);
	CHECK_RESULT(result);
    // Read send status
    LayerCommandStruct_T readCommand = {0};
	result = waitCommand(fd->SocketFd, &readCommand, LayerCommandType_SendResult);
	CHECK_RESULT(result);

	ServiceSendResultMetadata_T *metadata = readCommand.MetaData;
	switch (metadata->SendResult) {
		case AppSend_OK:
			if(msgId != NULL)
				*msgId = metadata->MsgId;
			//result = FUNC_RESULT_SUCCESS;
			result = (ssize_t) len;
			break;
		case AppSend_Failure:
			result = FUNC_RESULT_FAILED;
			break;
		default:
			result = FUNC_RESULT_FAILED;
	}
	FreeCommand(&readCommand);
    return result;
}

int moarMsgState(MoarDesc_T* fd, const MessageId_T *msgId, MessageState_T *state) {
    int result;
    if (NULL == fd || fd->SocketFd < 0 || NULL == msgId || NULL == state) {
        return FUNC_RESULT_FAILED_ARGUMENT;
    }
    LayerCommandStruct_T command = {0};
    AppMsgStateMetadata_T stateMetadata;
    stateMetadata.MsgId = *msgId;
    command.Command = LayerCommandType_MessageState;
    command.Data = NULL;
    command.DataSize = 0;
    command.MetaData = &stateMetadata;
    command.MetaSize = sizeof(stateMetadata);
    result = WriteCommand(fd->SocketFd, &command);
	CHECK_RESULT(result);
    LayerCommandStruct_T readCommand = {0};

	result = waitCommand(fd->SocketFd, &readCommand, LayerCommandType_MessageStateResult);
	CHECK_RESULT(result);

    ServiceMsgStateResultMetadata_T *metadata = readCommand.MetaData;
    *state = metadata->MsgState;
    FreeCommand(&readCommand);
    return FUNC_RESULT_SUCCESS;
}
