#ifndef __MOAR_API_H__
#define __MOAR_API_H__

#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <moarMessageId.h>
#include <moarRouting.h>


typedef int32_t MoarDesc_T;

typedef int32_t AppId_T;

typedef enum {
	UNKNOWN = 0,
	SENDING,
	SENT,
	LOST,
} MessageState_T;

extern MoarDesc_T moarSocket(void);

extern int moarBind(MoarDesc_T fd, AppId_T appId);

extern ssize_t moarRecvFrom(MoarDesc_T fd, void *msg, size_t len, RouteAddr_T *routeAddr, AppId_T  *appId);

extern ssize_t moarSendTo(MoarDesc_T fd, const void *msg, size_t len, const RouteAddr_T *routeAddr, const AppId_T *appId, MessageId_T *msgId);

extern MessageState_T moarMsgState(MoarDesc_T fd, MessageId_T msgId);

#endif
