#ifndef __MOAR_API_H__
#define __MOAR_API_H__

#include "moarApiCommon.h"

__BEGIN_DECLS

extern int moarAddrFromStr(char* address, RouteAddr_T* routeAddr);
extern int moarClose(MoarDesc_T *fd);
extern int moarSocketGetDescriptor(MoarDesc_T *fd);
extern MoarDesc_T* moarSocket();
extern int moarBind(MoarDesc_T* fd, const AppId_T *appId);
/* Traditional receive function. Places data to msg limiting by size len */
extern ssize_t moarRecvFrom(MoarDesc_T* fd, void *msg, size_t len, RouteAddr_T *routeAddr, AppId_T  *appId);
/* Raw variant of Read. Return allocated message buffer */
extern ssize_t moarRecvFromRaw(MoarDesc_T* fd, void **msg, RouteAddr_T *routeAddr, AppId_T  *appId);
extern ssize_t moarSendTo(MoarDesc_T* fd, const void *msg, size_t len, const RouteAddr_T *routeAddr, const AppId_T *appId, MessageId_T *msgId);
extern int moarMsgState(MoarDesc_T* fd, const MessageId_T *msgId, MessageState_T *state);


__END_DECLS

#endif
