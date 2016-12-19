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

ssize_t moarRecvFrom(MoarDesc_T fd, void *msg, size_t len, RouteAddr_T *routeAddr, AppId_T  *appId) {

}

ssize_t moarSendTo(MoarDesc_T fd, const void *msg, size_t len, const RouteAddr_T *routeAddr, const AppId_T *appId, MessageId_T *msgId) {
    int result;
    if (fd.SocketFd < 0) {
        perror("Invalid socket descriptor");
        return FUNC_RESULT_FAILED_ARGUMENT;
    }
    LayerCommandStruct_T command = {0};
    command.Command = LayerCommandType_SendWR;
    AppStartSendMetadata_T sendMetadata = {0};
    sendMetadata.RemoteAddr = *routeAddr;
    sendMetadata.RemoteAppId = *appId;
    command.MetaData = &sendMetadata;
    command.MetaSize = sizeof(AppStartSendMetadata_T);
    command.DataSize = len;
    command.Data = msg; //assignment const pointer to non-const pointer
    result = WriteCommand(fd.SocketFd, &command);
    if (result != FUNC_RESULT_SUCCESS) {
        perror("Write command failed");
        return result;
    }
    // Read send status
    LayerCommandStruct_T readCommand = {0};
    ReadCommand(fd.SocketFd, &readCommand);
    if (readCommand.Command != LayerCommandType_SendResult) {
        perror("Invalid incoming command");
        FreeCommand(&readCommand);
        return FUNC_RESULT_FAILED_UNEXPECTED_COMMAND;
    }
    ServiceSendResultMetadata_T *metadata = readCommand.MetaData;
    switch (metadata->SendResult) {
        case AppSend_OK:
            *msgId = metadata->MsgId;
            //result = FUNC_RESULT_SUCCESS;
            result = (ssize_t) len;
            break;
        case AppSend_Failure:
            result = FUNC_RESULT_FAILED;
            break;
        default:
            perror("Unknown SendResult");
            result = FUNC_RESULT_FAILED;
    }
    FreeCommand(&readCommand);
    return result;
}

MessageState_T moarMsgState(MoarDesc_T fd, const MessageId_T *msgId) {

}
