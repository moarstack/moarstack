#include <moarApiCommon.h>
#include <moarCommons.h>
#include <moarServiceApp.h>
#include "moarApiCommon.h"
#include "moarApiCommands.h"
#include "moarServiceApp.h"
#include "funcResults.h"

int moarSocket(MoarDesc_T *MoarDesc) {
    int result = 0;
    int socketValue;
    result = SocketOpenFile(SERVICE_APP_SOCKET_FILE, false, &socketValue);
    if (result != FUNC_RESULT_SUCCESS) {
        //Opening socket error
        perror("could not open socket");
        return FUNC_RESULT_FAILED_IO;
    }
    MoarDesc->SocketFd = socketValue;
    //Create command structure
    LayerCommandStruct_T command = {0};
    command.MetaSize = 0;
    command.DataSize = 0;
    command.MetaData = NULL;
    command.Data = NULL;
    command.Command = LayerCommandType_ConnectApplication;
    result = WriteCommand(socketValue, &command);
    if (result != FUNC_RESULT_SUCCESS) {
        perror("Write command failed");
        return result;
    }
    return FUNC_RESULT_SUCCESS;
}

int moarBind(MoarDesc_T fd, const AppId_T *appId) {
    int result;
    if (fd.SocketFd < 0) {
        perror("Invalid socket descriptor");
        return FUNC_RESULT_FAILED_ARGUMENT;
    }
    LayerCommandStruct_T command = {0};
    AppBindMetadata_T bindMetadata;
    //bindMetadata.MoarFd = fd.MoarFd;
    bindMetadata.appId = *appId;
    command.Command = LayerCommandType_Bind;
    command.Data = NULL;
    command.DataSize = 0;
    command.MetaSize = sizeof(AppBindMetadata_T);
    command.MetaData = &bindMetadata;
    result = WriteCommand(fd.SocketFd, &command);
    if (result != FUNC_RESULT_SUCCESS) {
        perror("Write command failed");
        return FUNC_RESULT_FAILED_IO;
    }
    LayerCommandStruct_T readCommand = {0};
    result = ReadCommand(fd.SocketFd, &readCommand);
    if (result != FUNC_RESULT_SUCCESS) {
        perror("Read command failed");
        return FUNC_RESULT_FAILED_IO;
    }
    //validation incoming command
    if (readCommand.Command != LayerCommandType_BindResult) {
        perror("Invalid incoming command");
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
ssize_t moarRecvFrom(MoarDesc_T fd, void *msg, size_t len, RouteAddr_T *routeAddr, AppId_T  *appId) {
#ifndef MIN
#define MIN(x,y) (((x)>(y))?(y):(x))
#endif
    int result;
    if (fd.SocketFd < 0) {
        perror("Invalid socket descriptor");
        return FUNC_RESULT_FAILED_ARGUMENT;
    }
    LayerCommandStruct_T readCommand = {0};
    result = ReadCommand(fd.SocketFd, &readCommand);
    if (result != FUNC_RESULT_SUCCESS) {
        perror("Read command failed");
        return result;
    }
    if (readCommand.Command != LayerCommandType_Receive) {
        perror("Invalid incoming command");
        FreeCommand(&readCommand);
        return FUNC_RESULT_FAILED_UNEXPECTED_COMMAND;
    }
    ServicePacketReceivedMetadata_T *metadata = readCommand.MetaData;
    *appId = metadata->RemoteAppId;
    *routeAddr = metadata->RemoteAddr;
    size_t copyLen = MIN(readCommand.DataSize, len);
    memcpy(msg, readCommand.Data, copyLen);
    FreeCommand(&readCommand);
    return copyLen;
}

/* Raw variant of Read. Return allocated message buffer */
ssize_t moarRecvFromRaw(MoarDesc_T fd, void *msg, RouteAddr_T *routeAddr, AppId_T  *appId) {
    int result;
    if (fd.SocketFd < 0) {
        perror("Invalid socket descriptor");
        return FUNC_RESULT_FAILED_ARGUMENT;
    }
    LayerCommandStruct_T readCommand = {0};
    result = ReadCommand(fd.SocketFd, &readCommand);
    if (result != FUNC_RESULT_SUCCESS) {
        perror("Read command failed");
        return result;
    }
    if (readCommand.Command != LayerCommandType_Receive) {
        perror("Invalid incoming command");
        FreeCommand(&readCommand);
        return FUNC_RESULT_FAILED_UNEXPECTED_COMMAND;
    }
    ServicePacketReceivedMetadata_T *metadata = readCommand.MetaData;
    *appId = metadata->RemoteAppId;
    *routeAddr = metadata->RemoteAddr;
    msg = readCommand.Data;
    readCommand.Data = NULL; // prevent Data ptr from being disposed in FreeCommand()
    ssize_t len = readCommand.DataSize;
    FreeCommand(&readCommand);
    return len;
}

ssize_t moarSendTo(MoarDesc_T fd, const void *msg, size_t len, const RouteAddr_T *routeAddr, const AppId_T *appId, MessageId_T *msgId) {

}

MessageState_T moarMsgState(MoarDesc_T fd, const MessageId_T *msgId) {

}
