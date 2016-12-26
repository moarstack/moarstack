#ifndef __MOAR_API_COMMON_H__
#define __MOAR_API_COMMON_H__

#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <moarMessageId.h>
#include <moarRouting.h>

#define APP_BIND_RESULT_BITS	8
#define APP_SENT_RESULT_BITS	8
#define MESSAGE_STATE_BITS		8

#pragma pack(push, 1)

typedef struct {
    int MoarFd;
    int SocketFd;
    //TODO: Possibly add mutex for multithreading apps
} MoarDesc_T;

typedef uint16_t AppId_T;

typedef enum {
    AppBind_OK = 0,
    AppBind_Used,
    AppBind_Error
} AppBindResult_T;

typedef enum {
    AppSend_OK = 0,
    AppSend_Failure
} AppSentResult_T;

typedef enum {
    MessageState_Unknown = 0,
    MessageState_Sending,
    MessageState_Sent,
    MessageState_Lost,
} MessageState_T;

#pragma pack(pop)

#endif