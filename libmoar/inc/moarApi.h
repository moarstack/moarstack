#ifndef __MOAR_API_H__
#define __MOAR_API_H__

#include "moarApiCommon.h"

__BEGIN_DECLS
/**
 * Utility function. Gets address from its string representation.
 * @param address Node address in string form
 * @param routeAddr Node address in MOAR form
 * @return execution result
 */
extern int moarAddrFromStr(char* address, RouteAddr_T* routeAddr);

/**
 * Closes connection, releases used memory, frees descriptor, drops binding. Automatic execution on parent application termination.
 * @param fd Descriptor
 * @return execution result
 */
extern int moarClose(MoarDesc_T *fd);

/**
 * Utility function. Gets internal connection descriptor.
 * @param fd Pointer to descriptor
 * @return execution result
 */
extern int moarSocketGetDescriptor(MoarDesc_T *fd);

/**
 * Generates socket descriptor.
 * @return new socket descriptor.
 */
extern MoarDesc_T* moarSocket();
extern MoarDesc_T* moarSocketFile(char* fileName);
/**
 * Binds application and socket. Meaning is similar to standard UDP socket.
 * @param fd Pointer to descriptor.
 * @param appId Application descriptor.
 * @return execution result
 */
extern int moarBind(MoarDesc_T* fd, const AppId_T *appId);


/**
 * Traditional receive function. Places data to msg limited by size len
 * @param fd socket descriptor
 * @param msg message
 * @param len Message length. If received data amount is greater that this parameter, all excessive data will be dropped.
 * @param routeAddr Sender address.
 * @param appId Sender application descriptor.
 * @return Amount of data received. Negative value means no data was received.
 */
extern ssize_t moarRecvFrom(MoarDesc_T* fd, void *msg, size_t len, RouteAddr_T *routeAddr, AppId_T  *appId);


/**
 * Raw variant of Read. Return allocated message buffer.
 * @param fd socket descriptor
 * @param msg message
 * @param routeAddr Sender address.
 * @param appId Sender application descriptor.
 * @return message size
 */
extern ssize_t moarRecvFromRaw(MoarDesc_T* fd, void **msg, RouteAddr_T *routeAddr, AppId_T  *appId);

/**
 * Traditional send function. Sends msg data to remote node application
 * @param fd socket descriptor
 * @param msg message
 * @param len message length.
 * @param routeAddr destination address
 * @param appId remote port application descriptor
 * @param msgId output parameter. Message id assigned by MOAR. Can be NULL.
 * @return amount of data transmitted. Must be equal to len parameter.
 */
extern ssize_t moarSendTo(MoarDesc_T* fd, const void *msg, size_t len, const RouteAddr_T *routeAddr, const AppId_T *appId, MessageId_T *msgId);

/**
 * Gets current message state
 * @param fd socket descriptor
 * @param msgId Message Id. Could be obtained from moarSendTo function.
 * @param state output parameter function state
 * @return execution result
 */
extern int moarMsgState(MoarDesc_T* fd, const MessageId_T *msgId, MessageState_T *state);


__END_DECLS

#endif
