#ifndef MOARSTACK_MOARSERVICEAPP_H
#define MOARSTACK_MOARSERVICEAPP_H

#include <stddef.h>
#include "moarCommons.h"
#include "moarService.h"
#include "moarApi.h"


typedef struct {
    //ISSUE: empty? no metadata needed to send?
} AppConnectMetadata_T;

/* Send when the application is asking to create MOARStack socket */
typedef struct {
    int MoarFd;
} AppConnectResultMetadata_T;

typedef struct {
    int MoarFd;
    AppId_T appId;
} AppBindMetadata_T;

typedef struct {
    AppBindResult_T BindResult;
} AppBindResultMetadata_T;

/* Sent by application to get state of message by MsgId */
typedef struct {
    int MoarFd;
    MessageId_T MsgId;
} AppMsgStateMetadata_T;

/* Response from service level containing state of message */
typedef struct {
    int MoarFd;
    MessageId_T MsgId;
    MessageState_T MsgState;
} AppMsgStateResultMetadata_T;

/* Command sent by application to start receiving data from all */
typedef struct {
    int MoarFd;
    size_t MaxLen;
} AppStartReceiveMetadata_T;

/* Command received by application when packet is arrived */
typedef struct {
    AppId_T appId;
    RouteAddr_T RemoteAddr;
} AppPacketReceivedMetadata_T;

/* Command sent by application to push packet to Service level */
typedef struct {
    int MoarFd;
    RouteAddr_T RemoteAddr;
    AppId_T  appId;
} AppStartSendMetadata_T;

/* Command received by application from moarSendTo. It does not
 * report message status but return messageId and return code */
typedef struct {
    AppSentResult_T SendResult;
    MessageId_T MsgId;
} AppSendResultMetadata_T;

#endif