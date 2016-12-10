#ifndef MOARSTACK_MOARSERVICEAPP_H
#define MOARSTACK_MOARSERVICEAPP_H

#include <stddef.h>
#include "moarCommons.h"
#include "moarService.h"
#include "moarApi.h"

/* Struct per command should be here */

typedef struct {
    int MoarFd;
} AppConnectMetadata_T;

typedef struct {
    int MoarFd;
    AppId_T appId;
} AppBindMetadata_T;

typedef struct {
    int MoarFd;
    MessageState_T MsgState;
    //MessageId_T MsgId;
} AppMsgStateMetadata_T;

// Metadata for packet arrived to app level
typedef struct {
    AppId_T appId;
    RouteAddr_T RemoteAddr;
    // received len
} AppReceiveMetadata_T;

typedef struct {
    //sent len
} AppSendMetadata_T;

#endif