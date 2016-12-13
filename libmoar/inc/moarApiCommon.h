#ifndef __MOAR_API_COMMON_H__
#define __MOAR_API_COMMON_H__

#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <moarMessageId.h>
#include <moarRouting.h>

typedef int32_t MoarDesc_T;

typedef int32_t AppId_T;

typedef enum {
    MessageState_Unknown = 0,
    MessageState_Sending,
    MessageState_Sent,
    MessageState_Lost,
} MessageState_T;


#endif