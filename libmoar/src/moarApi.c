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
    printf("Socket opened\r\n");
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
        return FUNC_RESULT_FAILED_IO;
    }
    LayerCommandStruct_T readCommand = {0};
    int attempts = APP_READ_ATTEMPTS_COUNT;
    do {
        result = ReadCommand(socketValue, &readCommand);
        if (result != FUNC_RESULT_SUCCESS && attempts > 0)
            sleep(1);
        attempts--;
    } while (attempts > 0 && result != FUNC_RESULT_SUCCESS);
    if (result != FUNC_RESULT_SUCCESS) {
        perror("No response from service level");
        return FUNC_RESULT_FAILED_IO;
    }
    // process incoming command
    if (readCommand.Command != LayerCommandType_ConnectApplicationResult) {
        perror("Invalid incoming command");
        return FUNC_RESULT_FAILED;
    }
    ServiceConnectResultMetadata_T *metadata = readCommand.MetaData;
    MoarDesc->SocketFd = socketValue;
    MoarDesc->MoarFd = metadata->MoarFd;
    return FUNC_RESULT_SUCCESS;
}

int moarBind(MoarDesc_T fd, const AppId_T *appId) {

}

ssize_t moarRecvFrom(MoarDesc_T fd, void *msg, size_t len, RouteAddr_T *routeAddr, AppId_T  *appId) {

}

ssize_t moarSendTo(MoarDesc_T fd, const void *msg, size_t len, const RouteAddr_T *routeAddr, const AppId_T *appId, MessageId_T *msgId) {

}

MessageState_T moarMsgState(MoarDesc_T fd, const MessageId_T *msgId) {

}
